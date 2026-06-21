#include "StopTransaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

StopTransaction::StopTransaction()
    : m_root(nullptr), m_type(0), m_idTagStatus(IDTAG_ACCEPTED),
      m_reason(STOP_REASON_OTHER), m_hasReason(false)
{
}

StopTransaction::StopTransaction(const char *value)
    : m_root(nullptr), m_type(0), m_idTagStatus(IDTAG_ACCEPTED),
      m_reason(STOP_REASON_OTHER), m_hasReason(false)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

StopTransaction::StopTransaction(cJSON *obj)
    : m_root(nullptr), m_type(0), m_idTagStatus(IDTAG_ACCEPTED),
      m_reason(STOP_REASON_OTHER), m_hasReason(false)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

StopTransaction::~StopTransaction()
{
    clear();
}

StopTransaction::StopTransaction(StopTransaction&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type),
      m_idTagStatus(other.m_idTagStatus), m_reason(other.m_reason),
      m_hasReason(other.m_hasReason)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_idTagStatus = IDTAG_ACCEPTED;
    other.m_reason = STOP_REASON_OTHER;
    other.m_hasReason = false;
}

StopTransaction& StopTransaction::operator=(StopTransaction&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_idTagStatus = other.m_idTagStatus;
        m_reason = other.m_reason;
        m_hasReason = other.m_hasReason;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_idTagStatus = IDTAG_ACCEPTED;
        other.m_reason = STOP_REASON_OTHER;
        other.m_hasReason = false;
    }
    return *this;
}

void StopTransaction::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_idTagStatus = IDTAG_ACCEPTED;
    m_reason = STOP_REASON_OTHER;
    m_hasReason = false;
}

bool StopTransaction::ensurePayload()
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

cJSON* StopTransaction::getIdTagInfo() const
{
    if (m_type != OCPP_CALLRESULT)
        return nullptr;
    if (!m_root || !cJSON_IsArray(m_root))
        return nullptr;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload || !cJSON_IsObject(payload))
        return nullptr;

    cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
    if (!idTagInfo || !cJSON_IsObject(idTagInfo))
        return nullptr;

    return idTagInfo;
}

cJSON* StopTransaction::ensureIdTagInfo()
{
    if (m_type != OCPP_CALLRESULT)
        return nullptr;
    if (!ensurePayload())
        return nullptr;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload)
        return nullptr;

    cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
    if (idTagInfo && cJSON_IsObject(idTagInfo)) {
        return idTagInfo;
    }

    if (idTagInfo) {
        cJSON_DeleteItemFromObject(payload, "idTagInfo");
    }

    cJSON *newInfo = cJSON_CreateObject();
    if (!newInfo)
        return nullptr;

    cJSON_AddItemToObject(payload, "idTagInfo", newInfo);
    return newInfo;
}

cJSON* StopTransaction::getTransactionDataArray() const
{
    if (m_type != OCPP_CALL)
        return nullptr;
    if (!m_root || !cJSON_IsArray(m_root))
        return nullptr;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return nullptr;

    cJSON *arr = cJSON_GetObjectItem(payload, "transactionData");
    if (!arr || !cJSON_IsArray(arr))
        return nullptr;

    return arr;
}

cJSON* StopTransaction::ensureTransactionDataArray()
{
    if (m_type != OCPP_CALL)
        return nullptr;
    if (!ensurePayload())
        return nullptr;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return nullptr;

    cJSON *arr = cJSON_GetObjectItem(payload, "transactionData");
    if (arr && cJSON_IsArray(arr)) {
        return arr;
    }

    if (arr) {
        cJSON_DeleteItemFromObject(payload, "transactionData");
    }

    cJSON *newArr = cJSON_CreateArray();
    if (!newArr)
        return nullptr;

    cJSON_AddItemToObject(payload, "transactionData", newArr);
    return newArr;
}

const char* StopTransaction::statusToString(quint8 status)
{
    switch (status) {
        case IDTAG_ACCEPTED:      return "Accepted";
        case IDTAG_BLOCKED:       return "Blocked";
        case IDTAG_EXPIRED:       return "Expired";
        case IDTAG_INVALID:       return "Invalid";
        case IDTAG_CONCURRENT_TX: return "ConcurrentTx";
        default:                  return "Accepted";
    }
}

