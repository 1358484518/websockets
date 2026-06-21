#include "StartTransaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

StartTransaction::StartTransaction()
    : m_root(nullptr), m_type(0), m_idTagStatus(IDTAG_ACCEPTED)
{
}

StartTransaction::StartTransaction(const char *value)
    : m_root(nullptr), m_type(0), m_idTagStatus(IDTAG_ACCEPTED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

StartTransaction::StartTransaction(cJSON *obj)
    : m_root(nullptr), m_type(0), m_idTagStatus(IDTAG_ACCEPTED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

StartTransaction::~StartTransaction()
{
    clear();
}

StartTransaction::StartTransaction(StartTransaction&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_idTagStatus(other.m_idTagStatus)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_idTagStatus = IDTAG_ACCEPTED;
}

StartTransaction& StartTransaction::operator=(StartTransaction&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_idTagStatus = other.m_idTagStatus;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_idTagStatus = IDTAG_ACCEPTED;
    }
    return *this;
}

void StartTransaction::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_idTagStatus = IDTAG_ACCEPTED;
}

bool StartTransaction::ensurePayload()
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

cJSON* StartTransaction::getIdTagInfo() const
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

cJSON* StartTransaction::ensureIdTagInfo()
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

const char* StartTransaction::statusToString(quint8 status)
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

quint8 StartTransaction::stringToStatus(const char *str)
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

bool StartTransaction::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("StartTransaction");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool StartTransaction::buildConf()
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

bool StartTransaction::setMsgSeq(quint64 i)
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

bool StartTransaction::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("StartTransaction");
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

bool StartTransaction::setConnectorId(qint32 connectorId)
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

bool StartTransaction::setIdTag(const char *idTag)
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

bool StartTransaction::setMeterStart(qint32 meterStart)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!ensurePayload())
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "meterStart");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)meterStart;
        item->valueint = (int)meterStart;
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "meterStart");
        cJSON *newItem = cJSON_CreateNumber((qreal)meterStart);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "meterStart", newItem);
    }

    return true;
}

bool StartTransaction::setTimestamp(const char *timestamp)
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

bool StartTransaction::setReservationId(qint32 reservationId)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!ensurePayload())
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "reservationId");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)reservationId;
        item->valueint = (int)reservationId;
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "reservationId");
        cJSON *newItem = cJSON_CreateNumber((qreal)reservationId);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "reservationId", newItem);
    }

    return true;
}

bool StartTransaction::clearReservationId()
{
    if (m_type != OCPP_CALL)
        return false;
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "reservationId");
    if (!item)
        return true;

    cJSON_DeleteItemFromObject(payload, "reservationId");
    return true;
}

// ===== Response setters =====

bool StartTransaction::setTransactionId(qint32 transactionId)
{
    if (m_type != OCPP_CALLRESULT)
        return false;
    if (!ensurePayload())
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
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

bool StartTransaction::setIdTagStatus(quint8 status)
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

bool StartTransaction::setIdTagExpiryDate(const char *expiryDate)
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

bool StartTransaction::clearIdTagExpiryDate()
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

bool StartTransaction::setIdTagParentIdTag(const char *parentIdTag)
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

bool StartTransaction::clearIdTagParentIdTag()
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

quint64 StartTransaction::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem))
        return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 StartTransaction::type() const
{
    return m_type;
}

qint32 StartTransaction::connectorId() const
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

const char* StartTransaction::idTag() const
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

qint32 StartTransaction::meterStart() const
{
    if (m_type != OCPP_CALL)
        return 0;
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "meterStart");
    if (!item || !cJSON_IsNumber(item))
        return 0;

    return (qint32)item->valuedouble;
}

const char* StartTransaction::timestamp() const
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

qint32 StartTransaction::reservationId() const
{
    if (m_type != OCPP_CALL)
        return 0;
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "reservationId");
    if (!item || !cJSON_IsNumber(item))
        return 0;

    return (qint32)item->valuedouble;
}

bool StartTransaction::hasReservationId() const
{
    if (m_type != OCPP_CALL)
        return false;
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    return cJSON_GetObjectItem(payload, "reservationId") != nullptr;
}

qint32 StartTransaction::transactionId() const
{
    if (m_type != OCPP_CALLRESULT)
        return 0;
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload || !cJSON_IsObject(payload))
        return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "transactionId");
    if (!item || !cJSON_IsNumber(item))
        return 0;

    return (qint32)item->valuedouble;
}

quint8 StartTransaction::idTagStatus() const
{
    return m_idTagStatus;
}

const char* StartTransaction::idTagStatusString() const
{
    return statusToString(m_idTagStatus);
}

const char* StartTransaction::idTagExpiryDate() const
{
    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return "";

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "expiryDate");
    if (!item || !cJSON_IsString(item))
        return "";

    return item->valuestring;
}

bool StartTransaction::hasIdTagExpiryDate() const
{
    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return false;

    return cJSON_GetObjectItem(idTagInfo, "expiryDate") != nullptr;
}

