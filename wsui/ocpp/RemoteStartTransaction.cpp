#include "remotestarttransaction.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <utility>

RemoteStartTransaction::RemoteStartTransaction()
    : m_root(nullptr), m_type(0), m_status(REMOTE_START_REJECTED)
{
}

RemoteStartTransaction::RemoteStartTransaction(const char *value)
    : m_root(nullptr), m_type(0), m_status(REMOTE_START_REJECTED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

RemoteStartTransaction::RemoteStartTransaction(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(REMOTE_START_REJECTED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

RemoteStartTransaction::~RemoteStartTransaction()
{
    clear();
}

RemoteStartTransaction::RemoteStartTransaction(RemoteStartTransaction&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = REMOTE_START_REJECTED;
}

RemoteStartTransaction& RemoteStartTransaction::operator=(RemoteStartTransaction&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = REMOTE_START_REJECTED;
    }
    return *this;
}

void RemoteStartTransaction::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = REMOTE_START_REJECTED;
}

bool RemoteStartTransaction::ensurePayload()
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

cJSON* RemoteStartTransaction::getChargingProfile() const
{
    if (m_type != OCPP_CALL || !m_root) return nullptr;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload)) return nullptr;

    cJSON *profile = cJSON_GetObjectItem(payload, "chargingProfile");
    if (!profile || !cJSON_IsObject(profile)) return nullptr;

    return profile;
}

cJSON* RemoteStartTransaction::ensureChargingProfile()
{
    if (m_type != OCPP_CALL) return nullptr;
    if (!ensurePayload()) return nullptr;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return nullptr;

    cJSON *profile = cJSON_GetObjectItem(payload, "chargingProfile");
    if (profile && cJSON_IsObject(profile)) {
        return profile;
    }

    if (profile) {
        cJSON_DeleteItemFromObject(payload, "chargingProfile");
    }

    profile = cJSON_CreateObject();
    if (!profile) return nullptr;
    cJSON_AddItemToObject(payload, "chargingProfile", profile);

    return profile;
}

cJSON* RemoteStartTransaction::getChargingSchedule() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return nullptr;

    cJSON *schedule = cJSON_GetObjectItem(profile, "chargingSchedule");
    if (!schedule || !cJSON_IsObject(schedule)) return nullptr;

    return schedule;
}

cJSON* RemoteStartTransaction::ensureChargingSchedule()
{
    cJSON *profile = ensureChargingProfile();
    if (!profile) return nullptr;

    cJSON *schedule = cJSON_GetObjectItem(profile, "chargingSchedule");
    if (schedule && cJSON_IsObject(schedule)) {
        return schedule;
    }

    if (schedule) {
        cJSON_DeleteItemFromObject(profile, "chargingSchedule");
    }

    schedule = cJSON_CreateObject();
    if (!schedule) return nullptr;
    cJSON_AddItemToObject(profile, "chargingSchedule", schedule);

    return schedule;
}

cJSON* RemoteStartTransaction::getSchedulePeriodArray() const
{
    cJSON *schedule = getChargingSchedule();
    if (!schedule) return nullptr;

    cJSON *arr = cJSON_GetObjectItem(schedule, "chargingSchedulePeriod");
    if (!arr || !cJSON_IsArray(arr)) return nullptr;

    return arr;
}

cJSON* RemoteStartTransaction::ensureSchedulePeriodArray()
{
    cJSON *schedule = ensureChargingSchedule();
    if (!schedule) return nullptr;

    cJSON *arr = cJSON_GetObjectItem(schedule, "chargingSchedulePeriod");
    if (arr && cJSON_IsArray(arr)) {
        return arr;
    }

    if (arr) {
        cJSON_DeleteItemFromObject(schedule, "chargingSchedulePeriod");
    }

    arr = cJSON_CreateArray();
    if (!arr) return nullptr;
    cJSON_AddItemToObject(schedule, "chargingSchedulePeriod", arr);

    return arr;
}

const char* RemoteStartTransaction::statusToString(quint8 status)
{
    switch (status) {
        case REMOTE_START_ACCEPTED: return "Accepted";
        case REMOTE_START_REJECTED: return "Rejected";
        default: return "Rejected";
    }
}