quint8 StopTransaction::stringToStatus(const char *str)
{
    if (!str)
        return IDTAG_ACCEPTED;
    if (strcmp(str, "Accepted") == 0)
        return IDTAG_ACCEPTED;
    if (strcmp(str, "Blocked") == 0)
        return IDTAG_BLOCKED;
    if (strcmp(str, "Expired") == 0)
        return IDTAG_EXPIRED;
    if (strcmp(str, "Invalid") == 0)
        return IDTAG_INVALID;
    if (strcmp(str, "ConcurrentTx") == 0)
        return IDTAG_CONCURRENT_TX;
    return IDTAG_ACCEPTED;
}

const char* StopTransaction::reasonToString(quint8 reason)
{
    switch (reason) {
        case STOP_REASON_EMERGENCY_STOP:  return "EmergencyStop";
        case STOP_REASON_EV_DISCONNECTED: return "EVDisconnected";
        case STOP_REASON_HARD_RESET:      return "HardReset";
        case STOP_REASON_LOCAL:           return "Local";
        case STOP_REASON_OTHER:           return "Other";
        case STOP_REASON_POWER_LOSS:      return "PowerLoss";
        case STOP_REASON_REBOOT:          return "Reboot";
        case STOP_REASON_REMOTE:          return "Remote";
        case STOP_REASON_SOFT_RESET:      return "SoftReset";
        case STOP_REASON_UNLOCK_COMMAND:  return "UnlockCommand";
        case STOP_REASON_DEAUTHORIZED:    return "DeAuthorized";
        default:                          return "Other";
    }
}

quint8 StopTransaction::stringToReason(const char *str)
{
    if (!str)
        return STOP_REASON_OTHER;
    if (strcmp(str, "EmergencyStop") == 0)
        return STOP_REASON_EMERGENCY_STOP;
    if (strcmp(str, "EVDisconnected") == 0)
        return STOP_REASON_EV_DISCONNECTED;
    if (strcmp(str, "HardReset") == 0)
        return STOP_REASON_HARD_RESET;
    if (strcmp(str, "Local") == 0)
        return STOP_REASON_LOCAL;
    if (strcmp(str, "Other") == 0)
        return STOP_REASON_OTHER;
    if (strcmp(str, "PowerLoss") == 0)
        return STOP_REASON_POWER_LOSS;
    if (strcmp(str, "Reboot") == 0)
        return STOP_REASON_REBOOT;
    if (strcmp(str, "Remote") == 0)
        return STOP_REASON_REMOTE;
    if (strcmp(str, "SoftReset") == 0)
        return STOP_REASON_SOFT_RESET;
    if (strcmp(str, "UnlockCommand") == 0)
        return STOP_REASON_UNLOCK_COMMAND;
    if (strcmp(str, "DeAuthorized") == 0)
        return STOP_REASON_DEAUTHORIZED;
    return STOP_REASON_OTHER;
}

bool StopTransaction::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("StopTransaction");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool StopTransaction::buildConf()
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

bool StopTransaction::setMsgSeq(quint64 i)
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

bool StopTransaction::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("StopTransaction");
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

bool StopTransaction::setIdTag(const char *idTag)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!idTag || *idTag == '\0')
        return false;
    if (!ensurePayload())
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "idTag");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(idTag);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(payload, "idTag", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "idTag");
        cJSON *newItem = cJSON_CreateString(idTag);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "idTag", newItem);
    }

    return true;
}

bool StopTransaction::clearIdTag()
{
    if (m_type != OCPP_CALL)
        return false;
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "idTag");
    if (!item)
        return true;

    cJSON_DeleteItemFromObject(payload, "idTag");
    return true;
}

bool StopTransaction::setMeterStop(qint32 meterStop)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!ensurePayload())
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "meterStop");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)meterStop;
        item->valueint = (int)meterStop;
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "meterStop");
        cJSON *newItem = cJSON_CreateNumber((qreal)meterStop);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "meterStop", newItem);
    }

    return true;
}

