const WebSocket = require('ws');
const http = require('http');
const express = require('express');
const { connected } = require('process');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

const maxClients = 10;                                                  // 동시 연결 수 제한 : 10
let connectedClients = [];                                              // 클라이언트 리스트
var orders = [];

// 클라이언트 ID 파악 함수
function generateUniqueID() {
  return Math.random().toString(36).substr(2, 9);
}

// 웹소켓 접속시 index.html 파일 전송
app.use("/", function(req, res){
  res.sendFile(__dirname + '/public/index.html');                       
});

// 웹소켓 접속시
wss.on('connection', (ws) => {
  if (connectedClients.length < maxClients){
    connectedClients.push(ws);                                          // 새로 연결된 클라이언트를 배열에 추가
    ws.id = generateUniqueID();
    orders.push(ws.id);
    console.log('==== Client Connected ====');                                    // 클라이언트 ID 파악 후 로그 출력
    console.log('Order : ', orders.indexOf(ws.id)+1);
    console.log('ID : ', ws.id);
  } else {
    ws.send('Server is at capacity, please try again later');           // 클라이언트 접속 개수 초과시
    ws.close();
    return;
  }

// 클라이언트로부터 받은 메세지를 다른 모든 클라이언트에게 전송
  ws.on('message', (message) => {
    connectedClients.forEach(client => {
      if(client !== ws && client.readyState === WebSocket.OPEN){
        client.send(message);
      }
    })
  });

  ws.on('close', () => {
    console.log('==== Client disconnected ====');
    console.log('Order : ', orders.indexOf(ws.id)+1);
    console.log('ID : ', ws.id);
    orders = orders.filter(client => client !== ws.id);
    connectedClients = connectedClients.filter(client => client !== ws);
  });
});

server.listen(8989, () => {
  console.log('Server started on Port 8989');
});




// const WebSocket = require('ws');
// const http = require('http');
// const express = require('express');
// const app = express();

// const server = http.createServer(app);
// const wss = new WebSocket.Server({ server });

// const maxClients = 5;                                                 // 동시 연결 수 제한
// let connectedClients = [];

// wss.on('connection', (ws) => {
//   if (connectedClients.length <= maxClients){
//     connectedClients.push(ws);                                            // 새로 연결된 클라이언트를 배열에 추가
//   }
//   console.log('Client connected');
  
//   ws.on('message', (message) => {
//     // 클라이언트로부터 받은 메세지를 다른 모든 클라이언트에게 전송
//     connectedClients.forEach(client => {
//       if(client !== ws && client.readyState === WebSocket.OPEN){
//             client.send(message);
//         }
//       })
//     });
    
//     ws.on('close', () => {
//       console.log('Client disconnected');
//       connectedClients = connectedClients.filter(client => client !== ws);
//     });
// });

// wss.on('connection', function connection(ws) {
//   ws.on('message', function incoming(data) {
//       wss.clients.forEach(function each(client){
//           if(client.readyState === WebSocket.OPEN) {
//               client.send(data);
//           }
//       });
//   });
// });

// server.listen(8999, () => {
//   console.log('Server started on Port 8999');
// });




//   if(connectedClients < maxClients){
//     connectedClients.push(ws);

//     ws.on('message', (message) => {
//       // 메시지를 비동기 처리하는 부분
//       broadcastMessage(message, ws)
//         .catch((err) => {
//           console.error(err);
//         });
//     });

//     ws.on('close', () => {
//       console.log('Client disconnected');
//       connectedClients = connectedClients.filter(client => client !== ws);
//     });
//   } else{
//     ws.close(1001, 'Too many connections');
//   };
// });

// async function broadcastMessage(message, ws) {
//   return new Promise((resolve, reject) => {
//     try{
//       // 클라이언트로부터 받은 메세지를 다른 모든 클라이언트에게 전송
//       connectedClients.forEach(client => {
//         if(client !== ws && client.readyState === WebSocket.OPEN){
//           client.send(message);
//         }
//       });
//       resolve();
//     }catch (err){
//       reject(err);
//     }
//   });
// }