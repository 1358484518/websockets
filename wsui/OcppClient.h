#ifndef OCPPCLIENT_H
#define OCPPCLIENT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QMap>
#include "OcppProtocol.h"

class WebSocketTcpClient;

class OcppClient : public QObject
{
    Q_OBJECT
public:
    explicit OcppClient(QObject *parent = nullptr);
    void setWebSocketClient(WebSocketTcpClient *wsClient);

Q_SIGNALS:
    // 业务信号（后面需要再加）
    void sigWebSocketTextSend( QByteArray data);
private slots:
    void onWebSocketTextReceived(QByteArray data);

private:
    WebSocketTcpClient *m_wsClient = nullptr;
    int m_messageId = 0;

    // 保存待匹配的请求：messageId -> action
//    QMap<QString, QString> m_pendingRequests;

    // 核心解析函数
    void parseOcppMessage(const QByteArray &data);

    // 工具函数
    QString generateMessageId();

    OcppProtocol *m_ocppProtocol;
};

#endif // OCPPCLIENT_H
