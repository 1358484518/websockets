#include "OcppClient.h"
#include "websockettcpclient.h"
#include "cJSON.h"
#include <QDebug>

OcppClient::OcppClient(QObject *parent) : QObject(parent)
{
//    m_ocppProtocol = new OcppProtocol;
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

void OcppClient::CancelReservationConf(cJSON *obj)
{
    if(!obj)return;
    CancelReservation conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(CANCEL_ACCEPTED);//同意取消预约
    char *response = conf.toJson();
    QByteArray ba = response;
    qDebug()<<"取消预约"<<ba;
    emit sigWebSocketTextSend(ba);
    // 发送响应...
    free(response);
}

void OcppClient::handleServerCall(const QString &action, cJSON *payload)
{
    auto it = m_MapActionConf.find(action);
    if (it != m_MapActionConf.end()) {
        (this->*it.value())(payload);  // 调用对应的 Conf 处理函数
    } else {
        qWarning() << "[OcppClient] 未支持的服务器请求:" << action;
        // 可选：回一个 NotImplemented 错误
    }
}


//接收信号槽
void OcppClient::onWebSocketTextReceived( QByteArray data)
{
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
        QString messageId = cJSON_GetArrayItem(root, 2)->valuestring;
        qDebug()<<"CALL：服务器发的请求"<<messageId;
        handleServerCall(messageId,root);
        break;
    }

    case 3: { // CALLRESULT：对我们请求的响应 客户端不用处理


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
