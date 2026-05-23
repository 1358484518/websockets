#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QCryptographicHash>
#include <QDebug>

class WebSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);

private slots:
    void onTcpConnected();
    void onDataReceived();
    void sendUpgradeHandshake();
    void parseWebSocketFrame();
    void sendAutoMessage();

private:
    QTcpSocket* m_tcpSocket;
    QTimer*     m_timer;
    bool        m_handshakeCompleted;
};

#endif