quint8 RemoteStartTransaction::stringToStatus(const char *str)
{
    if (!str) return REMOTE_START_REJECTED;
    if (strcmp(str, "Accepted") == 0) return REMOTE_START_ACCEPTED;
    return REMOTE_START_REJECTED;
}

bool RemoteStartTransaction::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("RemoteStartTransaction");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool RemoteStartTransaction::buildConf()
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

bool RemoteStartTransaction::setMsgSeq(quint64 i)
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

bool RemoteStartTransaction::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("RemoteStartTransaction");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request setters =====

bool RemoteStartTransaction::setConnectorId(qint32 connectorId)
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

bool RemoteStartTransaction::clearConnectorId()
{
    if (m_type != OCPP_CALL) return false;
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON_DeleteItemFromObject(payload, "connectorId");
    return true;
}

bool RemoteStartTransaction::setIdTag(const char *idTag)
{
    if (m_type != OCPP_CALL) return false;
    if (!idTag || *idTag == '\0') return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "idTag");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(idTag);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(payload, "idTag", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "idTag");
        cJSON *newItem = cJSON_CreateString(idTag);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "idTag", newItem);
    }

    return true;
}

// ===== ChargingProfile setters =====

bool RemoteStartTransaction::setChargingProfileId(qint32 id)
{
    if (m_type != OCPP_CALL) return false;

    cJSON *profile = ensureChargingProfile();
    if (!profile) return false;

    cJSON *item = cJSON_GetObjectItem(profile, "chargingProfileId");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)id;
        item->valueint = id;
    } else {
        if (item) cJSON_DeleteItemFromObject(profile, "chargingProfileId");
        cJSON *newItem = cJSON_CreateNumber((qreal)id);
        if (!newItem) return false;
        cJSON_AddItemToObject(profile, "chargingProfileId", newItem);
    }

    return true;
}

bool RemoteStartTransaction::setChargingProfileTransactionId(qint32 id)
{
    if (m_type != OCPP_CALL) return false;

    cJSON *profile = ensureChargingProfile();
    if (!profile) return false;

    cJSON *item = cJSON_GetObjectItem(profile, "transactionId");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)id;
        item->valueint = id;
    } else {
        if (item) cJSON_DeleteItemFromObject(profile, "transactionId");
        cJSON *newItem = cJSON_CreateNumber((qreal)id);
        if (!newItem) return false;
        cJSON_AddItemToObject(profile, "transactionId", newItem);
    }

    return true;
}

bool RemoteStartTransaction::clearChargingProfileTransactionId()
{
    if (m_type != OCPP_CALL) return false;

    cJSON *profile = getChargingProfile();
    if (!profile) return false;

    cJSON_DeleteItemFromObject(profile, "transactionId");
    return true;
}

bool RemoteStartTransaction::setStackLevel(qint32 level)
{
    if (m_type != OCPP_CALL) return false;

    cJSON *profile = ensureChargingProfile();
    if (!profile) return false;

    cJSON *item = cJSON_GetObjectItem(profile, "stackLevel");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)level;
        item->valueint = level;
    } else {
        if (item) cJSON_DeleteItemFromObject(profile, "stackLevel");
        cJSON *newItem = cJSON_CreateNumber((qreal)level);
        if (!newItem) return false;
        cJSON_AddItemToObject(profile, "stackLevel", newItem);
    }

    return true;
}

bool RemoteStartTransaction::setChargingProfilePurpose(const char *purpose)
{
    if (m_type != OCPP_CALL) return false;
    if (!purpose || *purpose == '\0') return false;

    cJSON *profile = ensureChargingProfile();
    if (!profile) return false;

    cJSON *item = cJSON_GetObjectItem(profile, "chargingProfilePurpose");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(purpose);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(profile, "chargingProfilePurpose", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(profile, "chargingProfilePurpose");
        cJSON *newItem = cJSON_CreateString(purpose);
        if (!newItem) return false;
        cJSON_AddItemToObject(profile, "chargingProfilePurpose", newItem);
    }

    return true;
}

bool RemoteStartTransaction::setChargingProfileKind(const char *kind)
{
    if (m_type != OCPP_CALL) return false;
    if (!kind || *kind == '\0') return false;

    cJSON *profile = ensureChargingProfile();
    if (!profile) return false;

    cJSON *item = cJSON_GetObjectItem(profile, "chargingProfileKind");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(kind);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(profile, "chargingProfileKind", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(profile, "chargingProfileKind");
        cJSON *newItem = cJSON_CreateString(kind);
        if (!newItem) return false;
        cJSON_AddItemToObject(profile, "chargingProfileKind", newItem);
    }

    return true;
}

