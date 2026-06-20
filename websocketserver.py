import asyncio
import websockets
from websockets.exceptions import ConnectionClosed

# 维护所有已连接的客户端
connected_clients: set[websockets.WebSocketServerProtocol] = set()


async def handle_client(websocket: websockets.WebSocketServerProtocol):
    """处理单个客户端连接"""
    # 新连接加入
    connected_clients.add(websocket)
    client_addr = websocket.remote_address
    print(f"[+] 客户端已连接: {client_addr}，当前在线: {len(connected_clients)}")

    try:
        # 持续接收消息
        async for message in websocket:
            print(f"[←] 收到 {client_addr} 消息: {message}")

            # 简单的回声 + 广播
            response = f"服务器收到: {message}"
            await websocket.send(response)
            print(f"[→] 回复 {client_addr}: {response}")

    except ConnectionClosed:
        print(f"[-] 客户端断开: {client_addr}")
    finally:
        connected_clients.discard(websocket)
        print(f"[i] 当前在线: {len(connected_clients)}")


async def broadcast(message: str):
    """向所有在线客户端广播消息"""
    if not connected_clients:
        return
    await asyncio.gather(
        *[client.send(message) for client in connected_clients],
        return_exceptions=True
    )


async def main(host: str = "127.0.0.1", port: int = 8765):
    """启动 WebSocket 服务器"""
    async with websockets.serve(handle_client, host, port):
        print(f"WebSocket 服务器已启动: ws://{host}:{port}")
        await asyncio.Future()  # 永久运行


if __name__ == "__main__":
    # 安装依赖: pip install websockets
    asyncio.run(main())