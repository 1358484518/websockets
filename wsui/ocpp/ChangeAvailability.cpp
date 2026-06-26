#include "ChangeAvailability.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Conversion Helpers =====

const char* ChangeAvailability::availTypeToString(quint8 t) const
{
    switch (t) {
        case AVAIL_OPERATIVE:   return "Operative";
        case AVAIL_INOPERATIVE: return "Inoperative";
        default: return "Inoperative";
    }
}

quint8 ChangeAvailability::stringToAvailType(const char *str) const
{
    if (!str) return AVAIL_INOPERATIVE;
    if (strcmp(str, "Operative") == 0)   return AVAIL_OPERATIVE;
    if (strcmp(str, "Inoperative") == 0) return AVAIL_INOPERATIVE;
    return AVAIL_INOPERATIVE;
}

const char* ChangeAvailability::statusToString(quint8 s) const
{
    switch (s) {
        case CHANGE_ACCEPTED:  return "Accepted";
        case CHANGE_REJECTED:  return "Rejected";
        case CHANGE_SCHEDULED: return "Scheduled";
        default: return "Rejected";
    }
}

quint8 ChangeAvailability::stringToStatus(const char *str) const
{
    if (!str) return CHANGE_REJECTED;
    if (strcmp(str, "Accepted") == 0)  return CHANGE_ACCEPTED;
    if (strcmp(str, "Rejected") == 0)  return CHANGE_REJECTED;
    if (strcmp(str, "Scheduled") == 0) return CHANGE_SCHEDULED;
    return CHANGE_REJECTED;
}

// ===== Construction / Destruction =====

ChangeAvailability::ChangeAvailability()
    : m_root(nullptr), m_type(0), m_availType(AVAIL_INOPERATIVE), m_status(CHANGE_REJECTED), m_connectorId(0) {}

ChangeAvailability::ChangeAvailability(const char *value)
    : m_root(nullptr), m_type(0), m_availType(AVAIL_INOPERATIVE), m_status(CHANGE_REJECTED), m_connectorId(0)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

ChangeAvailability::ChangeAvailability(cJSON *obj)
    : m_root(nullptr), m_type(0), m_availType(AVAIL_INOPERATIVE), m_status(CHANGE_REJECTED), m_connectorId(0)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

ChangeAvailability::~ChangeAvailability()
{
    clear();
}

ChangeAvailability::ChangeAvailability(ChangeAvailability&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_availType(other.m_availType), m_status(other.m_status), m_connectorId(other.m_connectorId)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_availType = AVAIL_INOPERATIVE;
    other.m_status = CHANGE_REJECTED;
    other.m_connectorId = 0;
}

ChangeAvailability& ChangeAvailability::operator=(ChangeAvailability&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_availType = other.m_availType;
        m_status = other.m_status;
        m_connectorId = other.m_connectorId;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_availType = AVAIL_INOPERATIVE;
        other.m_status = CHANGE_REJECTED;
        other.m_connectorId = 0;
    }
    return *this;
}

void ChangeAvailability::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_availType = AVAIL_INOPERATIVE;
    m_status = CHANGE_REJECTED;
    m_connectorId = 0;
}

// ===== Internal Helpers =====

bool ChangeAvailability::ensurePayload()
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

bool ChangeAvailability::setStringField(const char *key, const char *value)
{
    if (!key || !value) return false;
    if (!ensurePayload()) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (item && cJSON_IsString(item)) {
        char *newStr = strdup(value);
        if (!newStr) return false;
        free(item->valuestring);
        item->valuestring = newStr;
        return true;
    }

    if (item) {
        cJSON_DeleteItemFromObject(payload, key);
    }

    cJSON *newItem = cJSON_CreateString(value);
    if (!newItem) return false;

    cJSON_AddItemToObject(payload, key, newItem);
    return true;
}

bool ChangeAvailability::setNumberField(const char *key, double value)
{
    if (!key) return false;
    if (!ensurePayload()) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = value;
        item->valueint = (int)value;
        return true;
    }

    if (item) {
        cJSON_DeleteItemFromObject(payload, key);
    }

    cJSON *newItem = cJSON_CreateNumber(value);
    if (!newItem) return false;

    cJSON_AddItemToObject(payload, key, newItem);
    return true;
}

const char* ChangeAvailability::getStringField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return "";
    if (!key) return "";

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return "";

    cJSON *item = cJSON_GetObjectItem(payload, key);
    return item && cJSON_IsString(item) ? item->valuestring : "";
}

double ChangeAvailability::getNumberField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;
    if (!key) return 0;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    return item && cJSON_IsNumber(item) ? item->valuedouble : 0;
}

// ===== Build =====

bool ChangeAvailability::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("ChangeAvailability");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }

    cJSON *connItem = cJSON_CreateNumber(0);
    if (!connItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "connectorId", connItem);

    cJSON *typePayloadItem = cJSON_CreateString("Inoperative");
    if (!typePayloadItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "type", typePayloadItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    m_connectorId = 0;
    m_availType = AVAIL_INOPERATIVE;
    return true;
}

bool ChangeAvailability::buildConf()
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

    cJSON *statusItem = cJSON_CreateString("Accepted");
    if (!statusItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "status", statusItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALLRESULT;
    m_status = CHANGE_ACCEPTED;
    return true;
}

bool ChangeAvailability::setMsgSeq(quint64 i)
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

// ===== Common Setters =====

