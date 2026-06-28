#include "websockettcpclient.h"
#include <QSslConfiguration>
#include "cJSON.h"

// 默认服务器地址（兼容原有硬编码）
//#define WS_DEFAULT_URL "ws://cpmstest.timxon.com:8887/ws/ocpp/1358484518"
#define WS_DEFAULT_URL  "wss://cpmstest.timxon.com:8888/ws/ocpp/1358484518"

WebSocketTcpClient::WebSocketTcpClient(QObject *parent)
    : QObject(parent)
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
//    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_serverUrl(WS_DEFAULT_URL)
    , m_autoReconnect(true)
    , m_manualClose(false)
    , m_reconnectInterval(1000)
    , m_maxReconnectInterval(30000)
    , m_bootNotificationSent(false)
{
    m_reconnectTimer->setSingleShot(true);

    // 连接 QWebSocket 信号
    connect(m_webSocket, &QWebSocket::connected, this, &WebSocketTcpClient::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketTcpClient::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketTcpClient::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketTcpClient::onBinaryMessageReceived);
    connect(m_webSocket,
        static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
        this, &WebSocketTcpClient::onError);
    connect(m_webSocket, &QWebSocket::sslErrors, this, &WebSocketTcpClient::onSslErrors);

    // 心跳定时器
//    connect(m_heartbeatTimer, &QTimer::timeout, this, &WebSocketTcpClient::sendAutoMessage);

    // 重连定时器
    connect(m_reconnectTimer, &QTimer::timeout, this, &WebSocketTcpClient::performReconnect);

    // 自动发起连接（和原代码行为一致）
    open();
}

WebSocketTcpClient::~WebSocketTcpClient()
{
    m_manualClose = true;
    stopReconnect();
    if (m_webSocket->state() != QAbstractSocket::UnconnectedState) {
        m_webSocket->close();
    }
}

// ============================================================
// 公共接口
// ============================================================
void WebSocketTcpClient::setAutoReconnect(bool enable)
{
    m_autoReconnect = enable;
    if (!enable) {
        stopReconnect();
    }
}

bool WebSocketTcpClient::isAutoReconnect() const
{
    return m_autoReconnect;
}

bool WebSocketTcpClient::isConnected() const
{
    return m_webSocket->state() == QAbstractSocket::ConnectedState;
}

void WebSocketTcpClient::setServerUrl(const QString &url)
{
    m_serverUrl = QUrl(url);
    qDebug() << "[WS] Server URL set to:" << m_serverUrl.toString();
}

void WebSocketTcpClient::open()
{
    if (m_webSocket->state() == QAbstractSocket::ConnectedState ||
        m_webSocket->state() == QAbstractSocket::ConnectingState) {
        qDebug() << "[WS] Already connected or connecting, skip open.";
        return;
    }

    m_manualClose = false;
    qDebug() << "[WS] Connecting to:" << m_serverUrl.toString();

    QNetworkRequest request(m_serverUrl);

    // OCPP 1.6 强制子协议头（不加服务器返回 404）
    request.setRawHeader("Sec-WebSocket-Protocol", "ocpp1.6");

    // wss 模式：配置 TLS，支持 TLS 1.2 / 1.3 自动协商
    if (m_serverUrl.scheme() == "wss") {
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        request.setSslConfiguration(sslConfig);
        qDebug() << "[WS] WSS mode enabled, TLS 1.2+ auto-negotiation";
    }

    m_webSocket->open(request);
}

void WebSocketTcpClient::close()
{
    m_manualClose = true;
    stopReconnect();
//    m_heartbeatTimer->stop();
    m_webSocket->close();
    qDebug() << "[WS] Manually closed";
}

// ============================================================
// 发送消息（保留原有接口）
// ============================================================
void WebSocketTcpClient::sendTextMessage(QString text)
{
    if (m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "[WS] Not connected, message dropped";
        return;
    }
    m_webSocket->sendTextMessage(text);
}

void WebSocketTcpClient::sendTextMessage(QByteArray text)
{
    sendTextMessage(QString::fromUtf8(text));
}

void WebSocketTcpClient::sendBinaryMessage(QByteArray data)
{
    if (m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "[WS] Not connected, binary message dropped";
        return;
    }
    m_webSocket->sendBinaryMessage(data);
}

// ============================================================
// QWebSocket 槽函数
// ============================================================
void WebSocketTcpClient::onConnected()
{
    qDebug() << "[WS] Connected successfully";
    resetReconnect();
    m_bootNotificationSent = false;
//    m_heartbeatTimer->start(5000);
    emit connected();
}

void WebSocketTcpClient::onDisconnected()
{
    qDebug() << "[WS] Disconnected";
//    m_heartbeatTimer->stop();

    if (m_manualClose) {
        qDebug() << "[RECONNECT] Manual close, skip reconnect";
        m_manualClose = false;
        emit disconnected();
        return;
    }

    if (m_autoReconnect) {
        startReconnect();
    }

    emit disconnected();
}

void WebSocketTcpClient::onTextMessageReceived(const QString &message)
{
    QByteArray data = message.toUtf8();

    emit sendWebSocketTextFrame(data);
    emit ocppMessageReceived(data);
}

void WebSocketTcpClient::onBinaryMessageReceived(const QByteArray &message)
{
    qDebug() << "[WS] Binary message received, size:" << message.size();
}

void WebSocketTcpClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    qDebug() << "[WS] Socket error:" << m_webSocket->errorString();

    if (m_autoReconnect && !m_manualClose) {
        startReconnect();
    }
}

void WebSocketTcpClient::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "[WS] SSL errors:";
    for (const QSslError &err : errors) {
        qDebug() << "  -" << err.errorString();
    }
    // 开发环境忽略证书错误，生产环境建议删除此行
    m_webSocket->ignoreSslErrors();
}

// ============================================================
// 重连机制（指数退避）
// ============================================================
void WebSocketTcpClient::performReconnect()
{
    if (m_webSocket->state() == QAbstractSocket::ConnectedState ||
        m_webSocket->state() == QAbstractSocket::ConnectingState) {
        return;
    }

    qDebug() << "[RECONNECT] Attempting reconnect... interval:" << m_reconnectInterval << "ms";
    open();
}

void WebSocketTcpClient::startReconnect()
{
    if (m_reconnectTimer->isActive()) return;
    if (m_webSocket->state() == QAbstractSocket::ConnectingState) return;
    if (m_webSocket->state() == QAbstractSocket::ConnectedState) return;

    m_reconnectTimer->start(m_reconnectInterval);
    qDebug() << "[RECONNECT] Scheduled reconnect in" << m_reconnectInterval << "ms";

    m_reconnectInterval *= 2;
    if (m_reconnectInterval > m_maxReconnectInterval) {
        m_reconnectInterval = m_maxReconnectInterval;
    }
}

void WebSocketTcpClient::stopReconnect()
{
    if (m_reconnectTimer->isActive()) {
        m_reconnectTimer->stop();
        qDebug() << "[RECONNECT] Stopped";
    }
}

void WebSocketTcpClient::resetReconnect()
{
    stopReconnect();
    m_reconnectInterval = 1000;
}
