#include "ChangeConfiguration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Status Conversion =====

const char* ChangeConfiguration::statusToString(quint8 s) const
{
    switch (s) {
        case CONFIG_ACCEPTED:       return "Accepted";
        case CONFIG_REJECTED:       return "Rejected";
        case CONFIG_REBOOT_REQUIRED: return "RebootRequired";
        case CONFIG_NOT_SUPPORTED:  return "NotSupported";
        default: return "Rejected";
    }
}

quint8 ChangeConfiguration::stringToStatus(const char *str) const
{
    if (!str) return CONFIG_REJECTED;
    if (strcmp(str, "Accepted") == 0)       return CONFIG_ACCEPTED;
    if (strcmp(str, "Rejected") == 0)       return CONFIG_REJECTED;
    if (strcmp(str, "RebootRequired") == 0) return CONFIG_REBOOT_REQUIRED;
    if (strcmp(str, "NotSupported") == 0)   return CONFIG_NOT_SUPPORTED;
    return CONFIG_REJECTED;
}

// ===== Construction / Destruction =====

ChangeConfiguration::ChangeConfiguration()
    : m_root(nullptr), m_type(0), m_status(CONFIG_REJECTED) {}

ChangeConfiguration::ChangeConfiguration(const char *value)
    : m_root(nullptr), m_type(0), m_status(CONFIG_REJECTED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

ChangeConfiguration::ChangeConfiguration(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(CONFIG_REJECTED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

ChangeConfiguration::~ChangeConfiguration()
{
    clear();
}

ChangeConfiguration::ChangeConfiguration(ChangeConfiguration&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = CONFIG_REJECTED;
}

ChangeConfiguration& ChangeConfiguration::operator=(ChangeConfiguration&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = CONFIG_REJECTED;
    }
    return *this;
}

void ChangeConfiguration::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = CONFIG_REJECTED;
}

// ===== Internal Helpers =====

bool ChangeConfiguration::ensurePayload()
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

bool ChangeConfiguration::setStringField(const char *key, const char *value)
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

const char* ChangeConfiguration::getStringField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return "";
    if (!key) return "";

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return "";

    cJSON *item = cJSON_GetObjectItem(payload, key);
    return item && cJSON_IsString(item) ? item->valuestring : "";
}

// ===== Build =====

bool ChangeConfiguration::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("ChangeConfiguration");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }

    cJSON *keyItem = cJSON_CreateString("");
    if (!keyItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "key", keyItem);

    cJSON *valueItem = cJSON_CreateString("");
    if (!valueItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "value", valueItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool ChangeConfiguration::buildConf()
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
    m_status = CONFIG_ACCEPTED;
    return true;
}

// ===== Common Setters =====

bool ChangeConfiguration::setMsgSeq(quint64 i)
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

bool ChangeConfiguration::setMsgSeq(QString i)
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

bool ChangeConfiguration::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("ChangeConfiguration");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool ChangeConfiguration::setKey(const char *key)
{
    if (m_type != OCPP_CALL) return false;
    if (!key) return false;
    return setStringField("key", key);
}

bool ChangeConfiguration::setValue(const char *value)
{
    if (m_type != OCPP_CALL) return false;
    if (!value) return false;
    return setStringField("value", value);
}

// ===== Response Setters =====

bool ChangeConfiguration::setStatus(quint8 s)
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

quint64 ChangeConfiguration::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 ChangeConfiguration::type() const
{
    return m_type;
}

const char* ChangeConfiguration::key() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("key");
}

const char* ChangeConfiguration::value() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("value");
}

quint8 ChangeConfiguration::status() const
{
    return m_status;
}

const char* ChangeConfiguration::statusString() const
{
    return statusToString(m_status);
}

bool ChangeConfiguration::isCall() const
{
    return m_type == OCPP_CALL;
}

bool ChangeConfiguration::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool ChangeConfiguration::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool ChangeConfiguration::parse(const char *value)
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

bool ChangeConfiguration::parse(cJSON *obj)
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

    quint8 status = CONFIG_REJECTED;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "ChangeConfiguration") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // Required fields
        cJSON *keyItem = cJSON_GetObjectItem(payload, "key");
        if (!keyItem || !cJSON_IsString(keyItem)) return false;

        cJSON *valueItem = cJSON_GetObjectItem(payload, "value");
        if (!valueItem || !cJSON_IsString(valueItem)) return false;
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

char* ChangeConfiguration::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void ChangeConfiguration::print() const
{
    char *data = toJson();
    if (data) {
        printf("[ChangeConfiguration] %s\n", data);
        free(data);
    }
}
#if 0

// 解析收到的更改配置请求
ChangeConfiguration req;
if (req.parse(receivedJson)) {
    const char *key = req.key();
    const char *value = req.value();

    printf("Change config: %s = %s\n", key, value);

    // 构建响应
    ChangeConfiguration conf;
    conf.buildConf();
    conf.setMsgSeq(req.msgSeq());

    if (configSupported) {
        if (changeSuccess) {
            conf.setStatus(CONFIG_ACCEPTED);
        } else if (needReboot) {
            conf.setStatus(CONFIG_REBOOT_REQUIRED);
        } else {
            conf.setStatus(CONFIG_REJECTED);
        }
    } else {
        conf.setStatus(CONFIG_NOT_SUPPORTED);
    }

    char *response = conf.toJson();
    // 发送响应...
    free(response);
}
#endif

#if 0
OCPP 1.6 标准配置项大概有这些，ChangeConfiguration 都可以改：
类别      示例      key	说明
心跳	HeartbeatInterval	心跳间隔，秒
计量	MeterValueSampleInterval	电表采样间隔，秒
计量	MeterValuesSampledData	采样数据项，逗号分隔
连接器	NumberOfConnectors	连接器数量
时钟	ClockAlignedDataInterval	时钟对齐数据间隔
交易	StopTransactionOnEVSideDisconnect	拔枪是否停止交易
交易	StopTransactionOnInvalidId	无效 ID 是否停止交易
授权	LocalAuthorizeOffline	离线本地授权
固件	FirmwareUpdateStatusInterval	固件更新状态上报间隔
诊断	DiagnosticsStatusInterval	诊断状态上报间隔
#endif
