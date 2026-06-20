#ifndef WEBSOCKETTCP_H
#define WEBSOCKETTCP_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>

class WebSocketTcpClient : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketTcpClient(QObject *parent = nullptr);
    // 头文件 public 区加：
    void setAutoReconnect(bool enable);
    bool isAutoReconnect() const;
public slots:
    void sendTextMessage(const QString &text);   // 发送文本消息
    void sendBinaryMessage(const QByteArray &data); // 发送二进制消息
private slots:
    void onTcpConnected();
    void onDataReceived();
    void sendUpgradeRequest();
    void parseWebSocketFrame(const QByteArray &data);
    void sendAutoMessage();
    void sendMessage(const QByteArray &data);
    void closeWebSocket();
private slots:
    void onTcpDisconnected();   // TCP断开回调

private:
    void startReconnect();      // 开始重连
    void stopReconnect();       // 停止重连
    void resetReconnect();      // 重连成功后重置

    QTimer  *m_reconnectTimer;  // 重连定时器
    int      m_reconnectInterval;   // 当前重连间隔（毫秒）
    int      m_maxReconnectInterval; // 最大重连间隔
    bool     m_autoReconnect;   // 是否启用自动重连
    bool     m_manualClose;     // 是否主动关闭（主动关闭不重连）
private:
    void sendPong(const QByteArray &payload);   // 回 pong
    void buildAndSendFrame(int opcode, const QByteArray &payload); // 通用帧组装

    QTcpSocket *m_tcpSocket;
    QTimer     *m_timer;
    bool        m_handshakeDone;

    QByteArray  m_recvBuffer;       // 接收缓冲区（解决粘包/拆包）
    QByteArray  m_fragmentBuffer;   // 分片消息组装缓冲区
    bool        m_isFragmented;     // 是否处于分片消息中
};

#endif
