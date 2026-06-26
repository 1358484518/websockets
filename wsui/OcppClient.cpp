#include "OcppClient.h"
#include "QDateTime"
#include "websockettcpclient.h"
#include "cJSON.h"
#include <QDebug>

#define SEND_CONF_RESPONSE() do {    \
    char *response = conf.toJson();  \
    QByteArray ba = response;        \
    qDebug()<<"回复："<<ba;           \
    emit sigWebSocketTextSend(ba);   \
    free(response);                  \
    } while(0)


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

void OcppClient::RemoteStartTransactionConf(cJSON *obj)
{
    if(!obj)return;
    RemoteStartTransaction conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(REMOTE_START_ACCEPTED);
    SEND_CONF_RESPONSE();
}

void OcppClient::RemoteStopTransactionConf(cJSON *obj)
{
    if(!obj)return;
    RemoteStopTransaction conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(REMOTE_STOP_ACCEPTED);
    SEND_CONF_RESPONSE();
}

void OcppClient::ResetConf(cJSON *obj)
{
    if(!obj)return;
    Reset conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(RESET_STATUS_ACCEPTED);
    SEND_CONF_RESPONSE();
}

void OcppClient::ChangeAvailabilityConf(cJSON *obj)
{
    if(!obj)return;
    ChangeAvailability conf;
//    ChangeAvailability req;
//    if(!req.parse(obj))return;

    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
//    conf.setConnectorId(req.connectorId());
    conf.setStatus(CHANGE_REJECTED);
    SEND_CONF_RESPONSE();
}

void OcppClient::GetConfigurationConf(cJSON *obj)
{
    if(!obj)return;
    GetConfiguration conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.addConfigKey("test",1,"ok");
    SEND_CONF_RESPONSE();
}

void OcppClient::ChangeConfigurationConf(cJSON *obj)
{
    if(!obj)return;
    ChangeConfiguration conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(CONFIG_ACCEPTED);
    SEND_CONF_RESPONSE();
}

void OcppClient::GetLocalListVersionConf(cJSON *obj)
{
    if(!obj)return;
    GetLocalListVersion conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setListVersion(1);
    SEND_CONF_RESPONSE();
}

void OcppClient::SendLocalListConf(cJSON *obj)
{
    if(!obj)return;
    SendLocalList conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(SEND_LIST_ACCEPTED);
    SEND_CONF_RESPONSE();
}

void OcppClient::ClearCacheConf(cJSON *obj)
{
    if(!obj)return;
    ClearCache conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(CLEAR_ACCEPTED);
    SEND_CONF_RESPONSE();

}

void OcppClient::GetCompositeScheduleConf(cJSON *obj)
{
    if(!obj)return;
    GetCompositeSchedule conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(COMPOSITE_ACCEPTED);
    conf.setConnectorId(1);
    conf.setScheduleStart(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs).toUtf8().constData());
    SEND_CONF_RESPONSE();
}

void OcppClient::ClearChargingProfileConf(cJSON *obj)
{
    if(!obj)return;
    ClearChargingProfile conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(CLEAR_PROFILE_ACCEPTED);
    SEND_CONF_RESPONSE();
}

void OcppClient::ReserveNowConf(cJSON *obj)
{
    if(!obj)return;
    ReserveNow conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(RESERVE_ACCEPTED);
    SEND_CONF_RESPONSE();
}

void OcppClient::CancelReservationConf(cJSON *obj)
{
    if(!obj)return;
    CancelReservation conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(CANCEL_ACCEPTED);//取消预约
    SEND_CONF_RESPONSE();
}

void OcppClient::GetDiagnosticsConf(cJSON *obj)
{
    if(!obj)return;
    GetDiagnostics conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setFileName("simple.txt");
    SEND_CONF_RESPONSE();
}

void OcppClient::DataTransferConf(cJSON *obj)
{
    if(!obj)return;
    DataTransfer conf;
    conf.buildConf();
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    conf.setMsgSeq(messageId);
    conf.setStatus(DATA_TRANSFER_ACCEPTED);//数据传输同意
    SEND_CONF_RESPONSE();
}

void OcppClient::handleServerCall(const QString &action, cJSON *payload)
{
    auto it = m_MapActionConf.find(action);
    if (it != m_MapActionConf.end()) {
        (this->*it.value())(payload);  // 调用对应的 Conf 处理函数
    } else {
        qWarning() << "[OcppClient] 未支持的服务器请求:" << action;
        // 可选：回一个 NotImplemented 错误
        NotImplementedConf(payload);
    }
}

void OcppClient::NotImplementedConf(cJSON *obj)
{
    if(!obj)return;
    QString messageId = cJSON_GetArrayItem(obj, 1)->valuestring;
    OcppError conf(messageId.toUtf8().constData(), OcppErrorCode::NotSupported);
    SEND_CONF_RESPONSE();
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
