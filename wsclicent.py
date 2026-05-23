# websocket 客户端
import asyncio
import websockets

async def client_test():
    async with websockets.connect("ws://127.0.0.1:8765") as websocket:
        while True:
            msg = input("输入消息: ")
            await websocket.send(msg)
            res = await websocket.recv()
            print(res)

asyncio.run(client_test())
