#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include "TriggerMessage.h"

// ===== Constructors & Destructor =====

TriggerMessage::TriggerMessage()
    : m_root(nullptr), m_type(0), m_requestedMessage(REQ_MSG_HEARTBEAT),
      m_status(TRIGGER_ACCEPTED)
{
}

TriggerMessage::TriggerMessage(const char *value)
    : m_root(nullptr), m_type(0), m_requestedMessage(REQ_MSG_HEARTBEAT),
      m_status(TRIGGER_ACCEPTED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

TriggerMessage::TriggerMessage(cJSON *obj)
    : m_root(nullptr), m_type(0), m_requestedMessage(REQ_MSG_HEARTBEAT),
      m_status(TRIGGER_ACCEPTED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

TriggerMessage::~TriggerMessage()
{
    clear();
}

// ===== Move Semantics =====

TriggerMessage::TriggerMessage(TriggerMessage&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type),
      m_requestedMessage(other.m_requestedMessage), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_requestedMessage = REQ_MSG_HEARTBEAT;
    other.m_status = TRIGGER_ACCEPTED;
}

TriggerMessage& TriggerMessage::operator=(TriggerMessage&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_requestedMessage = other.m_requestedMessage;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_requestedMessage = REQ_MSG_HEARTBEAT;
        other.m_status = TRIGGER_ACCEPTED;
    }
    return *this;
}

// ===== Clear =====

void TriggerMessage::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_requestedMessage = REQ_MSG_HEARTBEAT;
    m_status = TRIGGER_ACCEPTED;
}

// ===== Internal Helpers =====

bool TriggerMessage::ensurePayload()
{
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);

    if (payload && cJSON_IsObject(payload)) {
        return true;
    }

    if (payload) {
        cJSON_DeleteItemFromArray(m_root, payloadIndex);
    }

    cJSON *newPayload = cJSON_CreateObject();
    if (!newPayload)
        return false;

    cJSON_InsertItemInArray(m_root, payloadIndex, newPayload);
    return true;
}

// ===== Enum Conversions =====

const char* TriggerMessage::statusToString(quint8 status)
{
    switch (status) {
        case TRIGGER_ACCEPTED:         return "Accepted";
        case TRIGGER_REJECTED:         return "Rejected";
        case TRIGGER_NOT_IMPLEMENTED:  return "NotImplemented";
        default:                       return "Accepted";
    }
}

quint8 TriggerMessage::stringToStatus(const char *str)
{
    if (!str)
        return TRIGGER_ACCEPTED;
    if (strcmp(str, "Accepted") == 0)
        return TRIGGER_ACCEPTED;
    if (strcmp(str, "Rejected") == 0)
        return TRIGGER_REJECTED;
    if (strcmp(str, "NotImplemented") == 0)
        return TRIGGER_NOT_IMPLEMENTED;
    return TRIGGER_ACCEPTED;
}

const char* TriggerMessage::requestedMessageToString(quint8 msgType)
{
    switch (msgType) {
        case REQ_MSG_BOOT_NOTIFICATION:                 return "BootNotification";
        case REQ_MSG_DIAGNOSTICS_STATUS_NOTIFICATION:   return "DiagnosticsStatusNotification";
        case REQ_MSG_FIRMWARE_STATUS_NOTIFICATION:      return "FirmwareStatusNotification";
        case REQ_MSG_HEARTBEAT:                         return "Heartbeat";
        case REQ_MSG_METER_VALUES:                      return "MeterValues";
        case REQ_MSG_STATUS_NOTIFICATION:               return "StatusNotification";
        default:                                        return "Heartbeat";
    }
}

