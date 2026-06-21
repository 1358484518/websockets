#include "datatransfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Status Conversion =====

const char* DataTransfer::statusToString(quint8 s) const
{
    switch (s) {
        case DATA_TRANSFER_ACCEPTED:            return "Accepted";
        case DATA_TRANSFER_REJECTED:            return "Rejected";
        case DATA_TRANSFER_UNKNOWN_MESSAGE_ID:  return "UnknownMessageId";
        case DATA_TRANSFER_UNKNOWN_VENDOR_ID:   return "UnknownVendorId";
        default: return "Rejected";
    }
}

quint8 DataTransfer::stringToStatus(const char *str) const
{
    if (!str) return DATA_TRANSFER_REJECTED;
    if (strcmp(str, "Accepted") == 0)            return DATA_TRANSFER_ACCEPTED;
    if (strcmp(str, "Rejected") == 0)            return DATA_TRANSFER_REJECTED;
    if (strcmp(str, "UnknownMessageId") == 0)    return DATA_TRANSFER_UNKNOWN_MESSAGE_ID;
    if (strcmp(str, "UnknownVendorId") == 0)     return DATA_TRANSFER_UNKNOWN_VENDOR_ID;
    return DATA_TRANSFER_REJECTED;
}

// ===== Construction / Destruction =====

DataTransfer::DataTransfer()
    : m_root(nullptr), m_type(0), m_status(DATA_TRANSFER_REJECTED) {}

DataTransfer::DataTransfer(const char *value)
    : m_root(nullptr), m_type(0), m_status(DATA_TRANSFER_REJECTED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

DataTransfer::DataTransfer(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(DATA_TRANSFER_REJECTED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

DataTransfer::~DataTransfer() { clear(); }

DataTransfer::DataTransfer(DataTransfer&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = DATA_TRANSFER_REJECTED;
}

DataTransfer& DataTransfer::operator=(DataTransfer&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = DATA_TRANSFER_REJECTED;
    }
    return *this;
}

void DataTransfer::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = DATA_TRANSFER_REJECTED;
}

// ===== Internal Helpers =====

bool DataTransfer::ensurePayload()
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

bool DataTransfer::setStringField(const char *key, const char *value)
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

const char* DataTransfer::getStringField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return "";
    if (!key) return "";

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return "";

    cJSON *item = cJSON_GetObjectItem(payload, key);
    return item && cJSON_IsString(item) ? item->valuestring : "";
}

bool DataTransfer::hasField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    if (!key) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return false;

    return cJSON_GetObjectItem(payload, key) != nullptr;
}

// ===== Build =====

bool DataTransfer::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("DataTransfer");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }

    // vendorId is required
    cJSON *vendorItem = cJSON_CreateString("");
    if (!vendorItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "vendorId", vendorItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool DataTransfer::buildConf()
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
    m_status = DATA_TRANSFER_ACCEPTED;
    return true;
}

// ===== Common Setters =====

bool DataTransfer::setMsgSeq(quint64 i)
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

bool DataTransfer::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("DataTransfer");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool DataTransfer::setVendorId(const char *vendorId)
{
    if (m_type != OCPP_CALL) return false;
    if (!vendorId) return false;
    return setStringField("vendorId", vendorId);
}

bool DataTransfer::setMessageId(const char *msgId)
{
    if (m_type != OCPP_CALL) return false;
    if (!msgId) return false;
    return setStringField("messageId", msgId);
}

bool DataTransfer::setData(const char *data)
{
    if (m_type != OCPP_CALL) return false;
    if (!data) return false;
    return setStringField("data", data);
}

// ===== Response Setters =====

bool DataTransfer::setStatus(quint8 s)
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

bool DataTransfer::setResponseData(const char *data)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!data) return false;
    return setStringField("data", data);
}

// ===== Getters =====

quint64 DataTransfer::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 DataTransfer::type() const { return m_type; }

const char* DataTransfer::vendorId() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("vendorId");
}

const char* DataTransfer::messageId() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("messageId");
}

const char* DataTransfer::data() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("data");
}

quint8 DataTransfer::status() const { return m_status; }
const char* DataTransfer::statusString() const { return statusToString(m_status); }

const char* DataTransfer::responseData() const
{
    if (m_type != OCPP_CALLRESULT) return "";
    return getStringField("data");
}

bool DataTransfer::isCall() const { return m_type == OCPP_CALL; }
bool DataTransfer::isCallResult() const { return m_type == OCPP_CALLRESULT; }
bool DataTransfer::isValid() const { return m_root != nullptr; }

// ===== Parse =====

bool DataTransfer::parse(const char *value)
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

bool DataTransfer::parse(cJSON *obj)
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

    quint8 status = DATA_TRANSFER_REJECTED;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "DataTransfer") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // vendorId is required
        cJSON *vendorJson = cJSON_GetObjectItem(payload, "vendorId");
        if (!vendorJson || !cJSON_IsString(vendorJson)) return false;
        // messageId and data are optional
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

char* DataTransfer::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void DataTransfer::print() const
{
    char *data = toJson();
    if (data) {
        printf("[DataTransfer] %s\n", data);
        free(data);
    }
}
#if 0
// 充电桩主动发送厂商自定义数据
DataTransfer req;
req.buildReq();
req.setMsgSeq(1001);
req.setVendorId("MyCompany");
req.setMessageId("GetDiagnostics");
req.setData("{\"type\":\"full\",\"level\":\"debug\"}");

char *json = req.toJson();
// 发送请求...
free(json);

// 解析收到的响应
DataTransfer conf(responseJson);
if (conf.isValid() && conf.status() == DATA_TRANSFER_ACCEPTED) {
    const char *result = conf.responseData();
    // 处理响应数据...
}
#endif