bool StopTransaction::setTimestamp(const char *timestamp)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!timestamp || *timestamp == '\0')
        return false;
    if (!ensurePayload())
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "timestamp");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(timestamp);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(payload, "timestamp", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "timestamp");
        cJSON *newItem = cJSON_CreateString(timestamp);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "timestamp", newItem);
    }

    return true;
}

bool StopTransaction::setTransactionId(qint32 transactionId)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!ensurePayload())
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "transactionId");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)transactionId;
        item->valueint = (int)transactionId;
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "transactionId");
        cJSON *newItem = cJSON_CreateNumber((qreal)transactionId);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "transactionId", newItem);
    }

    return true;
}

bool StopTransaction::setReason(quint8 reason)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!ensurePayload())
        return false;

    const char *reasonStr = reasonToString(reason);
    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "reason");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(reasonStr);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(payload, "reason", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "reason");
        cJSON *newItem = cJSON_CreateString(reasonStr);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "reason", newItem);
    }

    m_reason = reason;
    m_hasReason = true;
    return true;
}

bool StopTransaction::clearReason()
{
    if (m_type != OCPP_CALL)
        return false;
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "reason");
    if (!item) {
        m_hasReason = false;
        return true;
    }

    cJSON_DeleteItemFromObject(payload, "reason");
    m_hasReason = false;
    return true;
}

// ===== TransactionData operations =====

int StopTransaction::addTransactionData(const char *timestamp)
{
    if (m_type != OCPP_CALL)
        return -1;
    if (!timestamp || *timestamp == '\0')
        return -1;

    cJSON *arr = ensureTransactionDataArray();
    if (!arr)
        return -1;

    cJSON *meterValue = cJSON_CreateObject();
    if (!meterValue)
        return -1;

    cJSON *tsItem = cJSON_CreateString(timestamp);
    if (!tsItem) {
        cJSON_Delete(meterValue);
        return -1;
    }
    cJSON_AddItemToObject(meterValue, "timestamp", tsItem);

    cJSON *sampledArr = cJSON_CreateArray();
    if (!sampledArr) {
        cJSON_Delete(meterValue);
        return -1;
    }
    cJSON_AddItemToObject(meterValue, "sampledValue", sampledArr);

    int index = cJSON_GetArraySize(arr);
    cJSON_AddItemToArray(arr, meterValue);
    return index;
}

int StopTransaction::addSampledValue(int dataIndex, const char *value,
                                     const char *context, const char *format,
                                     const char *measurand, const char *phase,
                                     const char *location, const char *unit)
{
    if (m_type != OCPP_CALL)
        return -1;
    if (!value || *value == '\0')
        return -1;

    cJSON *arr = getTransactionDataArray();
    if (!arr)
        return -1;

    if (dataIndex < 0 || dataIndex >= cJSON_GetArraySize(arr))
        return -1;

    cJSON *meterValue = cJSON_GetArrayItem(arr, dataIndex);
    if (!meterValue || !cJSON_IsObject(meterValue))
        return -1;

    cJSON *sampledArr = cJSON_GetObjectItem(meterValue, "sampledValue");
    if (!sampledArr || !cJSON_IsArray(sampledArr))
        return -1;

    cJSON *sample = cJSON_CreateObject();
    if (!sample)
        return -1;

    cJSON *valItem = cJSON_CreateString(value);
    if (!valItem) {
        cJSON_Delete(sample);
        return -1;
    }
    cJSON_AddItemToObject(sample, "value", valItem);

    if (context && *context != '\0') {
        cJSON *item = cJSON_CreateString(context);
        if (item)
            cJSON_AddItemToObject(sample, "context", item);
    }
    if (format && *format != '\0') {
        cJSON *item = cJSON_CreateString(format);
        if (item)
            cJSON_AddItemToObject(sample, "format", item);
    }
    if (measurand && *measurand != '\0') {
        cJSON *item = cJSON_CreateString(measurand);
        if (item)
            cJSON_AddItemToObject(sample, "measurand", item);
    }
    if (phase && *phase != '\0') {
        cJSON *item = cJSON_CreateString(phase);
        if (item)
            cJSON_AddItemToObject(sample, "phase", item);
    }
    if (location && *location != '\0') {
        cJSON *item = cJSON_CreateString(location);
        if (item)
            cJSON_AddItemToObject(sample, "location", item);
    }
    if (unit && *unit != '\0') {
        cJSON *item = cJSON_CreateString(unit);
        if (item)
            cJSON_AddItemToObject(sample, "unit", item);
    }

    int index = cJSON_GetArraySize(sampledArr);
    cJSON_AddItemToArray(sampledArr, sample);
    return index;
}

