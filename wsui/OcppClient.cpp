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
    qDebug() << "[OCPP] ↓ 收到:" << data;
    parseOcppMessage(data);
}

// ============================================================
// 解析OCPP消息（核心框架）
// ============================================================
void OcppClient::parseOcppMessage(const QByteArray &data)
{
    cJSON *root = cJSON_Parse(data.constData());
    if (!root || !cJSON_IsArray(root)) {
        qDebug() << "[OCPP] ❌ 格式错误";
        cJSON_Delete(root);
        return;
    }

    int msgTypeId = cJSON_GetArrayItem(root, 0)->valueint;
    QString messageId = cJSON_GetArrayItem(root, 1)->valuestring;

    switch (msgTypeId)
    {
    case 2: { // CALL：服务器发的请求
        QString action = cJSON_GetArrayItem(root, 2)->valuestring;
        char *payloadStr = cJSON_PrintUnformatted(cJSON_GetArrayItem(root, 3));
        QByteArray payload(payloadStr);
        free(payloadStr);

        handleCall(messageId, action, payload);
        break;
    }

    case 3: { // CALLRESULT：对我们请求的响应
        char *payloadStr = cJSON_PrintUnformatted(cJSON_GetArrayItem(root, 2));
        QByteArray payload(payloadStr);
        free(payloadStr);

        handleCallResult(messageId, payload);
        break;
    }

    case 4: { // CALLERROR：错误响应
        QString errorCode = cJSON_GetArrayItem(root, 2)->valuestring;
        QString errorDesc = cJSON_GetArrayItem(root, 3)->valuestring;

        handleCallError(messageId, errorCode, errorDesc);
        break;
    }

    default:
        qDebug() << "[OCPP] ❌ 未知消息类型:" << msgTypeId;
        break;
    }

    cJSON_Delete(root);
}

// ============================================================
// 处理服务器发来的请求（CALL）
// ============================================================
void OcppClient::handleCall(const QString &messageId, const QString &action,
                            const QByteArray &payload)
{
    qDebug() << "[OCPP] ← 收到请求:" << action << "ID:" << messageId;

    // 后面在这里按action分发处理
    // if (action == "Reset") { ... }
    // else if (action == "ChangeConfiguration") { ... }
}

// ============================================================
// 处理服务器对我们请求的响应（CALLRESULT）
// ============================================================
void OcppClient::handleCallResult(const QString &messageId, const QByteArray &payload)
{
    qDebug() << "[OCPP] ✓ 收到响应 ID:" << messageId;

    // 查找对应的请求
    QString action = m_pendingRequests.value(messageId);
    m_pendingRequests.remove(messageId);

    if (action.isEmpty()) {
        qDebug() << "[OCPP] 未知消息ID:" << messageId;
        return;
    }

    // 后面在这里按action分发处理响应
    // if (action == "BootNotification") { ... }
    // else if (action == "Heartbeat") { ... }
}

// ============================================================
// 处理错误响应（CALLERROR）
// ============================================================
void OcppClient::handleCallError(const QString &messageId, const QString &errorCode,
                                 const QString &errorDescription)
{
    qDebug() << "[OCPP] ✗ 错误 ID:" << messageId;
    qDebug() << "  错误码:" << errorCode;
    qDebug() << "  描述:" << errorDescription;

    m_pendingRequests.remove(messageId);
}

// ============================================================
// 发送请求（CALL）
// ============================================================
void OcppClient::sendCall(const QString &action, const QByteArray &payloadJson)
{
    if (!m_wsClient) return;

    QString messageId = generateMessageId();

    // 构建数组: [2, "messageId", "action", {payload}]
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToArray(array, cJSON_CreateNumber(2));
    cJSON_AddItemToArray(array, cJSON_CreateString(messageId.toUtf8().constData()));
    cJSON_AddItemToArray(array, cJSON_CreateString(action.toUtf8().constData()));
    cJSON_AddItemToArray(array, cJSON_Parse(payloadJson.constData()));

    char *msgStr = cJSON_PrintUnformatted(array);
    QByteArray message(msgStr);
    free(msgStr);
    cJSON_Delete(array);

    m_pendingRequests[messageId] = action;

    qDebug() << "[OCPP] → 发送:" << action << "ID:" << messageId;
    m_wsClient->sendTextMessage(QString::fromUtf8(message));
}

// ============================================================
// 发送响应（CALLRESULT）
// ============================================================
void OcppClient::sendCallResult(const QString &messageId, const QByteArray &payloadJson)
{
    if (!m_wsClient) return;

    // 构建数组: [3, "messageId", {payload}]
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToArray(array, cJSON_CreateNumber(3));
    cJSON_AddItemToArray(array, cJSON_CreateString(messageId.toUtf8().constData()));
    cJSON_AddItemToArray(array, cJSON_Parse(payloadJson.constData()));

    char *msgStr = cJSON_PrintUnformatted(array);
    QByteArray message(msgStr);
    free(msgStr);
    cJSON_Delete(array);

    qDebug() << "[OCPP] → 响应 ID:" << messageId;
    m_wsClient->sendTextMessage(QString::fromUtf8(message));
}
