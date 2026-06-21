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


4.wsui程序说明
	4.1 采用分层架构 
	
		WebSocketTcpClient 管理 数据收发 心跳 bootnotify
		OcppClient 处理ocpp业务逻辑
		OcppProtocol 负责响应和请求协议数据的生成。

5.ocpp1.6J消息

		实现了所有消息的类，管理消息动作的生成。
		
		设计协议类，协议类分为客户端和服务端，然后在协议类中分别实现客户端和服务端相关的代码，
	协议类和ui进行交互，传递ui给的信息，到协议类，客户端类收到服务端的请求，直接调用协议类生成回复，并把发送的回复回传给协议类，
	协议类收到发送的回复json报文，把报文发送给ui，ui显示到log。
	
		ui要主动发送消息的情形，是ui主动发送信号给协议类，协议类收到信号后，组包内容发送信息给客户端，客户端再把消息发送到
	websocket和发送给协议类，协议类发送给ui展示log,客户端主动把消息发送出去之后，收到服务器的回复，再回传给协议类，协议类再回传给ui。



