#include "getlocallistversion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GetLocalListVersion::GetLocalListVersion()
    : m_root(nullptr), m_type(0)
{
}

GetLocalListVersion::GetLocalListVersion(const char *value)
    : m_root(nullptr), m_type(0)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

GetLocalListVersion::GetLocalListVersion(cJSON *obj)
    : m_root(nullptr), m_type(0)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

GetLocalListVersion::~GetLocalListVersion()
{
    clear();
}

GetLocalListVersion::GetLocalListVersion(GetLocalListVersion&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type)
{
    other.m_root = nullptr;
    other.m_type = 0;
}

GetLocalListVersion& GetLocalListVersion::operator=(GetLocalListVersion&& other) noexcept
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

void GetLocalListVersion::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
}

bool GetLocalListVersion::ensurePayload()
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

bool GetLocalListVersion::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("GetLocalListVersion");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool GetLocalListVersion::buildConf()
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

bool GetLocalListVersion::setMsgSeq(quint64 i)
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
bool GetLocalListVersion::setMsgSeq(QString i)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;

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
bool GetLocalListVersion::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("GetLocalListVersion");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Response setters =====

bool GetLocalListVersion::setListVersion(qint32 version)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "listVersion");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)version;
        item->valueint = version;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "listVersion");
        cJSON *newItem = cJSON_CreateNumber((qreal)version);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "listVersion", newItem);
    }

    return true;
}

// ===== Getters =====

quint64 GetLocalListVersion::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 GetLocalListVersion::type() const
{
    return m_type;
}

qint32 GetLocalListVersion::listVersion() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "listVersion");
    if (!item || !cJSON_IsNumber(item)) return 0;

    return (qint32)item->valuedouble;
}

bool GetLocalListVersion::isCall() const
{
    return m_type == OCPP_CALL;
}

bool GetLocalListVersion::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool GetLocalListVersion::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool GetLocalListVersion::parse(const char *value)
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

bool GetLocalListVersion::parse(cJSON *obj)
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
        if (strcmp(actionItem->valuestring, "GetLocalListVersion") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // listVersion is required
        cJSON *ver = cJSON_GetObjectItem(payload, "listVersion");
        if (!ver || !cJSON_IsNumber(ver)) return false;
    }

    clear();
    m_root = obj;
    m_type = msgType;

    return true;
}

// ===== Serialize =====

char* GetLocalListVersion::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void GetLocalListVersion::print() const
{
    char *data = toJson();
    if (data) {
        printf("[GetLocalListVersion] %s\n", data);
        free(data);
    }
}
