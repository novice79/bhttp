#!/usr/bin/env node
import WebSocket from 'ws';
// npm install ws -S
// chmod +x ws_cli.{js,py}
const ws = new WebSocket('ws://localhost:8888/ws');

ws.on('open', function open() {
  ws.send('something');
});

ws.on('message', function message(data) {
  console.log('received: %s', data);
});
ws.on('error', (err)=> {
    console.log(`err: ${err}`);
});
ws.on('close', (code, reason)=> {
    console.log(`reason: ${reason}`);
});