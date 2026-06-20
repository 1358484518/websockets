1.这是一个了解websocket协议的底层示例。

2.增加py示例：
	2.1 ws.py 是最简单的ws服务器。
	2.2 wsclicent.py 是ws客户端。
	2.3 websocketserver.py 是含打印的ws服务器。
	2.4 ocppcliect.py 是ocpp客户端。

3.修改wsui的qt项目连接ocpp服务器。
	3.1 ocpp服务器的地址是：ws://cpmstest.timxon.com:8887
	3.2 充电桩ID是 1358484518
	3.3 由TCP协议切换为ws协议的http请求：
	
		#define WS_HOST_SERVER_ADDR "cpmstest.timxon.com"//"ws://cpmstest.timxon.com"//"127.0.0.1"
		#define WS_HOST_SERVER_PORT 8887//8765
		QString key = "dGhlIHNhbXBsZSBub25jZQ==";
		QString req = QString(
			"GET /ws/ocpp/1358484518 HTTP/1.1\r\n"
			"Host: %1:%2\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Key: %3\r\n"
			"Sec-WebSocket-Version: 13\r\n"
			"Sec-WebSocket-Protocol: ocpp1.6\r\n"  // 2. 添加OCPP强制子协议头（不加必404）
			"\r\n"
		).arg(WS_HOST_SERVER_ADDR).arg(WS_HOST_SERVER_PORT).arg(key);








