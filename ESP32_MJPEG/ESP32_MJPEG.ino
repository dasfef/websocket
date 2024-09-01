#define APP_CPU 1
#define PRO_CPU 0

#include "OV2640.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <WebSocketsClient.h>

#include <esp_bt.h>
#include <esp_wifi.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"
//공유기 ssid, pwd 설정
#include "home_wifi_multi.h"

//정적 ip 할당
IPAddress staticIP(192, 168, 100, 30);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);

//카메라 모듈 이름
OV2640 cam;

WebServer server(80);
WebSocketsClient webSocket;    // webSocket 선언

TaskHandle_t tMjpeg;   // 웹 서버에 대한 클라이언트 연결을 처리
TaskHandle_t tCam;     // 카메라에서 사진 프레임을 가져와서 로컬에 저장하는 작업을 처리
TaskHandle_t tStream;  // 실제로 연결된 모든 클라이언트에 프레임 스트리밍

// frameSync semaphore - 다음 프레임으로 교체될 때 스트리밍 버퍼를 방지하기 위해 사용
SemaphoreHandle_t frameSync = NULL;

// 현재 스트리밍 중인 클라이언트에 연결된 대기열 저장소
QueueHandle_t streamingClients;

// 프레임 속도
const int FPS = 14;

//50ms(20Hz)마다 웹 클라이언트 요청을 처리
const int WSINTERVAL = 100;

// 웹소켓 전송 확인 함수
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length){
  switch(type) {
    case WStype_CONNECTED:
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      webSocket.sendTXT("Connected");
      break;
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
  }
}


