#include "remotestoptransaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

RemoteStopTransaction::RemoteStopTransaction()
    : m_root(nullptr), m_type(0), m_status(REMOTE_STOP_REJECTED)
{
}

RemoteStopTransaction::RemoteStopTransaction(const char *value)
    : m_root(nullptr), m_type(0), m_status(REMOTE_STOP_REJECTED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

RemoteStopTransaction::RemoteStopTransaction(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(REMOTE_STOP_REJECTED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

RemoteStopTransaction::~RemoteStopTransaction()
{
    clear();
}

RemoteStopTransaction::RemoteStopTransaction(RemoteStopTransaction&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = REMOTE_STOP_REJECTED;
}

RemoteStopTransaction& RemoteStopTransaction::operator=(RemoteStopTransaction&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = REMOTE_STOP_REJECTED;
    }
    return *this;
}

void RemoteStopTransaction::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = REMOTE_STOP_REJECTED;
}

bool RemoteStopTransaction::ensurePayload()
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);

    if (payload && cJSON_IsObject(payload)) {
        return true;
    }

    if (payload) {
        cJSON_DeleteItemFromArray(m_root, payloadIndex);
    }

    cJSON *newPayload = cJSON_CreateObject();
    if (!newPayload) return false;

    cJSON_InsertItemInArray(m_root, payloadIndex, newPayload);
    return true;
}

const char* RemoteStopTransaction::statusToString(quint8 status)
{
    switch (status) {
        case REMOTE_STOP_ACCEPTED: return "Accepted";
        case REMOTE_STOP_REJECTED: return "Rejected";
        default: return "Rejected";
    }
}

quint8 RemoteStopTransaction::stringToStatus(const char *str)
{
    if (!str) return REMOTE_STOP_REJECTED;
    if (strcmp(str, "Accepted") == 0) return REMOTE_STOP_ACCEPTED;
    return REMOTE_STOP_REJECTED;
}

bool RemoteStopTransaction::buildReq()
{
    clear();

    m_root = cJSON_CreateArray();
    if (!m_root) return false;

    cJSON *typeItem = cJSON_CreateNumber(OCPP_CALL);
    if (!typeItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, typeItem);

    cJSON *idItem = cJSON_CreateString("0");
    if (!idItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, idItem);

    cJSON *actionItem = cJSON_CreateString("RemoteStopTransaction");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool RemoteStopTransaction::buildConf()
{
    clear();

    m_root = cJSON_CreateArray();
    if (!m_root) return false;

    cJSON *typeItem = cJSON_CreateNumber(OCPP_CALLRESULT);
    if (!typeItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, typeItem);

    cJSON *idItem = cJSON_CreateString("0");
    if (!idItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, idItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALLRESULT;
    return true;
}

bool RemoteStopTransaction::setMsgSeq(quint64 i)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    char buf[32];
    snprintf(buf, sizeof(buf), "%llu", (unsigned long long)i);

    cJSON *newItem = cJSON_CreateString(buf);
    if (!newItem) return false;

    cJSON *oldItem = cJSON_GetArrayItem(m_root, 1);
    if (oldItem) {
        cJSON_ReplaceItemInArray(m_root, 1, newItem);
    } else {
        cJSON_InsertItemInArray(m_root, 1, newItem);
    }

    return true;
}

bool RemoteStopTransaction::setType(quint8 type)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    if (type != OCPP_CALL && type != OCPP_CALLRESULT) return false;

    if (m_type == type) return true;

    cJSON *typeItem = cJSON_GetArrayItem(m_root, 0);
    if (!typeItem || !cJSON_IsNumber(typeItem)) return false;

    if (m_type == OCPP_CALL && type == OCPP_CALLRESULT) {
        cJSON *actionItem = cJSON_GetArrayItem(m_root, 2);
        if (actionItem) {
            cJSON_DetachItemFromArray(m_root, 2);
            cJSON_Delete(actionItem);
        }
    } else if (m_type == OCPP_CALLRESULT && type == OCPP_CALL) {
        cJSON *actionItem = cJSON_CreateString("RemoteStopTransaction");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request setters =====

bool RemoteStopTransaction::setTransactionId(qint32 transactionId)
{
    if (m_type != OCPP_CALL) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "transactionId");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)transactionId;
        item->valueint = transactionId;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "transactionId");
        cJSON *newItem = cJSON_CreateNumber((qreal)transactionId);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "transactionId", newItem);
    }

    return true;
}

// ===== Response setters =====

bool RemoteStopTransaction::setStatus(quint8 status)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    const char *statusStr = statusToString(status);
    cJSON *item = cJSON_GetObjectItem(payload, "status");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(payload, "status", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "status");
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "status", newItem);
    }

    m_status = status;
    return true;
}

// ===== Getters =====

quint64 RemoteStopTransaction::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 RemoteStopTransaction::type() const
{
    return m_type;
}

qint32 RemoteStopTransaction::transactionId() const
{
    if (m_type != OCPP_CALL || !m_root) return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "transactionId");
    if (!item || !cJSON_IsNumber(item)) return 0;

    return (qint32)item->valuedouble;
}

quint8 RemoteStopTransaction::status() const
{
    return m_status;
}

const char* RemoteStopTransaction::statusString() const
{
    return statusToString(m_status);
}

bool RemoteStopTransaction::isCall() const
{
    return m_type == OCPP_CALL;
}

bool RemoteStopTransaction::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool RemoteStopTransaction::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool RemoteStopTransaction::parse(const char *value)
{
    if (!value || *value == '\0') return false;

    cJSON *obj = cJSON_Parse(value);
    if (!obj) return false;

    bool ok = parse(obj);
    if (!ok) {
        cJSON_Delete(obj);
    }
    return ok;
}

bool RemoteStopTransaction::parse(cJSON *obj)
{
    if (!obj || !cJSON_IsArray(obj)) return false;

    int size = cJSON_GetArraySize(obj);
    if (size < 3) return false;

    cJSON *typeItem = cJSON_GetArrayItem(obj, 0);
    if (!typeItem || !cJSON_IsNumber(typeItem)) return false;

    quint8 msgType = (quint8)typeItem->valueint;
    if (msgType != OCPP_CALL && msgType != OCPP_CALLRESULT) return false;

    cJSON *idItem = cJSON_GetArrayItem(obj, 1);
    if (!idItem || !cJSON_IsString(idItem)) return false;

    quint8 parsedStatus = REMOTE_STOP_REJECTED;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "RemoteStopTransaction") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // transactionId is required
        cJSON *txItem = cJSON_GetObjectItem(payload, "transactionId");
        if (!txItem || !cJSON_IsNumber(txItem)) return false;
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;

        cJSON *statusItem = cJSON_GetObjectItem(payload, "status");
        if (!statusItem || !cJSON_IsString(statusItem)) return false;

        parsedStatus = stringToStatus(statusItem->valuestring);
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_status = parsedStatus;

    return true;
}

// ===== Serialize =====

char* RemoteStopTransaction::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void RemoteStopTransaction::print() const
{
    char *data = toJson();
    if (data) {
        printf("[RemoteStopTransaction] %s\n", data);
        free(data);
    }
}

//请求（CALL）：[2, msgId, "RemoteStopTransaction", {"transactionId": 12345}]
//响应（CALLRESULT）：[3, msgId, {"status": "Accepted"}]
//状态枚举：REMOTE_STOP_ACCEPTED / REMOTE_STOP_REJECTED