bool RemoteStartTransaction::setRecurrencyKind(const char *kind)
{
    if (m_type != OCPP_CALL) return false;
    if (!kind || *kind == '\0') return false;

    cJSON *profile = ensureChargingProfile();
    if (!profile) return false;

    cJSON *item = cJSON_GetObjectItem(profile, "recurrencyKind");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(kind);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(profile, "recurrencyKind", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(profile, "recurrencyKind");
        cJSON *newItem = cJSON_CreateString(kind);
        if (!newItem) return false;
        cJSON_AddItemToObject(profile, "recurrencyKind", newItem);
    }

    return true;
}

bool RemoteStartTransaction::clearRecurrencyKind()
{
    if (m_type != OCPP_CALL) return false;

    cJSON *profile = getChargingProfile();
    if (!profile) return false;

    cJSON_DeleteItemFromObject(profile, "recurrencyKind");
    return true;
}

bool RemoteStartTransaction::setValidFrom(const char *time)
{
    if (m_type != OCPP_CALL) return false;
    if (!time || *time == '\0') return false;

    cJSON *profile = ensureChargingProfile();
    if (!profile) return false;

    cJSON *item = cJSON_GetObjectItem(profile, "validFrom");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(time);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(profile, "validFrom", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(profile, "validFrom");
        cJSON *newItem = cJSON_CreateString(time);
        if (!newItem) return false;
        cJSON_AddItemToObject(profile, "validFrom", newItem);
    }

    return true;
}

bool RemoteStartTransaction::clearValidFrom()
{
    if (m_type != OCPP_CALL) return false;

    cJSON *profile = getChargingProfile();
    if (!profile) return false;

    cJSON_DeleteItemFromObject(profile, "validFrom");
    return true;
}

bool RemoteStartTransaction::setValidTo(const char *time)
{
    if (m_type != OCPP_CALL) return false;
    if (!time || *time == '\0') return false;

    cJSON *profile = ensureChargingProfile();
    if (!profile) return false;

    cJSON *item = cJSON_GetObjectItem(profile, "validTo");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(time);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(profile, "validTo", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(profile, "validTo");
        cJSON *newItem = cJSON_CreateString(time);
        if (!newItem) return false;
        cJSON_AddItemToObject(profile, "validTo", newItem);
    }

    return true;
}

bool RemoteStartTransaction::clearValidTo()
{
    if (m_type != OCPP_CALL) return false;

    cJSON *profile = getChargingProfile();
    if (!profile) return false;

    cJSON_DeleteItemFromObject(profile, "validTo");
    return true;
}

// ===== ChargingSchedule setters =====

bool RemoteStartTransaction::setChargingScheduleDuration(qint32 duration)
{
    if (m_type != OCPP_CALL) return false;

    cJSON *schedule = ensureChargingSchedule();
    if (!schedule) return false;

    cJSON *item = cJSON_GetObjectItem(schedule, "duration");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = (qreal)duration;
        item->valueint = duration;
    } else {
        if (item) cJSON_DeleteItemFromObject(schedule, "duration");
        cJSON *newItem = cJSON_CreateNumber((qreal)duration);
        if (!newItem) return false;
        cJSON_AddItemToObject(schedule, "duration", newItem);
    }

    return true;
}

bool RemoteStartTransaction::clearChargingScheduleDuration()
{
    if (m_type != OCPP_CALL) return false;

    cJSON *schedule = getChargingSchedule();
    if (!schedule) return false;

    cJSON_DeleteItemFromObject(schedule, "duration");
    return true;
}

bool RemoteStartTransaction::setStartSchedule(const char *time)
{
    if (m_type != OCPP_CALL) return false;
    if (!time || *time == '\0') return false;

    cJSON *schedule = ensureChargingSchedule();
    if (!schedule) return false;

    cJSON *item = cJSON_GetObjectItem(schedule, "startSchedule");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(time);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(schedule, "startSchedule", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(schedule, "startSchedule");
        cJSON *newItem = cJSON_CreateString(time);
        if (!newItem) return false;
        cJSON_AddItemToObject(schedule, "startSchedule", newItem);
    }

    return true;
}

