#!/usr/bin/env python

import asyncio
# pip install websockets
import websockets
import ssl
async def hello(url):
    cert_ignore = ssl._create_unverified_context() if url.startswith('wss') else None
    async with websockets.connect(url, ssl=cert_ignore) as websocket:
        msg = "Hello world! to " + url
        await websocket.send(msg)
        print(f">>> {msg}")
        strback = await websocket.recv()
        print(f"<<< {strback}")

asyncio.run(hello("ws://localhost:8888/ws"))
asyncio.run(hello("wss://localhost:9999/hello"))