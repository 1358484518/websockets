#include "BootNotification.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Status Conversion =====

const char* BootNotification::statusToString(quint8 s) const
{
    switch (s) {
        case BOOT_ACCEPTED: return "Accepted";
        case BOOT_PENDING:  return "Pending";
        case BOOT_REJECTED: return "Rejected";
        default: return "Rejected";
    }
}

quint8 BootNotification::stringToStatus(const char *str) const
{
    if (!str) return BOOT_REJECTED;
    if (strcmp(str, "Accepted") == 0) return BOOT_ACCEPTED;
    if (strcmp(str, "Pending") == 0)  return BOOT_PENDING;
    if (strcmp(str, "Rejected") == 0) return BOOT_REJECTED;
    return BOOT_REJECTED;
}

// ===== Construction / Destruction =====

BootNotification::BootNotification()
    : m_root(nullptr), m_type(0), m_status(BOOT_REJECTED), m_interval(0) {}

BootNotification::BootNotification(const char *value)
    : m_root(nullptr), m_type(0), m_status(BOOT_REJECTED), m_interval(0)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

BootNotification::BootNotification(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(BOOT_REJECTED), m_interval(0)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

BootNotification::~BootNotification()
{
    clear();
}

BootNotification::BootNotification(BootNotification&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status), m_interval(other.m_interval)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = BOOT_REJECTED;
    other.m_interval = 0;
}

BootNotification& BootNotification::operator=(BootNotification&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        m_interval = other.m_interval;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = BOOT_REJECTED;
        other.m_interval = 0;
    }
    return *this;
}

void BootNotification::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = BOOT_REJECTED;
    m_interval = 0;
}

// ===== Internal Helpers =====

bool BootNotification::ensurePayload()
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

bool BootNotification::setStringField(const char *key, const char *value)
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

bool BootNotification::setNumberField(const char *key, double value)
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

const char* BootNotification::getStringField(const char *key) const
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

bool BootNotification::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("BootNotification");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }

    cJSON *vendorItem = cJSON_CreateString("");
    if (!vendorItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "chargePointVendor", vendorItem);

    cJSON *modelItem = cJSON_CreateString("");
    if (!modelItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "chargePointModel", modelItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool BootNotification::buildConf()
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

    cJSON *timeItem = cJSON_CreateString("");
    if (!timeItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "currentTime", timeItem);

    cJSON *intervalItem = cJSON_CreateNumber(0);
    if (!intervalItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "interval", intervalItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALLRESULT;
    m_status = BOOT_ACCEPTED;
    m_interval = 0;
    return true;
}

// ===== Common Setters =====

bool BootNotification::setMsgSeq(quint64 i)
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

bool BootNotification::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("BootNotification");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool BootNotification::setVendor(const char *vendor)
{
    if (m_type != OCPP_CALL) return false;
    return setStringField("chargePointVendor", vendor ? vendor : "");
}

bool BootNotification::setModel(const char *model)
{
    if (m_type != OCPP_CALL) return false;
    return setStringField("chargePointModel", model ? model : "");
}

bool BootNotification::setSerialNumber(const char *sn)
{
    if (m_type != OCPP_CALL) return false;
    if (sn && *sn) {
        return setStringField("chargePointSerialNumber", sn);
    } else {
        if (!ensurePayload()) return false;
        int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
        cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
        if (payload) cJSON_DeleteItemFromObject(payload, "chargePointSerialNumber");
        return true;
    }
}

bool BootNotification::setBoxSerialNumber(const char *sn)
{
    if (m_type != OCPP_CALL) return false;
    if (sn && *sn) {
        return setStringField("chargeBoxSerialNumber", sn);
    } else {
        if (!ensurePayload()) return false;
        int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
        cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
        if (payload) cJSON_DeleteItemFromObject(payload, "chargeBoxSerialNumber");
        return true;
    }
}

bool BootNotification::setFirmwareVersion(const char *version)
{
    if (m_type != OCPP_CALL) return false;
    if (version && *version) {
        return setStringField("firmwareVersion", version);
    } else {
        if (!ensurePayload()) return false;
        int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
        cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
        if (payload) cJSON_DeleteItemFromObject(payload, "firmwareVersion");
        return true;
    }
}

bool BootNotification::setIccid(const char *iccid)
{
    if (m_type != OCPP_CALL) return false;
    if (iccid && *iccid) {
        return setStringField("iccid", iccid);
    } else {
        if (!ensurePayload()) return false;
        int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
        cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
        if (payload) cJSON_DeleteItemFromObject(payload, "iccid");
        return true;
    }
}

bool BootNotification::setImsi(const char *imsi)
{
    if (m_type != OCPP_CALL) return false;
    if (imsi && *imsi) {
        return setStringField("imsi", imsi);
    } else {
        if (!ensurePayload()) return false;
        int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
        cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
        if (payload) cJSON_DeleteItemFromObject(payload, "imsi");
        return true;
    }
}

bool BootNotification::setMeterType(const char *type)
{
    if (m_type != OCPP_CALL) return false;
    if (type && *type) {
        return setStringField("meterType", type);
    } else {
        if (!ensurePayload()) return false;
        int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
        cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
        if (payload) cJSON_DeleteItemFromObject(payload, "meterType");
        return true;
    }
}

bool BootNotification::setMeterSerialNumber(const char *sn)
{
    if (m_type != OCPP_CALL) return false;
    if (sn && *sn) {
        return setStringField("meterSerialNumber", sn);
    } else {
        if (!ensurePayload()) return false;
        int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
        cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
        if (payload) cJSON_DeleteItemFromObject(payload, "meterSerialNumber");
        return true;
    }
}

// ===== Response Setters =====

bool BootNotification::setStatus(quint8 s)
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

bool BootNotification::setCurrentTime(const char *time)
{
    if (m_type != OCPP_CALLRESULT) return false;
    return setStringField("currentTime", time ? time : "");
}

bool BootNotification::setInterval(qint32 interval)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!setNumberField("interval", (double)interval)) return false;
    m_interval = interval;
    return true;
}

