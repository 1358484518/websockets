#include "MeterValues.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MeterValues::MeterValues()
    : m_root(nullptr), m_type(0)
{
}

MeterValues::MeterValues(const char *value)
    : m_root(nullptr), m_type(0)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

MeterValues::MeterValues(cJSON *obj)
    : m_root(nullptr), m_type(0)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

MeterValues::~MeterValues()
{
    clear();
}

MeterValues::MeterValues(MeterValues&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type)
{
    other.m_root = nullptr;
    other.m_type = 0;
}

MeterValues& MeterValues::operator=(MeterValues&& other) noexcept
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

void MeterValues::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
}

bool MeterValues::ensurePayload()
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

cJSON* MeterValues::getMeterValueArray() const
{
    if (m_type != OCPP_CALL || !m_root) return nullptr;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload)) return nullptr;

    cJSON *arr = cJSON_GetObjectItem(payload, "meterValue");
    if (!arr || !cJSON_IsArray(arr)) return nullptr;

    return arr;
}

cJSON* MeterValues::getSampledValueArray(cJSON *meterValue) const
{
    if (!meterValue || !cJSON_IsObject(meterValue)) return nullptr;

    cJSON *arr = cJSON_GetObjectItem(meterValue, "sampledValue");
    if (!arr || !cJSON_IsArray(arr)) return nullptr;

    return arr;
}

bool MeterValues::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("MeterValues");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool MeterValues::buildConf()
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

bool MeterValues::setMsgSeq(quint64 i)
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

bool MeterValues::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("MeterValues");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request setters =====

bool MeterValues::setConnectorId(qint32 connectorId)
{
    if (m_type != OCPP_CALL) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)connectorId;
        item->valueint = connectorId;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "connectorId");
        cJSON *newItem = cJSON_CreateNumber((qreal)connectorId);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "connectorId", newItem);
    }

    return true;
}

bool MeterValues::setTransactionId(qint32 transactionId)
{
    if (m_type != OCPP_CALL) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "transactionId");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)transactionId;
        item->valueint = transactionId;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "transactionId");
        cJSON *newItem = cJSON_CreateNumber((qreal)transactionId);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "transactionId", newItem);
    }

    return true;
}

bool MeterValues::clearTransactionId()
{
    if (m_type != OCPP_CALL) return false;
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON_DeleteItemFromObject(payload, "transactionId");
    return true;
}

int MeterValues::addMeterValue(const char *timestamp)
{
    if (m_type != OCPP_CALL) return -1;
    if (!timestamp || *timestamp == '\0') return -1;
    if (!ensurePayload()) return -1;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return -1;

    cJSON *meterValueArr = cJSON_GetObjectItem(payload, "meterValue");
    if (!meterValueArr || !cJSON_IsArray(meterValueArr)) {
        if (meterValueArr) cJSON_DeleteItemFromObject(payload, "meterValue");
        meterValueArr = cJSON_CreateArray();
        if (!meterValueArr) return -1;
        cJSON_AddItemToObject(payload, "meterValue", meterValueArr);
    }

    cJSON *meterValueObj = cJSON_CreateObject();
    if (!meterValueObj) return -1;

    cJSON *tsItem = cJSON_CreateString(timestamp);
    if (!tsItem) { cJSON_Delete(meterValueObj); return -1; }
    cJSON_AddItemToObject(meterValueObj, "timestamp", tsItem);

    cJSON *sampledValueArr = cJSON_CreateArray();
    if (!sampledValueArr) { cJSON_Delete(meterValueObj); return -1; }
    cJSON_AddItemToObject(meterValueObj, "sampledValue", sampledValueArr);

    int index = cJSON_GetArraySize(meterValueArr);
    cJSON_AddItemToArray(meterValueArr, meterValueObj);

    return index;
}