bool RemoteStartTransaction::clearStartSchedule()
{
    if (m_type != OCPP_CALL) return false;

    cJSON *schedule = getChargingSchedule();
    if (!schedule) return false;

    cJSON_DeleteItemFromObject(schedule, "startSchedule");
    return true;
}

bool RemoteStartTransaction::setChargingRateUnit(const char *unit)
{
    if (m_type != OCPP_CALL) return false;
    if (!unit || *unit == '\0') return false;

    cJSON *schedule = ensureChargingSchedule();
    if (!schedule) return false;

    cJSON *item = cJSON_GetObjectItem(schedule, "chargingRateUnit");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(unit);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(schedule, "chargingRateUnit", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(schedule, "chargingRateUnit");
        cJSON *newItem = cJSON_CreateString(unit);
        if (!newItem) return false;
        cJSON_AddItemToObject(schedule, "chargingRateUnit", newItem);
    }

    return true;
}

bool RemoteStartTransaction::setMinChargingRate(qreal rate)
{
    if (m_type != OCPP_CALL) return false;

    cJSON *schedule = ensureChargingSchedule();
    if (!schedule) return false;

    cJSON *item = cJSON_GetObjectItem(schedule, "minChargingRate");
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = rate;
    } else {
        if (item) cJSON_DeleteItemFromObject(schedule, "minChargingRate");
        cJSON *newItem = cJSON_CreateNumber(rate);
        if (!newItem) return false;
        cJSON_AddItemToObject(schedule, "minChargingRate", newItem);
    }

    return true;
}

bool RemoteStartTransaction::clearMinChargingRate()
{
    if (m_type != OCPP_CALL) return false;

    cJSON *schedule = getChargingSchedule();
    if (!schedule) return false;

    cJSON_DeleteItemFromObject(schedule, "minChargingRate");
    return true;
}

int RemoteStartTransaction::addSchedulePeriod(qint32 startPeriod, qreal limit, qint32 numberPhases)
{
    if (m_type != OCPP_CALL) return -1;

    cJSON *arr = ensureSchedulePeriodArray();
    if (!arr) return -1;

    cJSON *periodObj = cJSON_CreateObject();
    if (!periodObj) return -1;

    cJSON *spItem = cJSON_CreateNumber((qreal)startPeriod);
    if (!spItem) { cJSON_Delete(periodObj); return -1; }
    cJSON_AddItemToObject(periodObj, "startPeriod", spItem);

    cJSON *limitItem = cJSON_CreateNumber(limit);
    if (!limitItem) { cJSON_Delete(periodObj); return -1; }
    cJSON_AddItemToObject(periodObj, "limit", limitItem);

    if (numberPhases > 0) {
        cJSON *npItem = cJSON_CreateNumber((qreal)numberPhases);
        if (!npItem) { cJSON_Delete(periodObj); return -1; }
        cJSON_AddItemToObject(periodObj, "numberPhases", npItem);
    }

    int index = cJSON_GetArraySize(arr);
    cJSON_AddItemToArray(arr, periodObj);

    return index;
}

bool RemoteStartTransaction::clearSchedulePeriods()
{
    if (m_type != OCPP_CALL) return false;

    cJSON *arr = getSchedulePeriodArray();
    if (!arr) return false;

    cJSON_Delete(arr);
    cJSON *schedule = getChargingSchedule();
    if (!schedule) return false;

    arr = cJSON_CreateArray();
    if (!arr) return false;
    cJSON_AddItemToObject(schedule, "chargingSchedulePeriod", arr);

    return true;
}

// ===== Response setters =====

bool RemoteStartTransaction::setStatus(quint8 status)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    const char *statusStr = statusToString(status);
    cJSON *item = cJSON_GetObjectItem(payload, "status");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(payload, "status", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "status");
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "status", newItem);
    }

    m_status = status;
    return true;
}

// ===== Getters =====

quint64 RemoteStartTransaction::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 RemoteStartTransaction::type() const
{
    return m_type;
}

qint32 RemoteStartTransaction::connectorId() const
{
    if (m_type != OCPP_CALL || !m_root) return 0;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    if (!item || !cJSON_IsNumber(item)) return 0;

    return (qint32)item->valuedouble;
}

