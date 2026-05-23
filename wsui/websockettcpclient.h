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

private slots:
    void onTcpConnected();
    void onDataReceived();
    void sendUpgradeRequest();
    void parseWebSocketFrame(const QByteArray &data);
    void sendAutoMessage();
    void closeWebSocket();
private:
    QTcpSocket *m_tcpSocket;
    QTimer     *m_timer;
    bool        m_handshakeDone;
};

#endif
