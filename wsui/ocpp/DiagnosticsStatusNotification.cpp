#include "DiagnosticsStatusNotification.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Status Conversion =====

const char* DiagnosticsStatusNotification::statusToString(quint8 s) const
{
    switch (s) {
        case DIAG_IDLE:           return "Idle";
        case DIAG_UPLOADED:       return "Uploaded";
        case DIAG_UPLOAD_FAILED:  return "UploadFailed";
        case DIAG_UPLOADING:      return "Uploading";
        default: return "Idle";
    }
}

quint8 DiagnosticsStatusNotification::stringToStatus(const char *str) const
{
    if (!str) return DIAG_IDLE;
    if (strcmp(str, "Idle") == 0)           return DIAG_IDLE;
    if (strcmp(str, "Uploaded") == 0)       return DIAG_UPLOADED;
    if (strcmp(str, "UploadFailed") == 0)   return DIAG_UPLOAD_FAILED;
    if (strcmp(str, "Uploading") == 0)      return DIAG_UPLOADING;
    return DIAG_IDLE;
}

// ===== Construction / Destruction =====

DiagnosticsStatusNotification::DiagnosticsStatusNotification()
    : m_root(nullptr), m_type(0), m_status(DIAG_IDLE) {}

DiagnosticsStatusNotification::DiagnosticsStatusNotification(const char *value)
    : m_root(nullptr), m_type(0), m_status(DIAG_IDLE)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

DiagnosticsStatusNotification::DiagnosticsStatusNotification(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(DIAG_IDLE)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

DiagnosticsStatusNotification::~DiagnosticsStatusNotification() { clear(); }

DiagnosticsStatusNotification::DiagnosticsStatusNotification(DiagnosticsStatusNotification&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = DIAG_IDLE;
}

DiagnosticsStatusNotification& DiagnosticsStatusNotification::operator=(DiagnosticsStatusNotification&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = DIAG_IDLE;
    }
    return *this;
}

void DiagnosticsStatusNotification::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = DIAG_IDLE;
}

// ===== Internal Helpers =====

bool DiagnosticsStatusNotification::ensurePayload()
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

bool DiagnosticsStatusNotification::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("DiagnosticsStatusNotification");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }

    cJSON *statusItem = cJSON_CreateString("Idle");
    if (!statusItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "status", statusItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    m_status = DIAG_IDLE;
    return true;
}

bool DiagnosticsStatusNotification::buildConf()
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

// ===== Common Setters =====

bool DiagnosticsStatusNotification::setMsgSeq(quint64 i)
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

bool DiagnosticsStatusNotification::setMsgSeq(QString i)
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

bool DiagnosticsStatusNotification::setType(quint8 type)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    if (type != OCPP_CALL && type != OCPP_CALLRESULT) return false;

    if (m_type == type) return true;

    cJSON *typeItem = cJSON_GetArrayItem(m_root, 0);
    if (!typeItem || !cJSON_IsNumber(typeItem)) return false;

    // Adjust array structure when switching between CALL and CALLRESULT
    if (m_type == OCPP_CALL && type == OCPP_CALLRESULT) {
        cJSON *actionItem = cJSON_GetArrayItem(m_root, 2);
        if (actionItem) {
            cJSON_DetachItemFromArray(m_root, 2);
            cJSON_Delete(actionItem);
        }
    } else if (m_type == OCPP_CALLRESULT && type == OCPP_CALL) {
        cJSON *actionItem = cJSON_CreateString("DiagnosticsStatusNotification");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool DiagnosticsStatusNotification::setStatus(quint8 s)
{
    if (m_type != OCPP_CALL) return false;
    if (!ensurePayload()) return false;

    int payloadIndex = 3;
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

quint64 DiagnosticsStatusNotification::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 DiagnosticsStatusNotification::type() const { return m_type; }

quint8 DiagnosticsStatusNotification::status() const { return m_status; }
const char* DiagnosticsStatusNotification::statusString() const { return statusToString(m_status); }

bool DiagnosticsStatusNotification::isCall() const { return m_type == OCPP_CALL; }
bool DiagnosticsStatusNotification::isCallResult() const { return m_type == OCPP_CALLRESULT; }
bool DiagnosticsStatusNotification::isValid() const { return m_root != nullptr; }

// ===== Parse =====

bool DiagnosticsStatusNotification::parse(const char *value)
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

bool DiagnosticsStatusNotification::parse(cJSON *obj)
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

    quint8 status = DIAG_IDLE;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "DiagnosticsStatusNotification") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        cJSON *statusJson = cJSON_GetObjectItem(payload, "status");
        if (statusJson && cJSON_IsString(statusJson)) {
            status = stringToStatus(statusJson->valuestring);
        }
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;
        // Response payload is empty object
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_status = status;

    return true;
}

// ===== Serialize =====

char* DiagnosticsStatusNotification::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void DiagnosticsStatusNotification::print() const
{
    char *data = toJson();
    if (data) {
        printf("[DiagnosticsStatusNotification] %s\n", data);
        free(data);
    }
}

#if 0
// 充电桩开始上传诊断文件时通知
DiagnosticsStatusNotification req;
req.buildReq();
req.setMsgSeq(2001);
req.setStatus(DIAG_UPLOADING);

char *json = req.toJson();
// 发送请求...
free(json);

// 上传完成后再次通知
DiagnosticsStatusNotification req2;
req2.buildReq();
req2.setMsgSeq(2002);
req2.setStatus(DIAG_UPLOADED);
// 发送...

// 解析收到的响应（空对象）
DiagnosticsStatusNotification conf(responseJson);
if (conf.isValid() && conf.isCallResult()) {
    // 响应确认，无需处理额外数据
}
#endif
