#include "GetDiagnostics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GetDiagnostics::GetDiagnostics()
    : m_root(nullptr), m_type(0)
{
}

GetDiagnostics::GetDiagnostics(const char *value)
    : m_root(nullptr), m_type(0)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

GetDiagnostics::GetDiagnostics(cJSON *obj)
    : m_root(nullptr), m_type(0)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

GetDiagnostics::~GetDiagnostics()
{
    clear();
}

GetDiagnostics::GetDiagnostics(GetDiagnostics&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type)
{
    other.m_root = nullptr;
    other.m_type = 0;
}

GetDiagnostics& GetDiagnostics::operator=(GetDiagnostics&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        other.m_root = nullptr;
        other.m_type = 0;
    }
    return *this;
}

void GetDiagnostics::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
}

bool GetDiagnostics::ensurePayload()
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

bool GetDiagnostics::setStringField(const char *key, const char *value)
{
    if (!key) return false;
    if (!ensurePayload()) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return false;

    if (value && *value != '\0') {
        cJSON *item = cJSON_GetObjectItem(payload, key);
        if (item && cJSON_IsString(item)) {
            cJSON *newItem = cJSON_CreateString(value);
            if (!newItem) return false;
            cJSON_ReplaceItemInObject(payload, key, newItem);
        } else {
            if (item) cJSON_DeleteItemFromObject(payload, key);
            cJSON *newItem = cJSON_CreateString(value);
            if (!newItem) return false;
            cJSON_AddItemToObject(payload, key, newItem);
        }
    } else {
        cJSON_DeleteItemFromObject(payload, key);
    }

    return true;
}

bool GetDiagnostics::setNumberField(const char *key, qreal value)
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
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, key);
        cJSON *newItem = cJSON_CreateNumber(value);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, key, newItem);
    }

    return true;
}

bool GetDiagnostics::clearField(const char *key)
{
    if (!key) return false;
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return false;

    cJSON_DeleteItemFromObject(payload, key);
    return true;
}

const char* GetDiagnostics::getStringField(const char *key) const
{
    if (!key || !m_root) return "";

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return "";

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

qreal GetDiagnostics::getNumberField(const char *key) const
{
    if (!key || !m_root) return 0;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (!item || !cJSON_IsNumber(item)) return 0;

    return item->valuedouble;
}

bool GetDiagnostics::hasField(const char *key) const
{
    if (!key || !m_root) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    return item != nullptr;
}

bool GetDiagnostics::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("GetDiagnostics");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool GetDiagnostics::buildConf()
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

bool GetDiagnostics::setMsgSeq(quint64 i)
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

bool GetDiagnostics::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("GetDiagnostics");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request setters =====

bool GetDiagnostics::setLocation(const char *location)
{
    if (m_type != OCPP_CALL) return false;
    if (!location || *location == '\0') return false;
    return setStringField("location", location);
}

bool GetDiagnostics::setRetries(qint32 retries)
{
    if (m_type != OCPP_CALL) return false;
    return setNumberField("retries", (qreal)retries);
}

bool GetDiagnostics::clearRetries()
{
    if (m_type != OCPP_CALL) return false;
    return clearField("retries");
}

bool GetDiagnostics::setRetryInterval(qint32 interval)
{
    if (m_type != OCPP_CALL) return false;
    return setNumberField("retryInterval", (qreal)interval);
}

bool GetDiagnostics::clearRetryInterval()
{
    if (m_type != OCPP_CALL) return false;
    return clearField("retryInterval");
}

bool GetDiagnostics::setStartTime(const char *time)
{
    if (m_type != OCPP_CALL) return false;
    return setStringField("startTime", time);
}

bool GetDiagnostics::setStopTime(const char *time)
{
    if (m_type != OCPP_CALL) return false;
    return setStringField("stopTime", time);
}

// ===== Response setters =====

bool GetDiagnostics::setFileName(const char *fileName)
{
    if (m_type != OCPP_CALLRESULT) return false;
    return setStringField("fileName", fileName);
}

// ===== Getters =====

quint64 GetDiagnostics::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 GetDiagnostics::type() const
{
    return m_type;
}

const char* GetDiagnostics::location() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("location");
}

qint32 GetDiagnostics::retries() const
{
    if (m_type != OCPP_CALL) return 0;
    return (qint32)getNumberField("retries");
}

bool GetDiagnostics::hasRetries() const
{
    if (m_type != OCPP_CALL) return false;
    return hasField("retries");
}

qint32 GetDiagnostics::retryInterval() const
{
    if (m_type != OCPP_CALL) return 0;
    return (qint32)getNumberField("retryInterval");
}

bool GetDiagnostics::hasRetryInterval() const
{
    if (m_type != OCPP_CALL) return false;
    return hasField("retryInterval");
}

const char* GetDiagnostics::startTime() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("startTime");
}

const char* GetDiagnostics::stopTime() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("stopTime");
}

const char* GetDiagnostics::fileName() const
{
    if (m_type != OCPP_CALLRESULT) return "";
    return getStringField("fileName");
}

bool GetDiagnostics::isCall() const
{
    return m_type == OCPP_CALL;
}

bool GetDiagnostics::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool GetDiagnostics::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool GetDiagnostics::parse(const char *value)
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

bool GetDiagnostics::parse(cJSON *obj)
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

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "GetDiagnostics") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // location is required
        cJSON *loc = cJSON_GetObjectItem(payload, "location");
        if (!loc || !cJSON_IsString(loc)) return false;
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;
    }

    clear();
    m_root = obj;
    m_type = msgType;

    return true;
}

// ===== Serialize =====

char* GetDiagnostics::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void GetDiagnostics::print() const
{
    char *data = toJson();
    if (data) {
        printf("[GetDiagnostics] %s\n", data);
        free(data);
    }
}
