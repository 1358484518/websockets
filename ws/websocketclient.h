#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QDebug>

class WebSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);

private slots:
    void connectServer();
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &msg);
    void onError(QAbstractSocket::SocketError error);

    // 定时器1秒触发
    void sendTimerData();

private:
    QWebSocket *m_webSocket;
    QTimer *m_timer;
};

#endif // WEBSOCKETCLIENT_H
