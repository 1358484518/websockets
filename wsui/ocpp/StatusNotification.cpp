#include "statusnotification.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

StatusNotification::StatusNotification()
    : m_root(nullptr), m_type(0), m_status(STATUS_AVAILABLE), m_errorCode(ERROR_NO_ERROR)
{
}

StatusNotification::StatusNotification(const char *value)
    : m_root(nullptr), m_type(0), m_status(STATUS_AVAILABLE), m_errorCode(ERROR_NO_ERROR)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

StatusNotification::StatusNotification(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(STATUS_AVAILABLE), m_errorCode(ERROR_NO_ERROR)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

StatusNotification::~StatusNotification()
{
    clear();
}

StatusNotification::StatusNotification(StatusNotification&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status), m_errorCode(other.m_errorCode)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = STATUS_AVAILABLE;
    other.m_errorCode = ERROR_NO_ERROR;
}

StatusNotification& StatusNotification::operator=(StatusNotification&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        m_errorCode = other.m_errorCode;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = STATUS_AVAILABLE;
        other.m_errorCode = ERROR_NO_ERROR;
    }
    return *this;
}

void StatusNotification::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = STATUS_AVAILABLE;
    m_errorCode = ERROR_NO_ERROR;
}

bool StatusNotification::ensurePayload()
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

bool StatusNotification::setStringField(const char *key, const char *value)
{
    if (!key || !value || *value == '\0')
        return false;
    if (!ensurePayload())
        return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(value);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(payload, key, newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, key);
        cJSON *newItem = cJSON_CreateString(value);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, key, newItem);
    }

    return true;
}

bool StatusNotification::clearField(const char *key)
{
    if (!key)
        return false;
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (!item)
        return true;

    cJSON_DeleteItemFromObject(payload, key);
    return true;
}

const char* StatusNotification::getStringField(const char *key) const
{
    if (!key)
        return "";
    if (!m_root || !cJSON_IsArray(m_root))
        return "";

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload))
        return "";

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (!item || !cJSON_IsString(item))
        return "";

    return item->valuestring;
}

bool StatusNotification::hasField(const char *key) const
{
    if (!key)
        return false;
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    return cJSON_GetObjectItem(payload, key) != nullptr;
}

const char* StatusNotification::statusToString(quint8 status)
{
    switch (status) {
        case STATUS_AVAILABLE:      return "Available";
        case STATUS_PREPARING:      return "Preparing";
        case STATUS_CHARGING:       return "Charging";
        case STATUS_SUSPENDED_EVSE: return "SuspendedEVSE";
        case STATUS_SUSPENDED_EV:   return "SuspendedEV";
        case STATUS_FINISHING:      return "Finishing";
        case STATUS_RESERVED:       return "Reserved";
        case STATUS_UNAVAILABLE:    return "Unavailable";
        case STATUS_FAULTED:        return "Faulted";
        default:                    return "Available";
    }
}

quint8 StatusNotification::stringToStatus(const char *str)
{
    if (!str)
        return STATUS_AVAILABLE;
    if (strcmp(str, "Available") == 0)
        return STATUS_AVAILABLE;
    if (strcmp(str, "Preparing") == 0)
        return STATUS_PREPARING;
    if (strcmp(str, "Charging") == 0)
        return STATUS_CHARGING;
    if (strcmp(str, "SuspendedEVSE") == 0)
        return STATUS_SUSPENDED_EVSE;
    if (strcmp(str, "SuspendedEV") == 0)
        return STATUS_SUSPENDED_EV;
    if (strcmp(str, "Finishing") == 0)
        return STATUS_FINISHING;
    if (strcmp(str, "Reserved") == 0)
        return STATUS_RESERVED;
    if (strcmp(str, "Unavailable") == 0)
        return STATUS_UNAVAILABLE;
    if (strcmp(str, "Faulted") == 0)
        return STATUS_FAULTED;
    return STATUS_AVAILABLE;
}