//서==버 연결 처리기 작업==
void mjpegCB(void* pvParameters) {
  //작업 빈도를 제어하기 위해 사용
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = pdMS_TO_TICKS(WSINTERVAL);

  //프레임 동기화 세마포어 생성 및 초기화
  frameSync = xSemaphoreCreateBinary();
  xSemaphoreGive( frameSync );

  // 연결된 모든 클라이언트를 추적하기 위한 대기열 만들기
  streamingClients = xQueueCreate( 10, sizeof(WiFiClient*) );

  //section 설정

  //  카메라에서 프레임을 잡기 위한 RTOS 작업 생성
  xTaskCreatePinnedToCore(
    camCB,        // callback
    "cam",        // name
    4096,         // stacj size
    NULL,         // parameters
    2,            // priority
    &tCam,        // RTOS task handle
    APP_CPU);     // core

  //  연결된 모든 클라이언트에 스트림을 푸시하는 작업 생성
  xTaskCreatePinnedToCore(
    streamCB,
    "strmCB",
    4 * 1024,
    NULL, //(void*) handler,
    2,
    &tStream,
    APP_CPU);

  //  웹서버 처리 루틴 등록
  server.on("/mstream", HTTP_GET, handleJPGSstream);  //스트리밍
  server.on("/jpg", HTTP_GET, handleJPG);             //캡쳐
  server.onNotFound(handleNotFound);

  // webserver 시작
  server.begin();

  //=== loop() section  ===================
  xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    server.handleClient();

    //  모든 서버 클라이언트 처리 요청 후 다른 작업을 실행한 다음 일시 중지

    // 현재 실행 중인 태스크를 일시 중단하고 다른 태스크에게 실행 기회를 주는 명령
    taskYIELD();
    //정확한 주기로 태스크를 일시 중단하는 명령
    //xLastWakeTime 변수를 사용하여 이전에 일어난 시간을 추적, xFrequency 주기에 따라 태스크를 지정된 시간까지 일시 중단
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

volatile size_t camSize;    // 현재 프레임의 크기, 바이트
volatile char* camBuf;      // 현재 프레임에 대한 포인터


//카메라에서 프레임을 가져오는 RTOS 작업
void camCB(void* pvParameters) {

  TickType_t xLastWakeTime;

  //  현재 원하는 프레임 속도와 관련된 실행 간격
  const TickType_t xFrequency = pdMS_TO_TICKS(1000 / FPS);

  //xSemaphore
  //초기화된 세마포어(잠금 해체 상태)
  //다중 태스크 간 동기화를 달성하기 위해 사용
  //portMUX_INITIALIZER_UNLOCKED
  // FreeRTOS에서 제공하는 세마포어 동기화를 위한 포트 타입을 나타내는 매크로
  // 태스크 간 동기화를 달성하기 위해 사용
  portMUX_TYPE xSemaphore = portMUX_INITIALIZER_UNLOCKED;

  //  2개의 프레임에 대한 포인터, 각각의 크기 및 현재 프레임의 인덱스
  char* fbs[2] = { NULL, NULL };
  size_t fSize[2] = { 0, 0 };
  int ifb = 0;

  //loop() section
  
  //xLastWakeTime
  // 마지막으로 태스크가 깨어난 시간을 나타내는 변수
  //xTaskGetTickCount
  // FreeRTOS의 태스크 스케줄링 시스템에서 현재의 tick(주기적인 시간 단위) 값을 반환하는 함수
  xLastWakeTime = xTaskGetTickCount();

  for (;;) {

    //  카메라에서 프레임을 잡고 크기를 쿼리
    cam.run();
    size_t s = cam.getSize();

    //  프레임 크기가 이전에 할당한 것보다 크면 현재 프레임 공간의 125%를 요청
    if (s > fSize[ifb]) {
      fSize[ifb] = s * 4 / 3;
      fbs[ifb] = allocateMemory(fbs[ifb], fSize[ifb]);
    }

    //.getfb()-현재 프레임 버퍼(frame buffer)의 이미지 데이터를 가져오는 함수
    //  현재 프레임을 로컬 버퍼에 복사
    char* b = (char*) cam.getfb();
    memcpy(fbs[ifb], b, s);

    //  다른 작업을 실행하고 현재 프레임 속도 간격이 끝날 때까지 대기(남은 시간이 있는 경우).

    //함수는 현재 실행 중인 태스크를 일시 중단하고 다른 태스크에게 실행 기회를 주는 명령
    taskYIELD();
    //정확한 주기로 태스크를 일시 중단하는 명령
    //xLastWakeTime 변수를 사용하여 이전에 일어난 시간을 추적, xFrequency 주기에 따라 태스크를 지정된 시간까지 일시 중단
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    //  현재 클라이언트로 스트리밍 중인 프레임이 없는 경우에만 프레임 전환

    //xSemaphoreTake - 세마포어를 획득하기 위해 호출되는 함수
    //portMAX_DELAY - 무한정 대기할 수 있는 최대 시간
    // -> frameSync 세마포어를 획득하기 위해 무한정 대기하는 것
    //목적 -  다른 태스크와의 동기화를 관리하고, 작업의 진행을 조절
    xSemaphoreTake( frameSync, portMAX_DELAY );

    //  현재 프레임을 전환하는 동안 인터럽트를 허용X

    //임계 영역을 시작.  임계 영역에 진입하면 다른 태스크는 해당 임계 영역에 접근할 수 없음
    portENTER_CRITICAL(&xSemaphore);
    // 버퍼를 선택하고 해당 버퍼에 프레임을 저장하는 작업
    camBuf = fbs[ifb];
    //프레임의 크기를 설정
    camSize = s;
    ifb++;
    //1과 비트 AND 연산(&=)을 수행하여 1 또는 0의 순환 시퀀스를 생성(ifb 값을 0 또는 1로 유지하기 위한 작업)
    ifb &= 1;  // 이것은 1, 0, 1, 0, 1 ... 시퀀스를 생성해야 함
    //임계 영역을 종료
    portEXIT_CRITICAL(&xSemaphore);

    //  프레임을 대기자에게 프레임이 준비되었음을 알림.
    xSemaphoreGive( frameSync );

    // 스트리밍 작업에 최소한 하나의 프레임이 있음을 알립니다.
    // 클라이언트에게 프레임 전송을 시작할 수 있음(있는 경우).

    //tStream 태스크에 대한 알림을 보내는 역할
    xTaskNotifyGive( tStream );

    //  즉시 다른 (스트리밍) 작업 실행
    taskYIELD();

    // 스트리밍 작업이 자체적으로 정지된 경우(스트리밍할 활성 클라이언트가 없음)
    // 카메라에서 프레임을 가져올 필요가 없으므로 작업 일시 중단
    //eSuspended 상태는 태스크가 일시 중단된 상태를 의미
    if ( eTaskGetState( tStream ) == eSuspended ) {
      vTaskSuspend(NULL);  // vTaskSuspend(NULL) 함수를 호출하여 현재 실행 중인 태스크를 자체적으로 일시 중단
    }
  }
}


// PSRAM을 활용하는 메모리 할당자 함수
//
char* allocateMemory(char* aPtr, size_t aSize) {

  //  aPtr이 NULL이 아닌 경우, free 함수를 사용하여 해당 메모리를 해제
  if (aPtr != NULL) free(aPtr);

  //현재 사용 가능한 힙(heap)의 크기를 가져와서 freeHeap 변수에 저장
  size_t freeHeap = ESP.getFreeHeap();
  //메모리 할당 후의 포인터를 저장하는 변수
  char* ptr = NULL;

  // 요청된 메모리 크기 aSize가 현재 사용 가능한 힙의 2/3보다 큰 경우, 바로 PSRAM을 사용하여 메모리를 할당
  //PSRAM - ESP32 보드의 추가 외부 메모리로 사용될 수 있는 메모리 영역
  //psramFound() - PSRAM이 ESP32 보드에서 사용 가능한지 여부를 확인하는 함수
  if ( aSize > freeHeap * 2 / 3 ) {
    if ( psramFound() && ESP.getFreePsram() > aSize ) {
      ptr = (char*) ps_malloc(aSize);
    }
  }
  else {
    //  힙이 충분할 경우 - 빠른 RAM을 버퍼로 할당
    ptr = (char*) malloc(aSize);

    //  힙 할당에 실패하면 PSRAM 사용
    if ( ptr == NULL && psramFound() && ESP.getFreePsram() > aSize) {
      ptr = (char*) ps_malloc(aSize);
    }
  }

  // 메모리 할당에 실패한 것으로 간주하고 ESP32 보드를 재시작
  if (ptr == NULL) {
    ESP.restart();
  }
  return ptr;
}


// 스트리밍
//HTTP 응답 헤더 및 관련 상수들을 정의하는 부분
const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);
const int bdrLen = strlen(BOUNDARY);
const int cntLen = strlen(CTNTTYPE);


