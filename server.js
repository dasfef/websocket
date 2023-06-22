const WebSocket = require('ws');
const http = require('http');

const server = http.createServer();
const wss = new WebSocket.Server({ server });

// server.maxConnections = 10;                                         // 동시 연결 수 제한
const maxClients = 5;
let connectedClients = 0;

// let connectedClients = [];                                          // 연결된 클라이언트를 관리하는 배열
wss.on('connection', (ws) => {
  if(connectedClients < maxClients){
    connectedClients++;

    ws.on('message', (message) => {
      // 메시지를 비동기 처리하는 부분
      broadcastMessage(message, ws)
        .catch((err) => {
          console.error(err);
        });
    });

    ws.on('close', () => {
      console.log('Client disconnected');
      connectedClients--;
    });
  } else{
    ws.close(1001, 'Too many connections');
  };
});

async function broadcastMessage(message, ws) {
  return new Promise((resolve, reject) => {
    try{
      // 클라이언트로부터 받은 메세지를 다른 모든 클라이언트에게 전송
      connectedClients.forEach(client => {
        if(client !== ws && client.readyState === WebSocket.OPEN){
          client.send(message);
        }
      });
      resolve();
    }catch (err){
      reject(err);
    }
  });
}

//   console.log('Client connected');
//   connectedClients.push(ws);                                        // 새로 연결된 클라이언트를 배열에 추가

//   ws.on('message', (message) => {
//     // 클라이언트로부터 받은 메세지를 다른 모든 클라이언트에게 전송
//     connectedClients.forEach(client => {
//         if(client !== ws && client.readyState === WebSocket.OPEN){
//             client.send(message);
//         }
//     })
//   });

//   ws.on('close', () => {
//     console.log('Client disconnected');
//     connectedClients = connectedClients.filter(client => client !== ws);
//   });
// });

server.listen(8999, () => {
    console.log('Server started on Port 8999');
});
