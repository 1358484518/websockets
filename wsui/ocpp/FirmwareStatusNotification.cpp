#include "FirmwareStatusNotification.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Status Conversion =====

const char* FirmwareStatusNotification::statusToString(quint8 s) const
{
    switch (s) {
        case FW_DOWNLOADED:           return "Downloaded";
        case FW_DOWNLOAD_FAILED:      return "DownloadFailed";
        case FW_INSTALLATION_FAILED:  return "InstallationFailed";
        case FW_INSTALLING:           return "Installing";
        case FW_INSTALLED:            return "Installed";
        default: return "Downloaded";
    }
}

quint8 FirmwareStatusNotification::stringToStatus(const char *str) const
{
    if (!str) return FW_DOWNLOADED;
    if (strcmp(str, "Downloaded") == 0)           return FW_DOWNLOADED;
    if (strcmp(str, "DownloadFailed") == 0)       return FW_DOWNLOAD_FAILED;
    if (strcmp(str, "InstallationFailed") == 0)   return FW_INSTALLATION_FAILED;
    if (strcmp(str, "Installing") == 0)           return FW_INSTALLING;
    if (strcmp(str, "Installed") == 0)            return FW_INSTALLED;
    return FW_DOWNLOADED;
}

// ===== Construction / Destruction =====

FirmwareStatusNotification::FirmwareStatusNotification()
    : m_root(nullptr), m_type(0), m_status(FW_DOWNLOADED) {}

FirmwareStatusNotification::FirmwareStatusNotification(const char *value)
    : m_root(nullptr), m_type(0), m_status(FW_DOWNLOADED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

FirmwareStatusNotification::FirmwareStatusNotification(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(FW_DOWNLOADED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

FirmwareStatusNotification::~FirmwareStatusNotification() { clear(); }

FirmwareStatusNotification::FirmwareStatusNotification(FirmwareStatusNotification&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = FW_DOWNLOADED;
}

FirmwareStatusNotification& FirmwareStatusNotification::operator=(FirmwareStatusNotification&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = FW_DOWNLOADED;
    }
    return *this;
}

void FirmwareStatusNotification::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = FW_DOWNLOADED;
}

// ===== Internal Helpers =====

bool FirmwareStatusNotification::ensurePayload()
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

bool FirmwareStatusNotification::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("FirmwareStatusNotification");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }

    cJSON *statusItem = cJSON_CreateString("Downloaded");
    if (!statusItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "status", statusItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    m_status = FW_DOWNLOADED;
    return true;
}

bool FirmwareStatusNotification::buildConf()
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

bool FirmwareStatusNotification::setMsgSeq(quint64 i)
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

bool FirmwareStatusNotification::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("FirmwareStatusNotification");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool FirmwareStatusNotification::setStatus(quint8 s)
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

quint64 FirmwareStatusNotification::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 FirmwareStatusNotification::type() const { return m_type; }

quint8 FirmwareStatusNotification::status() const { return m_status; }
const char* FirmwareStatusNotification::statusString() const { return statusToString(m_status); }

bool FirmwareStatusNotification::isCall() const { return m_type == OCPP_CALL; }
bool FirmwareStatusNotification::isCallResult() const { return m_type == OCPP_CALLRESULT; }
bool FirmwareStatusNotification::isValid() const { return m_root != nullptr; }

// ===== Parse =====

bool FirmwareStatusNotification::parse(const char *value)
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

bool FirmwareStatusNotification::parse(cJSON *obj)
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

    quint8 status = FW_DOWNLOADED;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "FirmwareStatusNotification") != 0) return false;

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

char* FirmwareStatusNotification::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void FirmwareStatusNotification::print() const
{
    char *data = toJson();
    if (data) {
        printf("[FirmwareStatusNotification] %s\n", data);
        free(data);
    }
}

#if 0
// 充电桩固件下载完成时通知
FirmwareStatusNotification req;
req.buildReq();
req.setMsgSeq(3001);
req.setStatus(FW_DOWNLOADED);

char *json = req.toJson();
// 发送请求...
free(json);

// 开始安装时再次通知
FirmwareStatusNotification req2;
req2.buildReq();
req2.setMsgSeq(3002);
req2.setStatus(FW_INSTALLING);
// 发送...

// 安装完成后通知
FirmwareStatusNotification req3;
req3.buildReq();
req3.setMsgSeq(3003);
req3.setStatus(FW_INSTALLED);
// 发送...

// 解析收到的响应（空对象）
FirmwareStatusNotification conf(responseJson);
if (conf.isValid() && conf.isCallResult()) {
    // 响应确认，无需处理额外数据
}
#endif
