import asyncio
import websockets

async def echo(websocket):
    async for message in websocket:
        await websocket.send(f"服务器回复: {message}")

async def main():
    async with websockets.serve(echo, "0.0.0.0", 8765):
        await asyncio.Future()

asyncio.run(main())