const express = require('express');
const app = express();
const path = require('path');
const http = require('http');
const server = http.createServer(app);
const WebSocket = require('ws');
const wss = new WebSocket.Server({ server });

// public 디렉토리 내의 정적 파일 제공
app.use(express.static(path.join(__dirname, 'public')));

app.get('/', function(req, res){
    res.sendFile(path.join(__dirname, 'public/index.html'));
});

wss.on('connection', function connection(ws) {
    ws.on('message', function incoming(data) {
        wss.clients.forEach(function each(client){
            if(client.readyState === WebSocket.OPEN) {
                client.send(data);
            }
        })
    })
})

wss.on('error', (error) => {
    console.log('WebSocket error: ${error}');
});

server.listen(8800, function() {
    console.log('App and WebSocket are listening on port 8800');
});