void StopTransaction::clearTransactionData()
{
    if (m_type != OCPP_CALL)
        return;
    if (!m_root || !cJSON_IsArray(m_root))
        return;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return;

    cJSON *item = cJSON_GetObjectItem(payload, "transactionData");
    if (!item)
        return;

    cJSON_DeleteItemFromObject(payload, "transactionData");
}

// ===== Response setters =====

bool StopTransaction::setIdTagStatus(quint8 status)
{
    if (m_type != OCPP_CALLRESULT)
        return false;

    cJSON *idTagInfo = ensureIdTagInfo();
    if (!idTagInfo)
        return false;

    const char *statusStr = statusToString(status);
    cJSON *item = cJSON_GetObjectItem(idTagInfo, "status");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(idTagInfo, "status", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(idTagInfo, "status");
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(idTagInfo, "status", newItem);
    }

    m_idTagStatus = status;
    return true;
}

bool StopTransaction::setIdTagExpiryDate(const char *expiryDate)
{
    if (m_type != OCPP_CALLRESULT)
        return false;
    if (!expiryDate || *expiryDate == '\0')
        return false;

    cJSON *idTagInfo = ensureIdTagInfo();
    if (!idTagInfo)
        return false;

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "expiryDate");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(expiryDate);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(idTagInfo, "expiryDate", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(idTagInfo, "expiryDate");
        cJSON *newItem = cJSON_CreateString(expiryDate);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(idTagInfo, "expiryDate", newItem);
    }

    return true;
}

bool StopTransaction::clearIdTagExpiryDate()
{
    if (m_type != OCPP_CALLRESULT)
        return false;

    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return true;

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "expiryDate");
    if (!item)
        return true;

    cJSON_DeleteItemFromObject(idTagInfo, "expiryDate");
    return true;
}

bool StopTransaction::setIdTagParentIdTag(const char *parentIdTag)
{
    if (m_type != OCPP_CALLRESULT)
        return false;
    if (!parentIdTag || *parentIdTag == '\0')
        return false;

    cJSON *idTagInfo = ensureIdTagInfo();
    if (!idTagInfo)
        return false;

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "parentIdTag");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(parentIdTag);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(idTagInfo, "parentIdTag", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(idTagInfo, "parentIdTag");
        cJSON *newItem = cJSON_CreateString(parentIdTag);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(idTagInfo, "parentIdTag", newItem);
    }

    return true;
}

bool StopTransaction::clearIdTagParentIdTag()
{
    if (m_type != OCPP_CALLRESULT)
        return false;

    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return true;

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "parentIdTag");
    if (!item)
        return true;

    cJSON_DeleteItemFromObject(idTagInfo, "parentIdTag");
    return true;
}

// ===== Getters =====

quint64 StopTransaction::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem))
        return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 StopTransaction::type() const
{
    return m_type;
}

const char* StopTransaction::idTag() const
{
    if (m_type != OCPP_CALL)
        return "";
    if (!m_root || !cJSON_IsArray(m_root))
        return "";

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return "";

    cJSON *item = cJSON_GetObjectItem(payload, "idTag");
    if (!item || !cJSON_IsString(item))
        return "";

    return item->valuestring;
}

bool StopTransaction::hasIdTag() const
{
    if (m_type != OCPP_CALL)
        return false;
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    return cJSON_GetObjectItem(payload, "idTag") != nullptr;
}

