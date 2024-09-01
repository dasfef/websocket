const WebSocket = require('ws');
const express = require('express');
const http = require('http');
const { connected } = require('process');
const fs = require('fs');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// CORS 정책 허가
var cors = require('cors');
app.use(cors());

const maxClients = 10;                                                  // 동시 연결 수 제한 : 10
let connectedClients = [];                                              // 클라이언트 리스트
var orders = [];
let imgNum = 0;                                                         // 이미지 캡쳐 변수
let captureFlag = false;                                                // 캡쳐 메시지 수신 변수 선언
let captureTime = Date.now();                                           // 캡쳐용 현재 시간 체크

// 클라이언트 ID 파악 함수
function generateUniqueID() {
  return Math.random().toString(36).substr(2, 9);
}

// 웹소켓 접속시 index.html 파일 전송
// app.use("/", function(req, res){
//   res.sendFile(__dirname + '/public/index.html');                       
// });

// app.use('/models', express.static(path.join(__dirname + '/bestpt/best.onnx')));
app.use('/css', express.static('public'));
app.get('*', (req, res) => {
  res.sendFile(__dirname + '/public/index.html');
})
// 웹소켓 /model 폴더 root 로 설정

// 웹소켓 접속시 최초 이벤트
wss.on('connection', (ws) => {
  if (connectedClients.length < maxClients){
    captureTime = Date.now();                                           // 소켓 접속시간 초기화
    connectedClients.push(ws);                                          // 새로 연결된 클라이언트를 배열에 추가
    ws.id = generateUniqueID();
    orders.push(ws.id);
    console.log('=== Client Connected ===');                            // 클라이언트 ID 파악 후 로그 출력
    console.log('# Order :', orders.indexOf(ws.id)+1);
    console.log('# ID :', ws.id);
    console.log(captureTime);
  } else {
    ws.send('Server is at capacity, please try again later');           // 클라이언트 접속 개수 초과시
    ws.close();
    return;
  }

  // 클라이언트로부터 받은 메세지를 다른 모든 클라이언트에게 전송
  ws.on('message', (message) => {
    var arraybuffer = new Uint8Array(message);                            // ArrayBuffer to Uint8
    var s = String.fromCharCode.apply(null, arraybuffer);                 // Uint8 to String
    
    // 이미지 캡쳐 확인 부분
    if(typeof s === 'string'){
      if(s === 'start'){                                                  // 들어온 메시지가 'start' 일때
        captureFlag = true;                                               // 이미지 캡쳐 버튼 토글 ON
        console.log(`Capture Activate: ${captureFlag}`);
      }
      // 이미지 캡쳐 중지
      else if(s === 'stop'){
        captureFlag = false;                                              // 이미지 캡쳐 버튼 토글 OFF
        console.log(`Capture Activate: ${captureFlag}`);
      }
  
    // 이미지 캡쳐 시작
    else if(message instanceof Buffer && captureFlag){                    // 메시지 Buffer instance == True && captureFlag == True
      let currentTime = Date.now();
      console.log("Image capturing and Calculating Time...");
      if(currentTime - captureTime > 5000) {
        fs.writeFile(`/Users/dasfef/Desktop/WORK/websocket/capture/image_${imgNum}.jpg`, message, err => {
          console.log("Image Save SUCCEED");
          if(err) {
            console.log('Error: ', err);
            return;
          }
          imgNum++;
        });
        captureTime = Date.now();
      }
    }
  }

    // 이미 다른 클라이언트로 전송하는 부분
    connectedClients.forEach(client => {
      if(client !== ws && client.readyState === WebSocket.OPEN){
        client.send(message);
      }
    })
  });

  ws.on('close', () => {
    console.log('...Client disconnected...');
    delete orders.indexOf(ws.id);
    console.log('* Order :', orders.indexOf(ws.id)+1);
    console.log('* ID :', ws.id);
    // orders = orders.filter(client => client !== ws.id);
    connectedClients = connectedClients.filter(client => client !== ws);
  });
});

// app.get('/', (req, res) => {
//   res.sendFile(__dirname + '/index.html');
// });

server.listen(8989, () => {
  console.log('listening on *:8989');
});