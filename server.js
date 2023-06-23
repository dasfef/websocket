const WebSocket = require('ws');
const http = require('http');
const express = require('express');
const app = express();

const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

const maxClients = 5;                                                 // 동시 연결 수 제한
let connectedClients = [];

wss.on('connection', (ws) => {
  if (connectedClients.length < maxClients){
    connectedClients.push(ws);                                    // 새로 연결된 클라이언트를 배열에 추가
    console.log('Client connected');
  } else {
    ws.send('Server is at capacity, please try again later');
    ws.close();
    return;
  }

  ws.on('message', (message) => {
    // 클라이언트로부터 받은 메세지를 다른 모든 클라이언트에게 전송
    connectedClients.forEach(client => {
      if(client !== ws && client.readyState === WebSocket.OPEN){
        client.send(message);
      }
    })
  });

  ws.on('close', () => {
    console.log('Client disconnected');
    connectedClients = connectedClients.filter(client => client !== ws);
  });
});

server.listen(8999, () => {
  console.log('Server started on Port 8999');
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