bool RemoteStartTransaction::hasConnectorId() const
{
    if (m_type != OCPP_CALL || !m_root) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    return item != nullptr;
}

const char* RemoteStartTransaction::idTag() const
{
    if (m_type != OCPP_CALL || !m_root) return "";

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return "";

    cJSON *item = cJSON_GetObjectItem(payload, "idTag");
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

// ChargingProfile getters

qint32 RemoteStartTransaction::chargingProfileId() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return 0;

    cJSON *item = cJSON_GetObjectItem(profile, "chargingProfileId");
    if (!item || !cJSON_IsNumber(item)) return 0;

    return (qint32)item->valuedouble;
}

qint32 RemoteStartTransaction::chargingProfileTransactionId() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return 0;

    cJSON *item = cJSON_GetObjectItem(profile, "transactionId");
    if (!item || !cJSON_IsNumber(item)) return 0;

    return (qint32)item->valuedouble;
}

bool RemoteStartTransaction::hasChargingProfileTransactionId() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return false;

    cJSON *item = cJSON_GetObjectItem(profile, "transactionId");
    return item != nullptr;
}

qint32 RemoteStartTransaction::stackLevel() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return 0;

    cJSON *item = cJSON_GetObjectItem(profile, "stackLevel");
    if (!item || !cJSON_IsNumber(item)) return 0;

    return (qint32)item->valuedouble;
}

const char* RemoteStartTransaction::chargingProfilePurpose() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return "";

    cJSON *item = cJSON_GetObjectItem(profile, "chargingProfilePurpose");
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

const char* RemoteStartTransaction::chargingProfileKind() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return "";

    cJSON *item = cJSON_GetObjectItem(profile, "chargingProfileKind");
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

const char* RemoteStartTransaction::recurrencyKind() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return "";

    cJSON *item = cJSON_GetObjectItem(profile, "recurrencyKind");
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

const char* RemoteStartTransaction::validFrom() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return "";

    cJSON *item = cJSON_GetObjectItem(profile, "validFrom");
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