qint32 StopTransaction::meterStop() const
{
    if (m_type != OCPP_CALL)
        return 0;
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "meterStop");
    if (!item || !cJSON_IsNumber(item))
        return 0;

    return (qint32)item->valuedouble;
}

const char* StopTransaction::timestamp() const
{
    if (m_type != OCPP_CALL)
        return "";
    if (!m_root || !cJSON_IsArray(m_root))
        return "";

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return "";

    cJSON *item = cJSON_GetObjectItem(payload, "timestamp");
    if (!item || !cJSON_IsString(item))
        return "";

    return item->valuestring;
}

qint32 StopTransaction::transactionId() const
{
    if (m_type != OCPP_CALL)
        return 0;
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "transactionId");
    if (!item || !cJSON_IsNumber(item))
        return 0;

    return (qint32)item->valuedouble;
}

quint8 StopTransaction::reason() const
{
    return m_reason;
}

const char* StopTransaction::reasonString() const
{
    return reasonToString(m_reason);
}

bool StopTransaction::hasReason() const
{
    return m_hasReason;
}

int StopTransaction::transactionDataCount() const
{
    cJSON *arr = getTransactionDataArray();
    if (!arr)
        return 0;
    return cJSON_GetArraySize(arr);
}

const char* StopTransaction::transactionDataTimestamp(int index) const
{
    cJSON *arr = getTransactionDataArray();
    if (!arr)
        return "";
    if (index < 0 || index >= cJSON_GetArraySize(arr))
        return "";

    cJSON *meterValue = cJSON_GetArrayItem(arr, index);
    if (!meterValue || !cJSON_IsObject(meterValue))
        return "";

    cJSON *tsItem = cJSON_GetObjectItem(meterValue, "timestamp");
    if (!tsItem || !cJSON_IsString(tsItem))
        return "";

    return tsItem->valuestring;
}

int StopTransaction::sampledValueCount(int dataIndex) const
{
    cJSON *arr = getTransactionDataArray();
    if (!arr)
        return 0;
    if (dataIndex < 0 || dataIndex >= cJSON_GetArraySize(arr))
        return 0;

    cJSON *meterValue = cJSON_GetArrayItem(arr, dataIndex);
    if (!meterValue || !cJSON_IsObject(meterValue))
        return 0;

    cJSON *sampledArr = cJSON_GetObjectItem(meterValue, "sampledValue");
    if (!sampledArr || !cJSON_IsArray(sampledArr))
        return 0;

    return cJSON_GetArraySize(sampledArr);
}

bool StopTransaction::getSampledValue(int dataIndex, int sampleIndex,
                                      const char *&value, const char *&context,
                                      const char *&format, const char *&measurand,
                                      const char *&phase, const char *&location,
                                      const char *&unit) const
{
    value = "";
    context = "";
    format = "";
    measurand = "";
    phase = "";
    location = "";
    unit = "";

    cJSON *arr = getTransactionDataArray();
    if (!arr)
        return false;
    if (dataIndex < 0 || dataIndex >= cJSON_GetArraySize(arr))
        return false;

    cJSON *meterValue = cJSON_GetArrayItem(arr, dataIndex);
    if (!meterValue || !cJSON_IsObject(meterValue))
        return false;

    cJSON *sampledArr = cJSON_GetObjectItem(meterValue, "sampledValue");
    if (!sampledArr || !cJSON_IsArray(sampledArr))
        return false;

    if (sampleIndex < 0 || sampleIndex >= cJSON_GetArraySize(sampledArr))
        return false;

    cJSON *sample = cJSON_GetArrayItem(sampledArr, sampleIndex);
    if (!sample || !cJSON_IsObject(sample))
        return false;

    cJSON *valItem = cJSON_GetObjectItem(sample, "value");
    if (valItem && cJSON_IsString(valItem))
        value = valItem->valuestring;

    cJSON *ctxItem = cJSON_GetObjectItem(sample, "context");
    if (ctxItem && cJSON_IsString(ctxItem))
        context = ctxItem->valuestring;

    cJSON *fmtItem = cJSON_GetObjectItem(sample, "format");
    if (fmtItem && cJSON_IsString(fmtItem))
        format = fmtItem->valuestring;

    cJSON *measItem = cJSON_GetObjectItem(sample, "measurand");
    if (measItem && cJSON_IsString(measItem))
        measurand = measItem->valuestring;

    cJSON *phaseItem = cJSON_GetObjectItem(sample, "phase");
    if (phaseItem && cJSON_IsString(phaseItem))
        phase = phaseItem->valuestring;

    cJSON *locItem = cJSON_GetObjectItem(sample, "location");
    if (locItem && cJSON_IsString(locItem))
        location = locItem->valuestring;

    cJSON *unitItem = cJSON_GetObjectItem(sample, "unit");
    if (unitItem && cJSON_IsString(unitItem))
        unit = unitItem->valuestring;

    return true;
}

