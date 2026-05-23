#include "websocketclient.h"

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject{parent}
{
    m_webSocket = new QWebSocket();
    m_timer = new QTimer(this);

    // 连接信号槽
    connect(m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &WebSocketClient::onError);

    // 定时器 1000ms = 1秒
    connect(m_timer, &QTimer::timeout, this, &WebSocketClient::sendTimerData);

    // 连接服务器
    connectServer();
}

void WebSocketClient::connectServer()
{
    m_webSocket->open(QUrl("ws://127.0.0.1:8765"));
}

// 连接成功后启动定时器
void WebSocketClient::onConnected()
{
    qDebug() << "✅ 已连接服务器，启动1s定时器";
    m_timer->start(1000);
}

void WebSocketClient::onDisconnected()
{
    qDebug() << "❌ 断开连接";
    m_timer->stop();
}

// 每秒自动发送
void WebSocketClient::sendTimerData()
{
    static int count = 0;
    count++;
    QString msg = QString("Qt定时消息 %1").arg(count);
    m_webSocket->sendTextMessage(msg);
    qDebug() << "📤 已发送：" << msg;
}

void WebSocketClient::onTextMessageReceived(const QString &msg)
{
    qDebug() << "📥 服务器回复：" << msg;
}

void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "⚠️ 错误：" << m_webSocket->errorString();
}