quint8 TriggerMessage::stringToRequestedMessage(const char *str)
{
    if (!str)
        return REQ_MSG_HEARTBEAT;
    if (strcmp(str, "BootNotification") == 0)
        return REQ_MSG_BOOT_NOTIFICATION;
    if (strcmp(str, "DiagnosticsStatusNotification") == 0)
        return REQ_MSG_DIAGNOSTICS_STATUS_NOTIFICATION;
    if (strcmp(str, "FirmwareStatusNotification") == 0)
        return REQ_MSG_FIRMWARE_STATUS_NOTIFICATION;
    if (strcmp(str, "Heartbeat") == 0)
        return REQ_MSG_HEARTBEAT;
    if (strcmp(str, "MeterValues") == 0)
        return REQ_MSG_METER_VALUES;
    if (strcmp(str, "StatusNotification") == 0)
        return REQ_MSG_STATUS_NOTIFICATION;
    return REQ_MSG_HEARTBEAT;
}

// ===== Build =====

bool TriggerMessage::buildReq()
{
    clear();

    m_root = cJSON_CreateArray();
    if (!m_root)
        return false;

    cJSON *typeItem = cJSON_CreateNumber(OCPP_CALL);
    if (!typeItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, typeItem);

    cJSON *idItem = cJSON_CreateString("0");
    if (!idItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, idItem);

    cJSON *actionItem = cJSON_CreateString("TriggerMessage");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool TriggerMessage::buildConf()
{
    clear();

    m_root = cJSON_CreateArray();
    if (!m_root)
        return false;

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

// ===== Common Setters =====

bool TriggerMessage::setMsgSeq(quint64 i)
{
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    char buf[32];
    snprintf(buf, sizeof(buf), "%llu", (unsigned long long)i);

    cJSON *newItem = cJSON_CreateString(buf);
    if (!newItem)
        return false;

    cJSON *oldItem = cJSON_GetArrayItem(m_root, 1);
    if (oldItem) {
        cJSON_ReplaceItemInArray(m_root, 1, newItem);
    } else {
        cJSON_InsertItemInArray(m_root, 1, newItem);
    }

    return true;
}

bool TriggerMessage::setMsgSeq(QString i)
{
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    cJSON *newItem = cJSON_CreateString(i.toUtf8().constData());
    if (!newItem)
        return false;

    cJSON *oldItem = cJSON_GetArrayItem(m_root, 1);
    if (oldItem) {
        cJSON_ReplaceItemInArray(m_root, 1, newItem);
    } else {
        cJSON_InsertItemInArray(m_root, 1, newItem);
    }

    return true;
}
bool TriggerMessage::setType(quint8 type)
{
    if (!m_root || !cJSON_IsArray(m_root))
        return false;
    if (type != OCPP_CALL && type != OCPP_CALLRESULT)
        return false;

    if (m_type == type)
        return true;

    cJSON *typeItem = cJSON_GetArrayItem(m_root, 0);
    if (!typeItem || !cJSON_IsNumber(typeItem))
        return false;

    if (m_type == OCPP_CALL && type == OCPP_CALLRESULT) {
        cJSON *actionItem = cJSON_GetArrayItem(m_root, 2);
        if (actionItem) {
            cJSON_DetachItemFromArray(m_root, 2);
            cJSON_Delete(actionItem);
        }
    } else if (m_type == OCPP_CALLRESULT && type == OCPP_CALL) {
        cJSON *actionItem = cJSON_CreateString("TriggerMessage");
        if (!actionItem)
            return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool TriggerMessage::setRequestedMessage(quint8 msgType)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!ensurePayload())
        return false;

    const char *msgStr = requestedMessageToString(msgType);
    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "requestedMessage");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(msgStr);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(payload, "requestedMessage", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "requestedMessage");
        cJSON *newItem = cJSON_CreateString(msgStr);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "requestedMessage", newItem);
    }

    m_requestedMessage = msgType;
    return true;
}

bool TriggerMessage::setConnectorId(qint32 connectorId)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!ensurePayload())
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)connectorId;
        item->valueint = (int)connectorId;
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "connectorId");
        cJSON *newItem = cJSON_CreateNumber((qreal)connectorId);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "connectorId", newItem);
    }

    return true;
}

