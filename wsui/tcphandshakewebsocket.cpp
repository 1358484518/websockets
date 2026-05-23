#include "WebSocketClient.h"

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent)
    , m_handshakeCompleted(false)
{
    m_tcpSocket = new QTcpSocket(this);
    m_timer = new QTimer(this);

    connect(m_tcpSocket, &QTcpSocket::connected,    this, &WebSocketClient::onTcpConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead,    this, &WebSocketClient::onDataReceived);
    connect(m_timer,     &QTimer::timeout,          this, &WebSocketClient::sendAutoMessage);

    // 连接服务器
    m_tcpSocket->connectToHost("127.0.0.1", 8765);
}

void WebSocketClient::onTcpConnected()
{
    qDebug() << "[TCP] 连接成功";
    sendUpgradeHandshake();
}

void WebSocketClient::sendUpgradeHandshake()
{
    QString key = "dGhlIHNhbXBsZSBub25jZQ==";
    QString request = QString(
        "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:8765\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %1\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n"
    ).arg(key);

    m_tcpSocket->write(request.toUtf8());
}

void WebSocketClient::onDataReceived()
{
    QByteArray data = m_tcpSocket->readAll();

    if (!m_handshakeCompleted) {
        if (data.contains("101 Switching Protocols")) {
            qDebug() << "[WS] 握手成功，协议已升级";
            m_handshakeCompleted = true;
            m_timer->start(1000);
        }
    } else {
        parseWebSocketFrame();
    }
}

void WebSocketClient::parseWebSocketFrame()
{
    uchar h1 = m_tcpSocket->read(1)[0];
    uchar h2 = m_tcpSocket->read(1)[0];

    bool fin   = (h1 & 0x80) >> 7;
    int  op    = h1 & 0x0F;
    bool mask  = (h2 & 0x80) >> 7;
    int  len   = h2 & 0x7F;

    QByteArray maskKey;
    if (mask) {
        maskKey = m_tcpSocket->read(4);
    }

    QByteArray payload = m_tcpSocket->read(len);
    for (int i = 0; i < len; i++) {
        payload[i] ^= maskKey[i % 4];
    }

    qDebug() << "[RECV]" << payload;
}

void WebSocketClient::sendAutoMessage()
{
    static int count = 0;
    count++;

    QString msg = QString("Qt TCP WebSocket 消息 %1").arg(count);
    QByteArray data = msg.toUtf8();

    // 组帧
    uchar header1 = 0x81;
    uchar header2 = data.length();

    QByteArray frame;
    frame.append((char)header1);
    frame.append((char)header2);
    frame.append(data);

    m_tcpSocket->write(frame);
    qDebug() << "[SEND]" << msg;
}
