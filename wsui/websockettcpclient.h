#ifndef WEBSOCKETTCP_H
#define WEBSOCKETTCP_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>

class WebSocketTcpClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketTcpClient(QObject *parent = nullptr);
    ~WebSocketTcpClient();

    void setAutoReconnect(bool enable);
    bool isAutoReconnect() const;
    bool isConnected() const;

public slots:
    void sendTextMessage(QString text);
    void sendTextMessage(QByteArray text);
    void sendBinaryMessage(QByteArray data);

    // 设置 WebSocket 服务器地址（支持 ws:// 和 wss://）
    void setServerUrl(const QString &url);

    // 主动连接
    void open();

    // 主动关闭
    void close();

    // 设置心跳间隔（单位：秒）
//    void setHeartbeatInterval(int seconds);
signals:
    // 原有信号（保持兼容）
    void sendWebSocketTextFrame(QByteArray data);
    void ocppMessageReceived(QByteArray data);

    // 新增：连接状态变化
    void connected();
    void disconnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);
//    void sendAutoMessage();
    void performReconnect();

private:
    void startReconnect();
    void stopReconnect();
    void resetReconnect();

    QWebSocket *m_webSocket;
//    QTimer     *m_heartbeatTimer;
    QTimer     *m_reconnectTimer;

    QUrl       m_serverUrl;
    bool       m_autoReconnect;
    bool       m_manualClose;
    int        m_reconnectInterval;
    int        m_maxReconnectInterval;

    bool       m_bootNotificationSent;
    int        m_heartbeatInterval;
};

#endif // WEBSOCKETTCP_H