const char* StatusNotification::errorCodeToString(quint8 errorCode)
{
    switch (errorCode) {
        case ERROR_NO_ERROR:               return "NoError";
        case ERROR_CONNECTOR_LOCK_FAILURE: return "ConnectorLockFailure";
        case ERROR_EV_COMMUNICATION_ERROR: return "EVCommunicationError";
        case ERROR_GROUND_FAILURE:         return "GroundFailure";
        case ERROR_HIGH_TEMPERATURE:       return "HighTemperature";
        case ERROR_INTERNAL_ERROR:         return "InternalError";
        case ERROR_LOCAL_LIST_CONFLICT:    return "LocalListConflict";
        case ERROR_OVER_CURRENT_FAILURE:   return "OverCurrentFailure";
        case ERROR_POWER_METER_FAILURE:    return "PowerMeterFailure";
        case ERROR_POWER_SWITCH_FAILURE:   return "PowerSwitchFailure";
        case ERROR_READER_FAILURE:         return "ReaderFailure";
        case ERROR_RESET_FAILURE:          return "ResetFailure";
        case ERROR_UNDER_VOLTAGE:          return "UnderVoltage";
        case ERROR_OVER_VOLTAGE:           return "OverVoltage";
        case ERROR_WEAK_SIGNAL:            return "WeakSignal";
        default:                           return "NoError";
    }
}

quint8 StatusNotification::stringToErrorCode(const char *str)
{
    if (!str)
        return ERROR_NO_ERROR;
    if (strcmp(str, "NoError") == 0)
        return ERROR_NO_ERROR;
    if (strcmp(str, "ConnectorLockFailure") == 0)
        return ERROR_CONNECTOR_LOCK_FAILURE;
    if (strcmp(str, "EVCommunicationError") == 0)
        return ERROR_EV_COMMUNICATION_ERROR;
    if (strcmp(str, "GroundFailure") == 0)
        return ERROR_GROUND_FAILURE;
    if (strcmp(str, "HighTemperature") == 0)
        return ERROR_HIGH_TEMPERATURE;
    if (strcmp(str, "InternalError") == 0)
        return ERROR_INTERNAL_ERROR;
    if (strcmp(str, "LocalListConflict") == 0)
        return ERROR_LOCAL_LIST_CONFLICT;
    if (strcmp(str, "OverCurrentFailure") == 0)
        return ERROR_OVER_CURRENT_FAILURE;
    if (strcmp(str, "PowerMeterFailure") == 0)
        return ERROR_POWER_METER_FAILURE;
    if (strcmp(str, "PowerSwitchFailure") == 0)
        return ERROR_POWER_SWITCH_FAILURE;
    if (strcmp(str, "ReaderFailure") == 0)
        return ERROR_READER_FAILURE;
    if (strcmp(str, "ResetFailure") == 0)
        return ERROR_RESET_FAILURE;
    if (strcmp(str, "UnderVoltage") == 0)
        return ERROR_UNDER_VOLTAGE;
    if (strcmp(str, "OverVoltage") == 0)
        return ERROR_OVER_VOLTAGE;
    if (strcmp(str, "WeakSignal") == 0)
        return ERROR_WEAK_SIGNAL;
    return ERROR_NO_ERROR;
}

bool StatusNotification::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("StatusNotification");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool StatusNotification::buildConf()
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

bool StatusNotification::setMsgSeq(quint64 i)
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

bool StatusNotification::setMsgSeq(QString i)
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

bool StatusNotification::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("StatusNotification");
        if (!actionItem)
            return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request setters =====

bool StatusNotification::setConnectorId(qint32 connectorId)
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

bool StatusNotification::setStatus(quint8 status)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!ensurePayload())
        return false;

    const char *statusStr = statusToString(status);
    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
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

bool StatusNotification::setErrorCode(quint8 errorCode)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!ensurePayload())
        return false;

    const char *errorStr = errorCodeToString(errorCode);
    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "errorCode");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(errorStr);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(payload, "errorCode", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "errorCode");
        cJSON *newItem = cJSON_CreateString(errorStr);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "errorCode", newItem);
    }

    m_errorCode = errorCode;
    return true;
}

bool StatusNotification::setInfo(const char *info)
{
    if (m_type != OCPP_CALL)
        return false;
    return setStringField("info", info);
}

bool StatusNotification::clearInfo()
{
    if (m_type != OCPP_CALL)
        return false;
    return clearField("info");
}

bool StatusNotification::setTimestamp(const char *timestamp)
{
    if (m_type != OCPP_CALL)
        return false;
    return setStringField("timestamp", timestamp);
}

bool StatusNotification::clearTimestamp()
{
    if (m_type != OCPP_CALL)
        return false;
    return clearField("timestamp");
}

