const express = require('express');
const app = express();
const path = require('path');
const http = require('http');
const WebSocket = require('ws');

// public 디렉토리 내의 정적 파일 제공
app.use(express.static(path.join(__dirname, 'public')));

app.get('/', function(req, res){
    // res.sendFile(path.join(__dirname, 'public', 'index.html'));
    res.send('Hello World');
});

const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// app.listen(8999, function() {
//     console.log('Example app listening on port 8999');
// });

wss.on('error', (error) => {
    console.log('WebSocket error: ${error}');
});

server.listen(8999, function() {
    console.log('App and WebSocket are listening on port 8999');
});