// 클라이언트의 연결 요청 처리 
void handleJPGSstream(void)
{
  //  10명의 클라이언트만 수용
  //uxQueueSpacesAvailable(streamingClients) - streamingClients 큐에 남아있는 사용 가능한 공간의 개수를 반환
  if ( !uxQueueSpacesAvailable(streamingClients) ) return;


  // 서버에 새로운 클라이언트가 연결되면, 해당 클라이언트와 통신하기 위해 새로운 WiFiClient 객체를 생성하고 초기화
  WiFiClient* client = new WiFiClient();
  *client = server.client();

  // 즉시 클라이언트에 헤더를 보냄
  client->write(HEADER, hdrLen);
  client->write(BOUNDARY, bdrLen);

  // 클라이언트를 스트리밍 대기열로 푸시
  // 새로운 클라이언트를 streamingClients 큐로 보내어, 이후의 처리 단계에서 해당 클라이언트와 관련된 작업을 수행할 수 있도록 함
  //0 - 보내기 작업이 완료될 때까지 블록되지 않도록 하는 디레이 값
  xQueueSend(streamingClients, (void *) &client, 0);

  // 스트리밍 작업이 이전에 일시 중단된 경우 깨우기
  //tCam 작업의 상태를 확인하는 함수 호출. eSuspended 상태는 작업이 일시 중지된 상태.
  //tCam 작업을 재개하는 함수 호출
  if ( eTaskGetState( tCam ) == eSuspended ) vTaskResume( tCam );
  //이것도 위와 같음
  if ( eTaskGetState( tStream ) == eSuspended ) vTaskResume( tStream );
}