const char* StartTransaction::idTagParentIdTag() const
{
    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return "";

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "parentIdTag");
    if (!item || !cJSON_IsString(item))
        return "";

    return item->valuestring;
}

bool StartTransaction::hasIdTagParentIdTag() const
{
    cJSON *idTagInfo = getIdTagInfo();
    if (!idTagInfo)
        return false;

    return cJSON_GetObjectItem(idTagInfo, "parentIdTag") != nullptr;
}

bool StartTransaction::isCall() const
{
    return m_type == OCPP_CALL;
}

bool StartTransaction::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool StartTransaction::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool StartTransaction::parse(const char *value)
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

bool StartTransaction::parse(cJSON *obj)
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

    if (msgType == OCPP_CALL) {
        if (size < 4)
            return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem))
            return false;
        if (strcmp(actionItem->valuestring, "StartTransaction") != 0)
            return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Required: connectorId
        cJSON *connItem = cJSON_GetObjectItem(payload, "connectorId");
        if (!connItem || !cJSON_IsNumber(connItem))
            return false;

        // Required: idTag
        cJSON *idTagItem = cJSON_GetObjectItem(payload, "idTag");
        if (!idTagItem || !cJSON_IsString(idTagItem))
            return false;

        // Required: meterStart
        cJSON *meterItem = cJSON_GetObjectItem(payload, "meterStart");
        if (!meterItem || !cJSON_IsNumber(meterItem))
            return false;

        // Required: timestamp
        cJSON *tsItem = cJSON_GetObjectItem(payload, "timestamp");
        if (!tsItem || !cJSON_IsString(tsItem))
            return false;

        // Optional: reservationId - no validation needed
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Required: transactionId
        cJSON *txItem = cJSON_GetObjectItem(payload, "transactionId");
        if (!txItem || !cJSON_IsNumber(txItem))
            return false;

        // Required: idTagInfo
        cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
        if (!idTagInfo || !cJSON_IsObject(idTagInfo))
            return false;

        // Required: status in idTagInfo
        cJSON *statusItem = cJSON_GetObjectItem(idTagInfo, "status");
        if (!statusItem || !cJSON_IsString(statusItem))
            return false;

        parsedStatus = stringToStatus(statusItem->valuestring);
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_idTagStatus = parsedStatus;

    return true;
}

// ===== Serialize =====