// ===== Getters =====

quint64 BootNotification::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 BootNotification::type() const
{
    return m_type;
}

const char* BootNotification::vendor() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("chargePointVendor");
}

const char* BootNotification::model() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("chargePointModel");
}

const char* BootNotification::serialNumber() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("chargePointSerialNumber");
}

const char* BootNotification::boxSerialNumber() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("chargeBoxSerialNumber");
}

const char* BootNotification::firmwareVersion() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("firmwareVersion");
}

const char* BootNotification::iccid() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("iccid");
}

const char* BootNotification::imsi() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("imsi");
}

const char* BootNotification::meterType() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("meterType");
}

const char* BootNotification::meterSerialNumber() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("meterSerialNumber");
}

quint8 BootNotification::status() const
{
    return m_status;
}

const char* BootNotification::statusString() const
{
    return statusToString(m_status);
}

const char* BootNotification::currentTime() const
{
    if (m_type != OCPP_CALLRESULT) return "";
    return getStringField("currentTime");
}

qint32 BootNotification::interval() const
{
    if (m_type != OCPP_CALLRESULT) return 0;
    return m_interval;
}

bool BootNotification::isCall() const
{
    return m_type == OCPP_CALL;
}

bool BootNotification::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool BootNotification::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool BootNotification::parse(const char *value)
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

bool BootNotification::parse(cJSON *obj)
{
    if (!obj || !cJSON_IsArray(obj)) return false;

    int size = cJSON_GetArraySize(obj);
    if (size < 3) return false;

    cJSON *typeItem = cJSON_GetArrayItem(obj, 0);
    if (!typeItem || !cJSON_IsNumber(typeItem)) return false;

    quint8 type = (quint8)typeItem->valueint;
    if (type != OCPP_CALL && type != OCPP_CALLRESULT) return false;

    cJSON *idItem = cJSON_GetArrayItem(obj, 1);
    if (!idItem || !cJSON_IsString(idItem)) return false;

    quint8 status = BOOT_REJECTED;
    qint32 interval = 0;

    if (type == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "BootNotification") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // Required fields
        cJSON *vendor = cJSON_GetObjectItem(payload, "chargePointVendor");
        if (!vendor || !cJSON_IsString(vendor)) return false;

        cJSON *model = cJSON_GetObjectItem(payload, "chargePointModel");
        if (!model || !cJSON_IsString(model)) return false;
    }

    if (type == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;

        cJSON *statusItem = cJSON_GetObjectItem(payload, "status");
        if (statusItem && cJSON_IsString(statusItem)) {
            status = stringToStatus(statusItem->valuestring);
        }

        cJSON *intervalItem = cJSON_GetObjectItem(payload, "interval");
        if (intervalItem && cJSON_IsNumber(intervalItem)) {
            interval = (qint32)intervalItem->valueint;
        }
    }

    clear();
    m_root = obj;
    m_type = type;
    m_status = status;
    m_interval = interval;

    return true;
}

// ===== Serialize =====

char* BootNotification::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void BootNotification::print() const
{
    char *data = toJson();
    if (data) {
        printf("[BootNotification] %s\n", data);
        free(data);
    }
}


#if 0
// 构建请求
BootNotification req;
req.buildReq();
req.setMsgSeq(1);
req.setVendor("Tesla");
req.setModel("WallConnector");
req.setSerialNumber("SN-2024-0001");
req.setFirmwareVersion("v2.1.0");

char *json = req.toJson();
// 发送...
free(json);

// 解析响应
BootNotification conf;
if (conf.parse(responseJson)) {
    if (conf.status() == BOOT_ACCEPTED) {
        printf("Heartbeat interval: %d\n", conf.interval());
    }
}
#endif


