#include "OcppClient.h"
#include "websockettcpclient.h"
#include "cJSON.h"
#include <QDebug>

OcppClient::OcppClient(QObject *parent) : QObject(parent)
{
    m_ocppProtocol = new OcppProtocol;
}
//绑定websocket
void OcppClient::setWebSocketClient(WebSocketTcpClient *wsClient)
{
    if(!wsClient)return;
    m_wsClient = wsClient;
    connect(m_wsClient, &WebSocketTcpClient::ocppMessageReceived,
            this, &OcppClient::onWebSocketTextReceived);
    connect(this,SIGNAL(sigWebSocketTextSend(QByteArray)),
            m_wsClient,SLOT(sendTextMessage(QByteArray)));


}

QString OcppClient::generateMessageId()
{
    m_messageId++;
    return QString::number(m_messageId);
}
//接收信号槽
void OcppClient::onWebSocketTextReceived( QByteArray data)
{
//    qDebug() << "[OCPP] ↓ 收到:" << data;
    parseOcppMessage(data);
}

// ============================================================
// 解析OCPP消息（核心框架）
// ============================================================
void OcppClient::parseOcppMessage(const QByteArray &data)
{
    cJSON *root = cJSON_Parse(data.constData());//在函数最后删除
    if (!root || !cJSON_IsArray(root)) {
        qDebug() << "[OCPP] ❌ 格式错误";
        cJSON_Delete(root);
        return;
    }

    int msgTypeId = cJSON_GetArrayItem(root, 0)->valueint;
//    QString messageId = cJSON_GetArrayItem(root, 1)->valuestring;
    char *payloadStr = cJSON_PrintUnformatted(root);//在函数最后删除
    qDebug()<<payloadStr<<__FILE__<<__LINE__;
    switch (msgTypeId)
    {
    case 2: { // CALL：服务器发的请求

        break;
    }

    case 3: { // CALLRESULT：对我们请求的响应


        break;
    }

    case 4: { // CALLERROR：错误响应

        break;
    }

    default:
        qDebug() << "[OCPP] ❌ 未知消息类型:" << msgTypeId;
        break;
    }
    free(payloadStr);
    cJSON_Delete(root);
}