//실제로 연결된 모든 클라이언트에 콘텐츠 스트리밍
void streamCB(void * pvParameters) {
  char buf[16];
  TickType_t xLastWakeTime;
  TickType_t xFrequency;

  //  첫 번째 프레임이 캡처되고 보낼 항목이 있을 때까지 기다리기.
  //  클라이언트
  ulTaskNotifyTake( pdTRUE,          /* 종료하기 전에 알림 값을 지우기. */
                    portMAX_DELAY ); /* 알림이 전달될 때까지 작업이 무기한으로 차단 */

  xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    // 초당 프레임 수(FPS)에 따라 계산된 프레임 간격을 FreeRTOS 틱 단위로 나타내는 값
    xFrequency = pdMS_TO_TICKS(1000 / FPS);

    // 큐에 현재 대기 중인 클라이언트의 수를 확인하기 위해 사용되는 것
    UBaseType_t activeClients = uxQueueMessagesWaiting(streamingClients);

    //연결된 클라이언트가 있으면
    if ( activeClients ) {
      // 주파수/연결된 클라이언트 수
      // 클라이언트 수에 따라 작업 부하를 고르게 분산시키기 위해
      xFrequency /= activeClients;

      //xQueueReceive 함수를 사용하여 큐에서 대기 중인 연결된 클라이언트의 참조를 가져옴
      WiFiClient *client;
      xQueueReceive (streamingClients, (void*) &client, 0);

      //클라이언트가 여전히 연결되어 있지 않으면
      if (!client->connected()) {
        //클라이언트를 삭제하고 메모리 해체
        delete client;
      }
      //연결되어 있으면
      else {

        // 세마포어를 획득하여 프레임에 대한 접근을 동기화.
        xSemaphoreTake( frameSync, portMAX_DELAY );

        //프레임을 연결된 클라이언트에게 전송
        client->write(CTNTTYPE, cntLen);
        sprintf(buf, "%d\r\n\r\n", camSize);
        client->write(buf, strlen(buf));
        //camBuf : 실제 프레임 데이터
        client->write((char*) camBuf, (size_t)camSize);
        client->write(BOUNDARY, bdrLen);

        // client를 streamingClients 큐에 보내는 역할
        xQueueSend(streamingClients, (void *) &client, 0);

        //xSemaphoreGive( frameSync ) - frameSync 세마포어를 해제
        //taskYIELD - 현재 실행 중인 태스크를 일시 중단하고 다른 태스크에게 실행을 양보하는 명령
        xSemaphoreGive( frameSync );
        taskYIELD();
      }
    }
    //연결된 클라이언트가 없으면 
    else {
      // 현재 실행 중인 태스크를 일시 중단하는 명령
      vTaskSuspend(NULL);
    }

    // 모든 클라이언트에 서비스를 제공한 후 다른 작업을 실행
    //현재 실행 중인 태스크를 일시 중단하고 다른 태스크에게 실행을 양보
    taskYIELD();
    //정확한 주기로 태스크를 일시 중단하는 명령
    //xLastWakeTime 변수를 사용하여 이전에 일어난 시간을 추적, xFrequency 주기에 따라 태스크를 지정된 시간까지 일시 중단
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}


//HTTP 응답 헤더
const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);

//하나의 JPEG 프레임 제공
void handleJPG(void)
{
  WiFiClient client = server.client();

  if (!client.connected()) return;
  cam.run();
  client.write(JHEADER, jhdLen);
  client.write((char*)cam.getfb(), cam.getSize());
}


//잘못된 URL 요청 처리
void handleNotFound()
{
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(200, "text / plain", message);
}



// 메소드 설정
void setup()
{

  // 시리얼 연결 설정
  Serial.begin(115200);
  delay(1000); 

  if (WiFi.config(staticIP, gateway, subnet) == false) {
     Serial.println("Configuration failed.");
  }
  // 카메라 구성
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Frame parameters: pick one
  //  config.frame_size = FRAMESIZE_UXGA;
    config.frame_size = FRAMESIZE_SVGA;
  //  config.frame_size = FRAMESIZE_QVGA;
  //config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  if (cam.init(config) != ESP_OK) {
    Serial.println("Error initializing the camera");
    delay(10000);
    ESP.restart();
  }


  //  Configure and connect to WiFi
  IPAddress ip;

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID1, PWD1);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  ip = WiFi.localIP();
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.print("Stream Link: http://");
  Serial.print(ip);
  Serial.println("/mstream");

  // 웹소켓 경로 지정
  webSocket.begin("192.168.0.8", 8800);
  webSocket.onEvent(webSocketEvent);

  // 카메라 구성
  xTaskCreatePinnedToCore(
    mjpegCB,
    "mjpeg",
    4 * 1024,
    NULL,
    2,
    &tMjpeg,
    APP_CPU);
}

void loop() {
  webSocket.loop();

  camera_fb_t* fb = NULL;
  fb = esp_camera_fb_get();

  if(!fb) {
    Serial.println("Camera capture Failed");
    return;
  }

  webSocket.sendBIN(fb->buf, fb->len);
  esp_camera_fb_return(fb);
  
  // delayMicroseconds(50);
}