// ===== Response getters =====

quint8 StopTransaction::idTagStatus() const
{
    return m_idTagStatus;
}

const char* StopTransaction::idTagStatusString() const
{
    return statusToString(m_idTagStatus);
}

const char* StopTransaction::idTagExpiryDate() const
{
    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return "";

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "expiryDate");
    if (!item || !cJSON_IsString(item))
        return "";

    return item->valuestring;
}

bool StopTransaction::hasIdTagExpiryDate() const
{
    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return false;

    return cJSON_GetObjectItem(idTagInfo, "expiryDate") != nullptr;
}

const char* StopTransaction::idTagParentIdTag() const
{
    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return "";

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "parentIdTag");
    if (!item || !cJSON_IsString(item))
        return "";

    return item->valuestring;
}

bool StopTransaction::hasIdTagParentIdTag() const
{
    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return false;

    return cJSON_GetObjectItem(idTagInfo, "parentIdTag") != nullptr;
}

bool StopTransaction::hasIdTagInfo() const
{
    return getIdTagInfo() != nullptr;
}

bool StopTransaction::isCall() const
{
    return m_type == OCPP_CALL;
}

bool StopTransaction::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool StopTransaction::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool StopTransaction::parse(const char *value)
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

bool StopTransaction::parse(cJSON *obj)
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

    quint8 parsedStatus = IDTAG_ACCEPTED;
    quint8 parsedReason = STOP_REASON_OTHER;
    bool hasReason = false;

    if (msgType == OCPP_CALL) {
        if (size < 4)
            return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem))
            return false;
        if (strcmp(actionItem->valuestring, "StopTransaction") != 0)
            return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Required: meterStop
        cJSON *meterItem = cJSON_GetObjectItem(payload, "meterStop");
        if (!meterItem || !cJSON_IsNumber(meterItem))
            return false;

        // Required: timestamp
        cJSON *tsItem = cJSON_GetObjectItem(payload, "timestamp");
        if (!tsItem || !cJSON_IsString(tsItem))
            return false;

        // Required: transactionId
        cJSON *txItem = cJSON_GetObjectItem(payload, "transactionId");
        if (!txItem || !cJSON_IsNumber(txItem))
            return false;

        // Optional: idTag - no validation needed

        // Optional: reason
        cJSON *reasonItem = cJSON_GetObjectItem(payload, "reason");
        if (reasonItem && cJSON_IsString(reasonItem)) {
            parsedReason = stringToReason(reasonItem->valuestring);
            hasReason = true;
        }

        // Optional: transactionData - no validation needed (complex array)
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Optional: idTagInfo
        cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
        if (idTagInfo && cJSON_IsObject(idTagInfo)) {
            cJSON *statusItem = cJSON_GetObjectItem(idTagInfo, "status");
            if (statusItem && cJSON_IsString(statusItem)) {
                parsedStatus = stringToStatus(statusItem->valuestring);
            }
        }
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_idTagStatus = parsedStatus;
    m_reason = parsedReason;
    m_hasReason = hasReason;

    return true;
}

// ===== Serialize =====