char* StartTransaction::toJson() const
{
    if (!m_root)
        return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void StartTransaction::print() const
{
    char *data = toJson();
    if (data) {
        printf("[StartTransaction] %s\n", data);
        free(data);
    }
}

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "starttransaction.h"

int main()
{
    // ========================================
    // 示例 1: 构建请求（充电桩主动发送）
    // ========================================
    printf("=== 示例 1: 构建请求 ===\n");

    StartTransaction req;
    req.buildReq();
    req.setMsgSeq(1001);
    req.setConnectorId(1);
    req.setIdTag("TAG_1234567890");
    req.setMeterStart(50000);  // 50.00 kWh
    req.setTimestamp("2024-06-21T10:30:00Z");
    req.setReservationId(42);  // 可选

    char *json = req.toJson();
    printf("请求 JSON: %s\n\n", json);
    free(json);

    // ========================================
    // 示例 2: 构建响应（服务器返回）
    // ========================================
    printf("=== 示例 2: 构建响应 ===\n");

    StartTransaction conf;
    conf.buildConf();
    conf.setMsgSeq(1001);
    conf.setTransactionId(8888);
    conf.setIdTagStatus(IDTAG_ACCEPTED);
    conf.setIdTagExpiryDate("2025-06-21T23:59:59Z");
    conf.setIdTagParentIdTag("PARENT_TAG_001");

    json = conf.toJson();
    printf("响应 JSON: %s\n\n", json);
    free(json);

    // ========================================
    // 示例 3: 解析收到的请求
    // ========================================
    printf("=== 示例 3: 解析请求 ===\n");

    const char *incomingReq =
        "[2,\"2001\",\"StartTransaction\","
        "{\"connectorId\":2,\"idTag\":\"RFID_999\","
        "\"meterStart\":12345,\"timestamp\":\"2024-06-21T14:00:00Z\"}]";

    StartTransaction parsedReq;
    if (parsedReq.parse(incomingReq)) {
        printf("解析成功！\n");
        printf("  消息类型: %s\n", parsedReq.isCall() ? "CALL" : "其他");
        printf("  消息序号: %llu\n", parsedReq.msgSeq());
        printf("  连接器ID: %d\n", parsedReq.connectorId());
        printf("  授权标签: %s\n", parsedReq.idTag());
        printf("  起始电表: %d\n", parsedReq.meterStart());
        printf("  开始时间: %s\n", parsedReq.timestamp());
        printf("  有预约ID: %s\n", parsedReq.hasReservationId() ? "是" : "否");
    } else {
        printf("解析失败！\n");
    }
    printf("\n");

    // ========================================
    // 示例 4: 解析收到的响应
    // ========================================
    printf("=== 示例 4: 解析响应 ===\n");

    const char *incomingConf =
        "[3,\"2001\","
        "{\"transactionId\":9999,"
        "\"idTagInfo\":{\"status\":\"Accepted\","
        "\"expiryDate\":\"2024-12-31T00:00:00Z\","
        "\"parentIdTag\":\"MASTER_TAG\"}}]";

    StartTransaction parsedConf;
    if (parsedConf.parse(incomingConf)) {
        printf("解析成功！\n");
        printf("  消息类型: %s\n", parsedConf.isCallResult() ? "CALLRESULT" : "其他");
        printf("  交易ID: %d\n", parsedConf.transactionId());
        printf("  授权状态: %s\n", parsedConf.idTagStatusString());
        printf("  过期时间: %s\n", parsedConf.idTagExpiryDate());
        printf("  父标签: %s\n", parsedConf.idTagParentIdTag());
    } else {
        printf("解析失败！\n");
    }
    printf("\n");

    // ========================================
    // 示例 5: 不同状态的响应
    // ========================================
    printf("=== 示例 5: 不同授权状态 ===\n");

    quint8 statuses[] = {
        IDTAG_ACCEPTED,
        IDTAG_BLOCKED,
        IDTAG_EXPIRED,
        IDTAG_INVALID,
        IDTAG_CONCURRENT_TX
    };

    for (int i = 0; i < 5; i++) {
        StartTransaction msg;
        msg.buildConf();
        msg.setTransactionId(1000 + i);
        msg.setIdTagStatus(statuses[i]);

        json = msg.toJson();
        printf("状态 %d (%s): %s\n", i, msg.idTagStatusString(), json);
        free(json);
    }
    printf("\n");

    // ========================================
    // 示例 6: 清除可选字段
    // ========================================
    printf("=== 示例 6: 清除可选字段 ===\n");

    StartTransaction msg;
    msg.buildReq();
    msg.setConnectorId(1);
    msg.setIdTag("test");
    msg.setMeterStart(100);
    msg.setTimestamp("2024-01-01T00:00:00Z");
    msg.setReservationId(999);

    json = msg.toJson();
    printf("设置 reservationId 后: %s\n", json);
    free(json);

    msg.clearReservationId();

    json = msg.toJson();
    printf("清除 reservationId 后: %s\n", json);
    free(json);
    printf("\n");

    // ========================================
    // 示例 7: 移动语义
    // ========================================
    printf("=== 示例 7: 移动语义 ===\n");

    StartTransaction original;
    original.buildReq();
    original.setConnectorId(5);
    original.setIdTag("move_test");

    printf("移动前 - original.isValid(): %d\n", original.isValid());

    StartTransaction moved = std::move(original);

    printf("移动后 - original.isValid(): %d\n", original.isValid());
    printf("移动后 - moved.isValid(): %d\n", moved.isValid());
    printf("移动后 - moved.connectorId(): %d\n", moved.connectorId());
    printf("移动后 - moved.idTag(): %s\n", moved.idTag());
    printf("\n");

    // ========================================
    // 示例 8: 类型切换（CALL <-> CALLRESULT）
    // ========================================
    printf("=== 示例 8: 类型切换 ===\n");

    StartTransaction switchMsg;
    switchMsg.buildReq();
    switchMsg.setConnectorId(1);
    switchMsg.setIdTag("switch_test");
    switchMsg.setMeterStart(100);
    switchMsg.setTimestamp("2024-01-01T00:00:00Z");

    json = switchMsg.toJson();
    printf("CALL 类型: %s\n", json);
    free(json);

    switchMsg.setType(OCPP_CALLRESULT);

    json = switchMsg.toJson();
    printf("切换为 CALLRESULT: %s\n", json);
    free(json);

    switchMsg.setType(OCPP_CALL);

    json = switchMsg.toJson();
    printf("切回 CALL: %s\n", json);
    free(json);
    printf("\n");

    printf("=== 所有示例完成 ===\n");
    return 0;
}
=== 示例 1: 构建请求 ===
请求 JSON: [2,"1001","StartTransaction",{"connectorId":1,"idTag":"TAG_1234567890","meterStart":50000,"timestamp":"2024-06-21T10:30:00Z","reservationId":42}]

=== 示例 2: 构建响应 ===
响应 JSON: [3,"1001",{"transactionId":8888,"idTagInfo":{"status":"Accepted","expiryDate":"2025-06-21T23:59:59Z","parentIdTag":"PARENT_TAG_001"}}]

=== 示例 3: 解析请求 ===
解析成功！
  消息类型: CALL
  消息序号: 2001
  连接器ID: 2
  授权标签: RFID_999
  起始电表: 12345
  开始时间: 2024-06-21T14:00:00Z
  有预约ID: 否
#endif
