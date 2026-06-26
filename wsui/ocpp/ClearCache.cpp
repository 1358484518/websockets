#include "ClearCache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Status Conversion =====

const char* ClearCache::statusToString(quint8 s) const
{
    switch (s) {
        case CLEAR_ACCEPTED: return "Accepted";
        case CLEAR_REJECTED: return "Rejected";
        default: return "Rejected";
    }
}

quint8 ClearCache::stringToStatus(const char *str) const
{
    if (!str) return CLEAR_REJECTED;
    if (strcmp(str, "Accepted") == 0) return CLEAR_ACCEPTED;
    if (strcmp(str, "Rejected") == 0) return CLEAR_REJECTED;
    return CLEAR_REJECTED;
}

// ===== Construction / Destruction =====

ClearCache::ClearCache()
    : m_root(nullptr), m_type(0), m_status(CLEAR_REJECTED) {}

ClearCache::ClearCache(const char *value)
    : m_root(nullptr), m_type(0), m_status(CLEAR_REJECTED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

ClearCache::ClearCache(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(CLEAR_REJECTED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

ClearCache::~ClearCache()
{
    clear();
}

ClearCache::ClearCache(ClearCache&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = CLEAR_REJECTED;
}

ClearCache& ClearCache::operator=(ClearCache&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = CLEAR_REJECTED;
    }
    return *this;
}

void ClearCache::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = CLEAR_REJECTED;
}

// ===== Internal Helpers =====

bool ClearCache::ensurePayload()
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

// ===== Build =====

bool ClearCache::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("ClearCache");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool ClearCache::buildConf()
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
    m_status = CLEAR_ACCEPTED;
    return true;
}

// ===== Common Setters =====

bool ClearCache::setMsgSeq(quint64 i)
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

bool ClearCache::setMsgSeq(QString i)
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

bool ClearCache::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("ClearCache");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Response Setters =====

bool ClearCache::setStatus(quint8 s)
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

quint64 ClearCache::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 ClearCache::type() const
{
    return m_type;
}

quint8 ClearCache::status() const
{
    return m_status;
}

const char* ClearCache::statusString() const
{
    return statusToString(m_status);
}

bool ClearCache::isCall() const
{
    return m_type == OCPP_CALL;
}

bool ClearCache::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool ClearCache::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool ClearCache::parse(const char *value)
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

bool ClearCache::parse(cJSON *obj)
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

    quint8 status = CLEAR_REJECTED;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "ClearCache") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;
        // ClearCache request has no required fields
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
    m_status = status;

    return true;
}

// ===== Serialize =====

char* ClearCache::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void ClearCache::print() const
{
    char *data = toJson();
    if (data) {
        printf("[ClearCache] %s\n", data);
        free(data);
    }
}

#if 0
// 解析收到的清除缓存请求
ClearCache req;
if (req.parse(receivedJson)) {
    // 执行清除缓存操作
    bool success = clearAuthorizationCache();

    // 构建响应
    ClearCache conf;
    conf.buildConf();
    conf.setMsgSeq(req.msgSeq());

    if (success) {
        conf.setStatus(CLEAR_ACCEPTED);
    } else {
        conf.setStatus(CLEAR_REJECTED);
    }

    char *response = conf.toJson();
    // 发送响应...
    free(response);
}
#endif
