const express = require('express');
const http = require('http');
const socketIO = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = socketIO(server);

io.on('connection', (socket) => {
  console.log(socket.id + ' user connected');

  socket.on('disconnect', () =>{
    console.log(socket.id + ' user disconnected');
  });

  socket.on('message', (msg) => {
    console.log('message: ' + msg);
    io.emit('message', msg);
  });
});

app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});

// 서버에 데이터를 보낼 때 
app.post('/', function(req, res) {
  var data = '';
  req.setEncoding('binary');

  req.on('data', function(chunk) {
    data += chunk;
  });

  req.on('end', function() {
    io.emit('binaryData', data);
    res.end();
  });
});

server.listen(8989, () => {
  console.log('listening on *:8989');
});