bool StatusNotification::setVendorId(const char *vendorId)
{
    if (m_type != OCPP_CALL)
        return false;
    return setStringField("vendorId", vendorId);
}

bool StatusNotification::clearVendorId()
{
    if (m_type != OCPP_CALL)
        return false;
    return clearField("vendorId");
}

bool StatusNotification::setVendorErrorCode(const char *vendorErrorCode)
{
    if (m_type != OCPP_CALL)
        return false;
    return setStringField("vendorErrorCode", vendorErrorCode);
}

bool StatusNotification::clearVendorErrorCode()
{
    if (m_type != OCPP_CALL)
        return false;
    return clearField("vendorErrorCode");
}

// ===== Getters =====

quint64 StatusNotification::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem))
        return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 StatusNotification::type() const
{
    return m_type;
}

qint32 StatusNotification::connectorId() const
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

quint8 StatusNotification::status() const
{
    return m_status;
}

const char* StatusNotification::statusString() const
{
    return statusToString(m_status);
}

quint8 StatusNotification::errorCode() const
{
    return m_errorCode;
}

const char* StatusNotification::errorCodeString() const
{
    return errorCodeToString(m_errorCode);
}

const char* StatusNotification::info() const
{
    if (m_type != OCPP_CALL)
        return "";
    return getStringField("info");
}

bool StatusNotification::hasInfo() const
{
    if (m_type != OCPP_CALL)
        return false;
    return hasField("info");
}

const char* StatusNotification::timestamp() const
{
    if (m_type != OCPP_CALL)
        return "";
    return getStringField("timestamp");
}

bool StatusNotification::hasTimestamp() const
{
    if (m_type != OCPP_CALL)
        return false;
    return hasField("timestamp");
}

const char* StatusNotification::vendorId() const
{
    if (m_type != OCPP_CALL)
        return "";
    return getStringField("vendorId");
}

bool StatusNotification::hasVendorId() const
{
    if (m_type != OCPP_CALL)
        return false;
    return hasField("vendorId");
}

const char* StatusNotification::vendorErrorCode() const
{
    if (m_type != OCPP_CALL)
        return "";
    return getStringField("vendorErrorCode");
}

bool StatusNotification::hasVendorErrorCode() const
{
    if (m_type != OCPP_CALL)
        return false;
    return hasField("vendorErrorCode");
}

bool StatusNotification::isCall() const
{
    return m_type == OCPP_CALL;
}

bool StatusNotification::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool StatusNotification::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool StatusNotification::parse(const char *value)
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

bool StatusNotification::parse(cJSON *obj)
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

    quint8 parsedStatus = STATUS_AVAILABLE;
    quint8 parsedErrorCode = ERROR_NO_ERROR;

    if (msgType == OCPP_CALL) {
        if (size < 4)
            return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem))
            return false;
        if (strcmp(actionItem->valuestring, "StatusNotification") != 0)
            return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Required: connectorId
        cJSON *connItem = cJSON_GetObjectItem(payload, "connectorId");
        if (!connItem || !cJSON_IsNumber(connItem))
            return false;

        // Required: status
        cJSON *statusItem = cJSON_GetObjectItem(payload, "status");
        if (!statusItem || !cJSON_IsString(statusItem))
            return false;
        parsedStatus = stringToStatus(statusItem->valuestring);

        // Required: errorCode
        cJSON *errorItem = cJSON_GetObjectItem(payload, "errorCode");
        if (!errorItem || !cJSON_IsString(errorItem))
            return false;
        parsedErrorCode = stringToErrorCode(errorItem->valuestring);
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_status = parsedStatus;
    m_errorCode = parsedErrorCode;

    return true;
}

// ===== Serialize =====

