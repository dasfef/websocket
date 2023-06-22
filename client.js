var socket = new WebSocket('ws://localhost:8999');

socket.onopen = function() {
  console.log('Connection opened');
};

socket.onmessage = function(event) {
  var messages = document.getElementById('messages');
  var message = document.createElement('li');
  message.innerText = event.data;
  messages.appendChild(message);
};

socket.onclose = function() {
  console.log('Connection closed');
};

function sendText() {
  var input = document.getElementById('message');
  socket.send(input.value);
  input.value = '';
}