bool TriggerMessage::clearConnectorId()
{
    if (m_type != OCPP_CALL)
        return false;
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    if (!item)
        return true;

    cJSON_DeleteItemFromObject(payload, "connectorId");
    return true;
}

// ===== Response Setters =====

bool TriggerMessage::setStatus(quint8 status)
{
    if (m_type != OCPP_CALLRESULT)
        return false;
    if (!ensurePayload())
        return false;

    const char *statusStr = statusToString(status);
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "status");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(payload, "status", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "status");
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "status", newItem);
    }

    m_status = status;
    return true;
}

// ===== Getters =====

quint64 TriggerMessage::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem))
        return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 TriggerMessage::type() const
{
    return m_type;
}

quint8 TriggerMessage::requestedMessage() const
{
    return m_requestedMessage;
}

const char* TriggerMessage::requestedMessageString() const
{
    return requestedMessageToString(m_requestedMessage);
}

qint32 TriggerMessage::connectorId() const
{
    if (m_type != OCPP_CALL)
        return 0;
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    if (!item || !cJSON_IsNumber(item))
        return 0;

    return (qint32)item->valuedouble;
}

bool TriggerMessage::hasConnectorId() const
{
    if (m_type != OCPP_CALL)
        return false;
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    return cJSON_GetObjectItem(payload, "connectorId") != nullptr;
}

quint8 TriggerMessage::status() const
{
    return m_status;
}

const char* TriggerMessage::statusString() const
{
    return statusToString(m_status);
}

bool TriggerMessage::isCall() const
{
    return m_type == OCPP_CALL;
}

bool TriggerMessage::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool TriggerMessage::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool TriggerMessage::parse(const char *value)
{
    if (!value || *value == '\0')
        return false;

    cJSON *obj = cJSON_Parse(value);
    if (!obj)
        return false;

    bool ok = parse(obj);
    if (!ok) {
        cJSON_Delete(obj);
    }
    return ok;
}

bool TriggerMessage::parse(cJSON *obj)
{
    if (!obj || !cJSON_IsArray(obj))
        return false;

    int size = cJSON_GetArraySize(obj);
    if (size < 3)
        return false;

    cJSON *typeItem = cJSON_GetArrayItem(obj, 0);
    if (!typeItem || !cJSON_IsNumber(typeItem))
        return false;

    quint8 msgType = (quint8)typeItem->valueint;
    if (msgType != OCPP_CALL && msgType != OCPP_CALLRESULT)
        return false;

    cJSON *idItem = cJSON_GetArrayItem(obj, 1);
    if (!idItem || !cJSON_IsString(idItem))
        return false;

    quint8 parsedRequestedMsg = REQ_MSG_HEARTBEAT;
    quint8 parsedStatus = TRIGGER_ACCEPTED;

    if (msgType == OCPP_CALL) {
        if (size < 4)
            return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem))
            return false;
        if (strcmp(actionItem->valuestring, "TriggerMessage") != 0)
            return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Required: requestedMessage
        cJSON *reqMsgItem = cJSON_GetObjectItem(payload, "requestedMessage");
        if (!reqMsgItem || !cJSON_IsString(reqMsgItem))
            return false;

        parsedRequestedMsg = stringToRequestedMessage(reqMsgItem->valuestring);

        // Optional: connectorId - no validation needed
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Required: status
        cJSON *statusItem = cJSON_GetObjectItem(payload, "status");
        if (!statusItem || !cJSON_IsString(statusItem))
            return false;

        parsedStatus = stringToStatus(statusItem->valuestring);
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_requestedMessage = parsedRequestedMsg;
    m_status = parsedStatus;

    return true;
}

// ===== Serialize =====

char* TriggerMessage::toJson() const
{
    if (!m_root)
        return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void TriggerMessage::print() const
{
    char *data = toJson();
    if (data) {
        printf("[TriggerMessage] %s\n", data);
        free(data);
    }
}