bool ChangeAvailability::setMsgSeq(QString i)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;

//    char buf[32];
//    snprintf(buf, sizeof(buf), "%llu", (unsigned long long)i);

    cJSON *newItem = cJSON_CreateString(i.toUtf8().constData());
    if (!newItem) return false;

    cJSON *oldItem = cJSON_GetArrayItem(m_root, 1);
    if (oldItem) {
        cJSON_ReplaceItemInArray(m_root, 1, newItem);
    } else {
        cJSON_InsertItemInArray(m_root, 1, newItem);
    }

    return true;
}

bool ChangeAvailability::setType(quint8 type)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    if (type != OCPP_CALL && type != OCPP_CALLRESULT) return false;

    if (m_type == type) return true;

    cJSON *typeItem = cJSON_GetArrayItem(m_root, 0);
    if (!typeItem || !cJSON_IsNumber(typeItem)) return false;

    // Adjust array structure when switching between CALL and CALLRESULT
    // CALL:        [type, id, action, payload]  (4 elements)
    // CALLRESULT:  [type, id, payload]          (3 elements)
    if (m_type == OCPP_CALL && type == OCPP_CALLRESULT) {
        cJSON *actionItem = cJSON_GetArrayItem(m_root, 2);
        if (actionItem) {
            cJSON_DetachItemFromArray(m_root, 2);
            cJSON_Delete(actionItem);
        }
    } else if (m_type == OCPP_CALLRESULT && type == OCPP_CALL) {
        cJSON *actionItem = cJSON_CreateString("ChangeAvailability");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool ChangeAvailability::setConnectorId(qint32 id)
{
    if (m_type != OCPP_CALL) return false;
    if (!setNumberField("connectorId", (double)id)) return false;
    m_connectorId = id;
    return true;
}

bool ChangeAvailability::setAvailabilityType(quint8 type)
{
    if (m_type != OCPP_CALL) return false;
    if (type != AVAIL_OPERATIVE && type != AVAIL_INOPERATIVE) return false;

    if (!setStringField("type", availTypeToString(type))) return false;
    m_availType = type;
    return true;
}

// ===== Response Setters =====

bool ChangeAvailability::setStatus(quint8 s)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!ensurePayload()) return false;

    int payloadIndex = 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "status");
    if (item && cJSON_IsString(item)) {
        char *newStr = strdup(statusToString(s));
        if (!newStr) return false;
        free(item->valuestring);
        item->valuestring = newStr;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "status");
        cJSON *newItem = cJSON_CreateString(statusToString(s));
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "status", newItem);
    }

    m_status = s;
    return true;
}

// ===== Getters =====

quint64 ChangeAvailability::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 ChangeAvailability::type() const
{
    return m_type;
}

qint32 ChangeAvailability::connectorId() const
{
    if (m_type != OCPP_CALL) return 0;
    return m_connectorId;
}

quint8 ChangeAvailability::availabilityType() const
{
    if (m_type != OCPP_CALL) return AVAIL_INOPERATIVE;
    return m_availType;
}

const char* ChangeAvailability::availabilityTypeString() const
{
    return availTypeToString(m_availType);
}

quint8 ChangeAvailability::status() const
{
    return m_status;
}

const char* ChangeAvailability::statusString() const
{
    return statusToString(m_status);
}

bool ChangeAvailability::isCall() const
{
    return m_type == OCPP_CALL;
}

bool ChangeAvailability::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool ChangeAvailability::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool ChangeAvailability::parse(const char *value)
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

bool ChangeAvailability::parse(cJSON *obj)
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

    quint8 availType = AVAIL_INOPERATIVE;
    quint8 status = CHANGE_REJECTED;
    qint32 connectorId = 0;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "ChangeAvailability") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // Required fields
        cJSON *conn = cJSON_GetObjectItem(payload, "connectorId");
        if (!conn || !cJSON_IsNumber(conn)) return false;
        connectorId = (qint32)conn->valueint;

        cJSON *typePayload = cJSON_GetObjectItem(payload, "type");
        if (!typePayload || !cJSON_IsString(typePayload)) return false;
        availType = stringToAvailType(typePayload->valuestring);
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;

        cJSON *statusItem = cJSON_GetObjectItem(payload, "status");
        if (statusItem && cJSON_IsString(statusItem)) {
            status = stringToStatus(statusItem->valuestring);
        }
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_availType = availType;
    m_status = status;
    m_connectorId = connectorId;

    return true;
}

// ===== Serialize =====

char* ChangeAvailability::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void ChangeAvailability::print() const
{
    char *data = toJson();
    if (data) {
        printf("[ChangeAvailability] %s\n", data);
        free(data);
    }
}

#if 0
// 解析收到的更改可用性请求
ChangeAvailability req;
if (req.parse(receivedJson)) {
    qint32 connId = req.connectorId();
    quint8 availType = req.availabilityType();

    printf("Change connector %d to %s\n",
           connId, req.availabilityTypeString());

    // 构建响应
    ChangeAvailability conf;
    conf.buildConf();
    conf.setMsgSeq(req.msgSeq());

    if (changeSuccess) {
        conf.setStatus(CHANGE_ACCEPTED);
    } else if (changeScheduled) {
        conf.setStatus(CHANGE_SCHEDULED);
    } else {
        conf.setStatus(CHANGE_REJECTED);
    }

    char *response = conf.toJson();
    // 发送响应...
    free(response);
}
#endif