char* StatusNotification::toJson() const
{
    if (!m_root)
        return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void StatusNotification::print() const
{
    char *data = toJson();
    if (data) {
        printf("[StatusNotification] %s\n", data);
        free(data);
    }
}
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "statusnotification.h"

int main()
{
    // ========================================
    // 示例 1: 上报充电桩可用状态
    // ========================================
    printf("=== 示例 1: 上报可用状态 ===\n");

    StatusNotification available;
    available.buildReq();
    available.setMsgSeq(1001);
    available.setConnectorId(1);
    available.setStatus(STATUS_AVAILABLE);
    available.setErrorCode(ERROR_NO_ERROR);
    available.setTimestamp("2024-06-21T08:00:00Z");

    char *json = available.toJson();
    printf("JSON: %s\n\n", json);
    free(json);

    // ========================================
    // 示例 2: 上报充电中状态
    // ========================================
    printf("=== 示例 2: 上报充电中状态 ===\n");

    StatusNotification charging;
    charging.buildReq();
    charging.setMsgSeq(1002);
    charging.setConnectorId(1);
    charging.setStatus(STATUS_CHARGING);
    charging.setErrorCode(ERROR_NO_ERROR);
    charging.setTimestamp("2024-06-21T10:30:00Z");

    json = charging.toJson();
    printf("JSON: %s\n\n", json);
    free(json);

    // ========================================
    // 示例 3: 上报故障状态（带错误码和详情）
    // ========================================
    printf("=== 示例 3: 上报故障状态 ===\n");

    StatusNotification fault;
    fault.buildReq();
    fault.setMsgSeq(1003);
    fault.setConnectorId(2);
    fault.setStatus(STATUS_FAULTED);
    fault.setErrorCode(ERROR_GROUND_FAILURE);
    fault.setInfo("Ground fault detected on phase L1");
    fault.setTimestamp("2024-06-21T14:15:30Z");
    fault.setVendorId("ACME_Charging");
    fault.setVendorErrorCode("ACME-GF-007");

    json = fault.toJson();
    printf("JSON: %s\n", json);
    printf("状态: %s\n", fault.statusString());
    printf("错误码: %s\n", fault.errorCodeString());
    printf("详情: %s\n", fault.info());
    printf("厂商: %s\n", fault.vendorId());
    printf("\n");
    free(json);

    // ========================================
    // 示例 4: 所有状态枚举演示
    // ========================================
    printf("=== 示例 4: 所有状态类型 ===\n");

    quint8 statuses[] = {
        STATUS_AVAILABLE,
        STATUS_PREPARING,
        STATUS_CHARGING,
        STATUS_SUSPENDED_EVSE,
        STATUS_SUSPENDED_EV,
        STATUS_FINISHING,
        STATUS_RESERVED,
        STATUS_UNAVAILABLE,
        STATUS_FAULTED
    };

    for (int i = 0; i < 9; i++) {
        StatusNotification msg;
        msg.buildReq();
        msg.setConnectorId(1);
        msg.setStatus(statuses[i]);
        msg.setErrorCode(ERROR_NO_ERROR);
        printf("  %2d: %s\n", i, msg.statusString());
    }
    printf("\n");

    // ========================================
    // 示例 5: 所有错误码枚举演示
    // ========================================
    printf("=== 示例 5: 所有错误码类型 ===\n");

    quint8 codes[] = {
        ERROR_NO_ERROR,
        ERROR_CONNECTOR_LOCK_FAILURE,
        ERROR_EV_COMMUNICATION_ERROR,
        ERROR_GROUND_FAILURE,
        ERROR_HIGH_TEMPERATURE,
        ERROR_INTERNAL_ERROR,
        ERROR_LOCAL_LIST_CONFLICT,
        ERROR_OVER_CURRENT_FAILURE,
        ERROR_POWER_METER_FAILURE,
        ERROR_POWER_SWITCH_FAILURE,
        ERROR_READER_FAILURE,
        ERROR_RESET_FAILURE,
        ERROR_UNDER_VOLTAGE,
        ERROR_OVER_VOLTAGE,
        ERROR_WEAK_SIGNAL
    };

    for (int i = 0; i < 15; i++) {
        StatusNotification msg;
        msg.buildReq();
        msg.setConnectorId(1);
        msg.setStatus(STATUS_FAULTED);
        msg.setErrorCode(codes[i]);
        printf("  %2d: %s\n", i, msg.errorCodeString());
    }
    printf("\n");

    // ========================================
    // 示例 6: 解析收到的状态通知
    // ========================================
    printf("=== 示例 6: 解析状态通知 ===\n");

    const char *incoming =
        "[2,\"5001\",\"StatusNotification\","
        "{\"connectorId\":3,"
        "\"status\":\"SuspendedEVSE\","
        "\"errorCode\":\"NoError\","
        "\"info\":\"EV paused charging\","
        "\"timestamp\":\"2024-06-21T16:00:00Z\"}]";

    StatusNotification parsed;
    if (parsed.parse(incoming)) {
        printf("解析成功！\n");
        printf("  连接器ID: %d\n", parsed.connectorId());
        printf("  状态: %s\n", parsed.statusString());
        printf("  错误码: %s\n", parsed.errorCodeString());
        printf("  有详情: %s\n", parsed.hasInfo() ? "是" : "否");
        printf("  详情内容: %s\n", parsed.info());
        printf("  时间戳: %s\n", parsed.timestamp());
    } else {
        printf("解析失败！\n");
    }
    printf("\n");

    // ========================================
    // 示例 7: 清除可选字段
    // ========================================
    printf("=== 示例 7: 清除可选字段 ===\n");

    StatusNotification msg;
    msg.buildReq();
    msg.setConnectorId(1);
    msg.setStatus(STATUS_AVAILABLE);
    msg.setErrorCode(ERROR_NO_ERROR);
    msg.setInfo("Some info");
    msg.setVendorId("VendorX");
    msg.setVendorErrorCode("VX-001");

    json = msg.toJson();
    printf("设置后: %s\n", json);
    free(json);

    msg.clearInfo();
    msg.clearVendorId();
    msg.clearVendorErrorCode();

    json = msg.toJson();
    printf("清除后: %s\n", json);
    free(json);
    printf("\n");

    // ========================================
    // 示例 8: 构建响应（服务器返回空对象）
    // ========================================
    printf("=== 示例 8: 构建响应 ===\n");

    StatusNotification conf;
    conf.buildConf();
    conf.setMsgSeq(1001);

    json = conf.toJson();
    printf("响应 JSON: %s\n", json);
    free(json);

    // 解析响应
    const char *confJson = "[3,\"1001\",{}]";
    StatusNotification parsedConf;
    if (parsedConf.parse(confJson)) {
        printf("解析响应成功，类型: %s\n", parsedConf.isCallResult() ? "CALLRESULT" : "其他");
    }
    printf("\n");

    // ========================================
    // 示例 9: 移动语义
    // ========================================
    printf("=== 示例 9: 移动语义 ===\n");

    StatusNotification original;
    original.buildReq();
    original.setConnectorId(5);
    original.setStatus(STATUS_CHARGING);

    printf("移动前 - original.isValid(): %d\n", original.isValid());

    StatusNotification moved = std::move(original);

    printf("移动后 - original.isValid(): %d\n", original.isValid());
    printf("移动后 - moved.isValid(): %d\n", moved.isValid());
    printf("移动后 - moved.connectorId(): %d\n", moved.connectorId());
    printf("移动后 - moved.status(): %s\n", moved.statusString());
    printf("\n");

    // ========================================
    // 示例 10: 典型状态流转场景
    // ========================================
    printf("=== 示例 10: 典型状态流转 ===\n");

    printf("  1. 充电桩启动 -> Available\n");
    printf("  2. 用户插枪 -> Preparing\n");
    printf("  3. 授权通过开始充电 -> Charging\n");
    printf("  4. 电动汽车暂停 -> SuspendedEV\n");
    printf("  5. 恢复充电 -> Charging\n");
    printf("  6. 充电完成 -> Finishing\n");
    printf("  7. 用户拔枪 -> Available\n");
    printf("  8. 故障 -> Faulted + 错误码\n");
    printf("  9. 预约中 -> Reserved\n");
    printf("  10. 维护中 -> Unavailable\n");
    printf("\n");

    printf("=== 所有示例完成 ===\n");
    return 0;
}

=== 示例 1: 上报可用状态 ===
JSON: [2,"1001","StatusNotification",{"connectorId":1,"status":"Available","errorCode":"NoError","timestamp":"2024-06-21T08:00:00Z"}]

=== 示例 3: 上报故障状态 ===
JSON: [2,"1003","StatusNotification",{"connectorId":2,"status":"Faulted","errorCode":"GroundFailure","info":"Ground fault detected on phase L1","timestamp":"2024-06-21T14:15:30Z","vendorId":"ACME_Charging","vendorErrorCode":"ACME-GF-007"}]
状态: Faulted
错误码: GroundFailure
详情: Ground fault detected on phase L1
厂商: ACME_Charging
#endif