char* StopTransaction::toJson() const
{
    if (!m_root)
        return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void StopTransaction::print() const
{
    char *data = toJson();
    if (data) {
        printf("[StopTransaction] %s\n", data);
        free(data);
    }
}

#if 0
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "stoptransaction.h"

int main()
{
    // 1. 构建停止交易请求（充电桩上报结束充电）
    printf("===== 1. 构造 StopTransaction 请求 =====\n");
    StopTransaction req;
    req.buildReq();
    req.setMsgSeq(3005);
    req.setTransactionId(10008);
    req.setMeterStop(126800);
    req.setTimestamp("2026-06-21T16:45:30Z");
    req.setIdTag("RFID_886699");
    req.setReason(STOP_REASON_EV_DISCONNECTED);

    // 添加分段电表数据 transactionData
    int dataIdx = req.addTransactionData("2026-06-21T16:00:00Z");
    req.addSampledValue(dataIdx, "120000", "SamplePeriodic", "Raw", "Energy.Active.Import", "", "Outlet", "Wh");
    dataIdx = req.addTransactionData("2026-06-21T16:45:30Z");
    req.addSampledValue(dataIdx, "126800", "SamplePeriodic", "Raw", "Energy.Active.Import", "", "Outlet", "Wh");

    char *jsonReq = req.toJson();
    printf("请求JSON：%s\n\n", jsonReq);
    free(jsonReq);

    // 2. 中央系统返回响应 CALLRESULT
    printf("===== 2. 构造 StopTransaction 响应 =====\n");
    StopTransaction conf;
    conf.buildConf();
    conf.setMsgSeq(3005);
    conf.setIdTagStatus(IDTAG_ACCEPTED);
    conf.setIdTagExpiryDate("2027-01-01T00:00:00Z");
    conf.setIdTagParentIdTag("ROOT_CARD_001");

    char *jsonConf = conf.toJson();
    printf("响应JSON：%s\n\n", jsonConf);
    free(jsonConf);

    // 3. 解析收到的请求报文
    printf("===== 3. 解析外部传入请求 =====\n");
    const char *recvReq = R"(
[2,"3010","StopTransaction",{
    "transactionId":10009,
    "meterStop":98500,
    "timestamp":"2026-06-21T17:10:00Z",
    "idTag":"TEST_CARD_01",
    "reason":"Remote"
}]
)";
    StopTransaction parseReq;
    if (parseReq.parse(recvReq))
    {
        printf("解析成功\n");
        printf("交易ID：%d\n", parseReq.transactionId());
        printf("结束电表读数：%d\n", parseReq.meterStop());
        printf("卡号：%s\n", parseReq.idTag());
        printf("停止原因：%s\n", parseReq.reasonString());
        printf("是否携带卡号：%s\n", parseReq.hasIdTag() ? "是" : "否");
        printf("是否携带停止原因：%s\n", parseReq.hasReason() ? "是" : "否");
    }
    printf("\n");

    // 4. 解析响应报文
    printf("===== 4. 解析响应报文 =====\n");
    const char *recvConf = R"(
[3,"3010",{
    "idTagInfo":{
        "status":"Accepted",
        "expiryDate":"2026-12-31T00:00:00Z"
    }
}]
)";
    StopTransaction parseConf;
    if (parseConf.parse(recvConf))
    {
        printf("响应解析成功\n");
        printf("鉴权状态：%s\n", parseConf.idTagStatusString());
        printf("过期时间：%s\n", parseConf.idTagExpiryDate());
        printf("是否存在idTagInfo：%s\n", parseConf.hasIdTagInfo() ? "是" : "否");
    }
    printf("\n");

    // 5. 清除可选字段演示
    printf("===== 5. 清除可选字段 =====\n");
    StopTransaction testClear;
    testClear.buildReq();
    testClear.setTransactionId(999);
    testClear.setMeterStop(5000);
    testClear.setTimestamp("2026-01-01T00:00:00Z");
    testClear.setIdTag("TEMP_TAG");
    testClear.setReason(STOP_REASON_POWER_LOSS);

    char *beforeClear = testClear.toJson();
    printf("清除前：%s\n", beforeClear);
    free(beforeClear);

    testClear.clearIdTag();
    testClear.clearReason();
    testClear.clearTransactionData();

    char *afterClear = testClear.toJson();
    printf("清除idTag、reason后：%s\n", afterClear);
    free(afterClear);

    return 0;
}


#endif