bool MeterValues::addSampledValue(int meterIndex, const char *value,
                                  const char *context,
                                  const char *format,
                                  const char *measurand,
                                  const char *phase,
                                  const char *location,
                                  const char *unit)
{
    if (m_type != OCPP_CALL) return false;
    if (!value || *value == '\0') return false;
    if (meterIndex < 0) return false;

    cJSON *meterValueArr = getMeterValueArray();
    if (!meterValueArr) return false;

    if (meterIndex >= cJSON_GetArraySize(meterValueArr)) return false;

    cJSON *meterValueObj = cJSON_GetArrayItem(meterValueArr, meterIndex);
    if (!meterValueObj || !cJSON_IsObject(meterValueObj)) return false;

    cJSON *sampledValueArr = getSampledValueArray(meterValueObj);
    if (!sampledValueArr) {
        sampledValueArr = cJSON_CreateArray();
        if (!sampledValueArr) return false;
        cJSON_AddItemToObject(meterValueObj, "sampledValue", sampledValueArr);
    }

    cJSON *sampleObj = cJSON_CreateObject();
    if (!sampleObj) return false;

    cJSON *valItem = cJSON_CreateString(value);
    if (!valItem) { cJSON_Delete(sampleObj); return false; }
    cJSON_AddItemToObject(sampleObj, "value", valItem);

    if (context && *context != '\0') {
        cJSON *item = cJSON_CreateString(context);
        if (!item) { cJSON_Delete(sampleObj); return false; }
        cJSON_AddItemToObject(sampleObj, "context", item);
    }

    if (format && *format != '\0') {
        cJSON *item = cJSON_CreateString(format);
        if (!item) { cJSON_Delete(sampleObj); return false; }
        cJSON_AddItemToObject(sampleObj, "format", item);
    }

    if (measurand && *measurand != '\0') {
        cJSON *item = cJSON_CreateString(measurand);
        if (!item) { cJSON_Delete(sampleObj); return false; }
        cJSON_AddItemToObject(sampleObj, "measurand", item);
    }

    if (phase && *phase != '\0') {
        cJSON *item = cJSON_CreateString(phase);
        if (!item) { cJSON_Delete(sampleObj); return false; }
        cJSON_AddItemToObject(sampleObj, "phase", item);
    }

    if (location && *location != '\0') {
        cJSON *item = cJSON_CreateString(location);
        if (!item) { cJSON_Delete(sampleObj); return false; }
        cJSON_AddItemToObject(sampleObj, "location", item);
    }

    if (unit && *unit != '\0') {
        cJSON *item = cJSON_CreateString(unit);
        if (!item) { cJSON_Delete(sampleObj); return false; }
        cJSON_AddItemToObject(sampleObj, "unit", item);
    }

    cJSON_AddItemToArray(sampledValueArr, sampleObj);
    return true;
}

// ===== Getters =====

quint64 MeterValues::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 MeterValues::type() const
{
    return m_type;
}

qint32 MeterValues::connectorId() const
{
    if (m_type != OCPP_CALL || !m_root) return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    if (!item || !cJSON_IsNumber(item)) return 0;

    return (qint32)item->valuedouble;
}

qint32 MeterValues::transactionId() const
{
    if (m_type != OCPP_CALL || !m_root) return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "transactionId");
    if (!item || !cJSON_IsNumber(item)) return 0;

    return (qint32)item->valuedouble;
}

bool MeterValues::hasTransactionId() const
{
    if (m_type != OCPP_CALL || !m_root) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "transactionId");
    return item != nullptr;
}

int MeterValues::meterValueCount() const
{
    cJSON *arr = getMeterValueArray();
    if (!arr) return 0;
    return cJSON_GetArraySize(arr);
}

const char* MeterValues::meterValueTimestamp(int index) const
{
    cJSON *arr = getMeterValueArray();
    if (!arr || index < 0) return "";
    if (index >= cJSON_GetArraySize(arr)) return "";

    cJSON *meterValueObj = cJSON_GetArrayItem(arr, index);
    if (!meterValueObj || !cJSON_IsObject(meterValueObj)) return "";

    cJSON *tsItem = cJSON_GetObjectItem(meterValueObj, "timestamp");
    if (!tsItem || !cJSON_IsString(tsItem)) return "";

    return tsItem->valuestring;
}

int MeterValues::sampledValueCount(int meterIndex) const
{
    cJSON *arr = getMeterValueArray();
    if (!arr || meterIndex < 0) return 0;
    if (meterIndex >= cJSON_GetArraySize(arr)) return 0;

    cJSON *meterValueObj = cJSON_GetArrayItem(arr, meterIndex);
    if (!meterValueObj) return 0;

    cJSON *sampledArr = getSampledValueArray(meterValueObj);
    if (!sampledArr) return 0;

    return cJSON_GetArraySize(sampledArr);
}