const char* RemoteStartTransaction::validTo() const
{
    cJSON *profile = getChargingProfile();
    if (!profile) return "";

    cJSON *item = cJSON_GetObjectItem(profile, "validTo");
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

bool RemoteStartTransaction::hasChargingProfile() const
{
    return getChargingProfile() != nullptr;
}

// ChargingSchedule getters

qint32 RemoteStartTransaction::chargingScheduleDuration() const
{
    cJSON *schedule = getChargingSchedule();
    if (!schedule) return 0;

    cJSON *item = cJSON_GetObjectItem(schedule, "duration");
    if (!item || !cJSON_IsNumber(item)) return 0;

    return (qint32)item->valuedouble;
}

bool RemoteStartTransaction::hasChargingScheduleDuration() const
{
    cJSON *schedule = getChargingSchedule();
    if (!schedule) return false;

    cJSON *item = cJSON_GetObjectItem(schedule, "duration");
    return item != nullptr;
}

const char* RemoteStartTransaction::startSchedule() const
{
    cJSON *schedule = getChargingSchedule();
    if (!schedule) return "";

    cJSON *item = cJSON_GetObjectItem(schedule, "startSchedule");
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

const char* RemoteStartTransaction::chargingRateUnit() const
{
    cJSON *schedule = getChargingSchedule();
    if (!schedule) return "";

    cJSON *item = cJSON_GetObjectItem(schedule, "chargingRateUnit");
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

qreal RemoteStartTransaction::minChargingRate() const
{
    cJSON *schedule = getChargingSchedule();
    if (!schedule) return 0.0;

    cJSON *item = cJSON_GetObjectItem(schedule, "minChargingRate");
    if (!item || !cJSON_IsNumber(item)) return 0.0;

    return item->valuedouble;
}

bool RemoteStartTransaction::hasMinChargingRate() const
{
    cJSON *schedule = getChargingSchedule();
    if (!schedule) return false;

    cJSON *item = cJSON_GetObjectItem(schedule, "minChargingRate");
    return item != nullptr;
}

int RemoteStartTransaction::schedulePeriodCount() const
{
    cJSON *arr = getSchedulePeriodArray();
    if (!arr) return 0;
    return cJSON_GetArraySize(arr);
}

bool RemoteStartTransaction::getSchedulePeriod(int index, qint32 &startPeriod, qreal &limit, qint32 &numberPhases) const
{
    cJSON *arr = getSchedulePeriodArray();
    if (!arr || index < 0) return false;
    if (index >= cJSON_GetArraySize(arr)) return false;

    cJSON *periodObj = cJSON_GetArrayItem(arr, index);
    if (!periodObj || !cJSON_IsObject(periodObj)) return false;

    cJSON *spItem = cJSON_GetObjectItem(periodObj, "startPeriod");
    if (!spItem || !cJSON_IsNumber(spItem)) return false;
    startPeriod = (qint32)spItem->valuedouble;

    cJSON *limitItem = cJSON_GetObjectItem(periodObj, "limit");
    if (!limitItem || !cJSON_IsNumber(limitItem)) return false;
    limit = limitItem->valuedouble;

    cJSON *npItem = cJSON_GetObjectItem(periodObj, "numberPhases");
    numberPhases = (npItem && cJSON_IsNumber(npItem)) ? (qint32)npItem->valuedouble : 0;

    return true;
}

// Response getters

quint8 RemoteStartTransaction::status() const
{
    return m_status;
}

const char* RemoteStartTransaction::statusString() const
{
    return statusToString(m_status);
}

bool RemoteStartTransaction::isCall() const
{
    return m_type == OCPP_CALL;
}

bool RemoteStartTransaction::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool RemoteStartTransaction::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool RemoteStartTransaction::parse(const char *value)
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

bool RemoteStartTransaction::parse(cJSON *obj)
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

    quint8 parsedStatus = REMOTE_START_REJECTED;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "RemoteStartTransaction") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // idTag is required
        cJSON *idTagItem = cJSON_GetObjectItem(payload, "idTag");
        if (!idTagItem || !cJSON_IsString(idTagItem)) return false;

        // chargingProfile is optional, but if present, validate it
        cJSON *profile = cJSON_GetObjectItem(payload, "chargingProfile");
        if (profile && cJSON_IsObject(profile)) {
            // Validate chargingProfile required fields
            cJSON *profileId = cJSON_GetObjectItem(profile, "chargingProfileId");
            if (!profileId || !cJSON_IsNumber(profileId)) return false;

            cJSON *stackLevelItem = cJSON_GetObjectItem(profile, "stackLevel");
            if (!stackLevelItem || !cJSON_IsNumber(stackLevelItem)) return false;

            cJSON *purpose = cJSON_GetObjectItem(profile, "chargingProfilePurpose");
            if (!purpose || !cJSON_IsString(purpose)) return false;

            cJSON *kind = cJSON_GetObjectItem(profile, "chargingProfileKind");
            if (!kind || !cJSON_IsString(kind)) return false;

            // Validate chargingSchedule if present
            cJSON *schedule = cJSON_GetObjectItem(profile, "chargingSchedule");
            if (schedule && cJSON_IsObject(schedule)) {
                cJSON *rateUnit = cJSON_GetObjectItem(schedule, "chargingRateUnit");
                if (!rateUnit || !cJSON_IsString(rateUnit)) return false;

                // Validate chargingSchedulePeriod if present
                cJSON *periodArr = cJSON_GetObjectItem(schedule, "chargingSchedulePeriod");
                if (periodArr && cJSON_IsArray(periodArr)) {
                    cJSON *periodObj = nullptr;
                    cJSON_ArrayForEach(periodObj, periodArr) {
                        if (!cJSON_IsObject(periodObj)) return false;

                        cJSON *sp = cJSON_GetObjectItem(periodObj, "startPeriod");
                        if (!sp || !cJSON_IsNumber(sp)) return false;

                        cJSON *lim = cJSON_GetObjectItem(periodObj, "limit");
                        if (!lim || !cJSON_IsNumber(lim)) return false;
                    }
                }
            }
        }
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;

        cJSON *statusItem = cJSON_GetObjectItem(payload, "status");
        if (!statusItem || !cJSON_IsString(statusItem)) return false;

        parsedStatus = stringToStatus(statusItem->valuestring);
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_status = parsedStatus;

    return true;
}

// ===== Serialize =====

char* RemoteStartTransaction::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void RemoteStartTransaction::print() const
{
    char *data = toJson();
    if (data) {
        printf("[RemoteStartTransaction] %s\n", data);
        free(data);
    }
}