bool MeterValues::getSampledValue(int meterIndex, int sampleIndex,
                                  const char *&value,
                                  const char *&context,
                                  const char *&format,
                                  const char *&measurand,
                                  const char *&phase,
                                  const char *&location,
                                  const char *&unit) const
{
    cJSON *arr = getMeterValueArray();
    if (!arr || meterIndex < 0) return false;
    if (meterIndex >= cJSON_GetArraySize(arr)) return false;

    cJSON *meterValueObj = cJSON_GetArrayItem(arr, meterIndex);
    if (!meterValueObj) return false;

    cJSON *sampledArr = getSampledValueArray(meterValueObj);
    if (!sampledArr || sampleIndex < 0) return false;
    if (sampleIndex >= cJSON_GetArraySize(sampledArr)) return false;

    cJSON *sampleObj = cJSON_GetArrayItem(sampledArr, sampleIndex);
    if (!sampleObj || !cJSON_IsObject(sampleObj)) return false;

    cJSON *valItem = cJSON_GetObjectItem(sampleObj, "value");
    if (!valItem || !cJSON_IsString(valItem)) return false;
    value = valItem->valuestring;

    cJSON *ctxItem = cJSON_GetObjectItem(sampleObj, "context");
    context = (ctxItem && cJSON_IsString(ctxItem)) ? ctxItem->valuestring : "";

    cJSON *fmtItem = cJSON_GetObjectItem(sampleObj, "format");
    format = (fmtItem && cJSON_IsString(fmtItem)) ? fmtItem->valuestring : "";

    cJSON *measItem = cJSON_GetObjectItem(sampleObj, "measurand");
    measurand = (measItem && cJSON_IsString(measItem)) ? measItem->valuestring : "";

    cJSON *phaseItem = cJSON_GetObjectItem(sampleObj, "phase");
    phase = (phaseItem && cJSON_IsString(phaseItem)) ? phaseItem->valuestring : "";

    cJSON *locItem = cJSON_GetObjectItem(sampleObj, "location");
    location = (locItem && cJSON_IsString(locItem)) ? locItem->valuestring : "";

    cJSON *unitItem = cJSON_GetObjectItem(sampleObj, "unit");
    unit = (unitItem && cJSON_IsString(unitItem)) ? unitItem->valuestring : "";

    return true;
}

bool MeterValues::isCall() const
{
    return m_type == OCPP_CALL;
}

bool MeterValues::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool MeterValues::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool MeterValues::parse(const char *value)
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

bool MeterValues::parse(cJSON *obj)
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
        if (strcmp(actionItem->valuestring, "MeterValues") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // connectorId is required
        cJSON *connId = cJSON_GetObjectItem(payload, "connectorId");
        if (!connId || !cJSON_IsNumber(connId)) return false;

        // meterValue is required
        cJSON *meterValueArr = cJSON_GetObjectItem(payload, "meterValue");
        if (!meterValueArr || !cJSON_IsArray(meterValueArr)) return false;

        // Validate each meterValue
        cJSON *meterValueObj = nullptr;
        cJSON_ArrayForEach(meterValueObj, meterValueArr) {
            if (!cJSON_IsObject(meterValueObj)) return false;

            cJSON *ts = cJSON_GetObjectItem(meterValueObj, "timestamp");
            if (!ts || !cJSON_IsString(ts)) return false;

            cJSON *sampledValueArr = cJSON_GetObjectItem(meterValueObj, "sampledValue");
            if (!sampledValueArr || !cJSON_IsArray(sampledValueArr)) return false;

            // Validate each sampledValue
            cJSON *sampleObj = nullptr;
            cJSON_ArrayForEach(sampleObj, sampledValueArr) {
                if (!cJSON_IsObject(sampleObj)) return false;

                cJSON *val = cJSON_GetObjectItem(sampleObj, "value");
                if (!val || !cJSON_IsString(val)) return false;
            }
        }
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;
    }

    clear();
    m_root = obj;
    m_type = msgType;

    return true;
}

// ===== Serialize =====

char* MeterValues::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void MeterValues::print() const
{
    char *data = toJson();
    if (data) {
        printf("[MeterValues] %s\n", data);
        free(data);
    }
}
