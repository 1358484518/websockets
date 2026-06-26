#include "getcompositeschedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Helper functions =====

const char* GetCompositeSchedule::statusToString(quint8 s) const
{
    switch (s) {
        case COMPOSITE_ACCEPTED: return "Accepted";
        case COMPOSITE_REJECTED: return "Rejected";
        default: return "Accepted";
    }
}

quint8 GetCompositeSchedule::stringToStatus(const char *str) const
{
    if (!str) return COMPOSITE_ACCEPTED;
    if (strcmp(str, "Accepted") == 0) return COMPOSITE_ACCEPTED;
    if (strcmp(str, "Rejected") == 0) return COMPOSITE_REJECTED;
    return COMPOSITE_ACCEPTED;
}

const char* GetCompositeSchedule::unitToString(quint8 u) const
{
    switch (u) {
        case RATE_UNIT_W: return "W";
        case RATE_UNIT_A: return "A";
        default: return "W";
    }
}

quint8 GetCompositeSchedule::stringToUnit(const char *str) const
{
    if (!str) return RATE_UNIT_W;
    if (strcmp(str, "W") == 0) return RATE_UNIT_W;
    if (strcmp(str, "A") == 0) return RATE_UNIT_A;
    return RATE_UNIT_W;
}

// ===== Constructors & destructor =====

GetCompositeSchedule::GetCompositeSchedule()
    : m_root(nullptr), m_type(0), m_status(COMPOSITE_ACCEPTED),
      m_connectorId(0), m_duration(0), m_rateUnit(RATE_UNIT_W)
{
}

GetCompositeSchedule::GetCompositeSchedule(const char *value)
    : m_root(nullptr), m_type(0), m_status(COMPOSITE_ACCEPTED),
      m_connectorId(0), m_duration(0), m_rateUnit(RATE_UNIT_W)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

GetCompositeSchedule::GetCompositeSchedule(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(COMPOSITE_ACCEPTED),
      m_connectorId(0), m_duration(0), m_rateUnit(RATE_UNIT_W)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

GetCompositeSchedule::~GetCompositeSchedule()
{
    clear();
}

// ===== Move semantics =====

GetCompositeSchedule::GetCompositeSchedule(GetCompositeSchedule&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status),
      m_connectorId(other.m_connectorId), m_duration(other.m_duration),
      m_rateUnit(other.m_rateUnit)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = COMPOSITE_ACCEPTED;
    other.m_connectorId = 0;
    other.m_duration = 0;
    other.m_rateUnit = RATE_UNIT_W;
}

GetCompositeSchedule& GetCompositeSchedule::operator=(GetCompositeSchedule&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        m_connectorId = other.m_connectorId;
        m_duration = other.m_duration;
        m_rateUnit = other.m_rateUnit;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = COMPOSITE_ACCEPTED;
        other.m_connectorId = 0;
        other.m_duration = 0;
        other.m_rateUnit = RATE_UNIT_W;
    }
    return *this;
}

// ===== Clear =====

void GetCompositeSchedule::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = COMPOSITE_ACCEPTED;
    m_connectorId = 0;
    m_duration = 0;
    m_rateUnit = RATE_UNIT_W;
}

// ===== Ensure helpers =====

bool GetCompositeSchedule::ensurePayload()
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

bool GetCompositeSchedule::ensureChargingSchedule()
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (schedule && cJSON_IsObject(schedule)) {
        return true;
    }

    if (schedule) {
        cJSON_DeleteItemFromObject(payload, "chargingSchedule");
    }

    schedule = cJSON_CreateObject();
    if (!schedule) return false;

    cJSON_AddItemToObject(payload, "chargingSchedule", schedule);
    return true;
}

// ===== Build =====

bool GetCompositeSchedule::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("GetCompositeSchedule");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }

    cJSON *connItem = cJSON_CreateNumber(0);
    if (!connItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "connectorId", connItem);

    cJSON *durItem = cJSON_CreateNumber(0);
    if (!durItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "duration", durItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    m_connectorId = 0;
    m_duration = 0;
    return true;
}

bool GetCompositeSchedule::buildConf()
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
    m_status = COMPOSITE_ACCEPTED;
    return true;
}

// ===== Common setters =====

bool GetCompositeSchedule::setMsgSeq(quint64 i)
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

bool GetCompositeSchedule::setMsgSeq(QString i)
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

bool GetCompositeSchedule::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("GetCompositeSchedule");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request setters =====

bool GetCompositeSchedule::setConnectorId(qint32 id)
{
    if (m_type != OCPP_CALL) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    if (item && cJSON_IsNumber(item)) {
        item->valueint = id;
        item->valuedouble = id;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "connectorId");
        cJSON *newItem = cJSON_CreateNumber(id);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "connectorId", newItem);
    }

    m_connectorId = id;
    return true;
}

bool GetCompositeSchedule::setDuration(qint32 seconds)
{
    if (m_type != OCPP_CALL) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "duration");
    if (item && cJSON_IsNumber(item)) {
        item->valueint = seconds;
        item->valuedouble = seconds;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "duration");
        cJSON *newItem = cJSON_CreateNumber(seconds);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "duration", newItem);
    }

    m_duration = seconds;
    return true;
}

bool GetCompositeSchedule::setChargingRateUnit(quint8 unit)
{
    if (m_type != OCPP_CALL) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "chargingRateUnit");
    if (item && cJSON_IsString(item)) {
        char *newStr = strdup(unitToString(unit));
        if (!newStr) return false;
        free(item->valuestring);
        item->valuestring = newStr;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "chargingRateUnit");
        cJSON *newItem = cJSON_CreateString(unitToString(unit));
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "chargingRateUnit", newItem);
    }

    m_rateUnit = unit;
    return true;
}

// ===== Response setters =====

bool GetCompositeSchedule::setStatus(quint8 s)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
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

bool GetCompositeSchedule::setScheduleStart(const char *time)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    if (!time || *time == '\0') {
        cJSON_DeleteItemFromObject(payload, "scheduleStart");
        return true;
    }

    cJSON *item = cJSON_GetObjectItem(payload, "scheduleStart");
    if (item && cJSON_IsString(item)) {
        char *newStr = strdup(time);
        if (!newStr) return false;
        free(item->valuestring);
        item->valuestring = newStr;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "scheduleStart");
        cJSON *newItem = cJSON_CreateString(time);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "scheduleStart", newItem);
    }

    return true;
}

bool GetCompositeSchedule::setConfConnectorId(qint32 id)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    if (item && cJSON_IsNumber(item)) {
        item->valueint = id;
        item->valuedouble = id;
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "connectorId");
        cJSON *newItem = cJSON_CreateNumber(id);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "connectorId", newItem);
    }

    return true;
}

// ===== Charging schedule setters =====

bool GetCompositeSchedule::setChargingScheduleDuration(qint32 seconds)
{
    if (!ensureChargingSchedule()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return false;

    cJSON *item = cJSON_GetObjectItem(schedule, "duration");
    if (item && cJSON_IsNumber(item)) {
        item->valueint = seconds;
        item->valuedouble = seconds;
    } else {
        if (item) cJSON_DeleteItemFromObject(schedule, "duration");
        cJSON *newItem = cJSON_CreateNumber(seconds);
        if (!newItem) return false;
        cJSON_AddItemToObject(schedule, "duration", newItem);
    }

    return true;
}

bool GetCompositeSchedule::setChargingScheduleStart(const char *time)
{
    if (!ensureChargingSchedule()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return false;

    if (!time || *time == '\0') {
        cJSON_DeleteItemFromObject(schedule, "startSchedule");
        return true;
    }

    cJSON *item = cJSON_GetObjectItem(schedule, "startSchedule");
    if (item && cJSON_IsString(item)) {
        char *newStr = strdup(time);
        if (!newStr) return false;
        free(item->valuestring);
        item->valuestring = newStr;
    } else {
        if (item) cJSON_DeleteItemFromObject(schedule, "startSchedule");
        cJSON *newItem = cJSON_CreateString(time);
        if (!newItem) return false;
        cJSON_AddItemToObject(schedule, "startSchedule", newItem);
    }

    return true;
}

bool GetCompositeSchedule::setChargingScheduleUnit(quint8 unit)
{
    if (!ensureChargingSchedule()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return false;

    cJSON *item = cJSON_GetObjectItem(schedule, "chargingRateUnit");
    if (item && cJSON_IsString(item)) {
        char *newStr = strdup(unitToString(unit));
        if (!newStr) return false;
        free(item->valuestring);
        item->valuestring = newStr;
    } else {
        if (item) cJSON_DeleteItemFromObject(schedule, "chargingRateUnit");
        cJSON *newItem = cJSON_CreateString(unitToString(unit));
        if (!newItem) return false;
        cJSON_AddItemToObject(schedule, "chargingRateUnit", newItem);
    }

    return true;
}

bool GetCompositeSchedule::setMinChargingRate(qreal rate)
{
    if (!ensureChargingSchedule()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
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

bool GetCompositeSchedule::addSchedulePeriod(qint32 startPeriod, qreal limit, qint32 numberPhases)
{
    if (!ensureChargingSchedule()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return false;

    cJSON *periods = cJSON_GetObjectItem(schedule, "chargingSchedulePeriod");
    if (!periods || !cJSON_IsArray(periods)) {
        if (periods) cJSON_DeleteItemFromObject(schedule, "chargingSchedulePeriod");
        periods = cJSON_CreateArray();
        if (!periods) return false;
        cJSON_AddItemToObject(schedule, "chargingSchedulePeriod", periods);
    }

    cJSON *period = cJSON_CreateObject();
    if (!period) return false;

    cJSON *spItem = cJSON_CreateNumber(startPeriod);
    if (!spItem) { cJSON_Delete(period); return false; }
    cJSON_AddItemToObject(period, "startPeriod", spItem);

    cJSON *limitItem = cJSON_CreateNumber(limit);
    if (!limitItem) { cJSON_Delete(period); return false; }
    cJSON_AddItemToObject(period, "limit", limitItem);

    if (numberPhases > 0) {
        cJSON *npItem = cJSON_CreateNumber(numberPhases);
        if (!npItem) { cJSON_Delete(period); return false; }
        cJSON_AddItemToObject(period, "numberPhases", npItem);
    }

    cJSON_AddItemToArray(periods, period);
    return true;
}

bool GetCompositeSchedule::clearSchedulePeriods()
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return true;

    cJSON_DeleteItemFromObject(schedule, "chargingSchedulePeriod");
    return true;
}

// ===== Getters =====

quint64 GetCompositeSchedule::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 GetCompositeSchedule::type() const
{
    return m_type;
}

qint32 GetCompositeSchedule::connectorId() const
{
    return m_connectorId;
}

qint32 GetCompositeSchedule::duration() const
{
    return m_duration;
}

quint8 GetCompositeSchedule::chargingRateUnit() const
{
    return m_rateUnit;
}

const char* GetCompositeSchedule::chargingRateUnitString() const
{
    return unitToString(m_rateUnit);
}

quint8 GetCompositeSchedule::status() const
{
    return m_status;
}

const char* GetCompositeSchedule::statusString() const
{
    return statusToString(m_status);
}

const char* GetCompositeSchedule::scheduleStart() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return "";
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return "";
    cJSON *item = cJSON_GetObjectItem(payload, "scheduleStart");
    if (item && cJSON_IsString(item)) return item->valuestring;
    return "";
}

qint32 GetCompositeSchedule::confConnectorId() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return 0;
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return 0;
    cJSON *item = cJSON_GetObjectItem(payload, "connectorId");
    if (item && cJSON_IsNumber(item)) return item->valueint;
    return 0;
}

qint32 GetCompositeSchedule::scheduleDuration() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return 0;
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return 0;
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return 0;
    cJSON *item = cJSON_GetObjectItem(schedule, "duration");
    if (item && cJSON_IsNumber(item)) return item->valueint;
    return 0;
}

const char* GetCompositeSchedule::scheduleStartTime() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return "";
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return "";
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return "";
    cJSON *item = cJSON_GetObjectItem(schedule, "startSchedule");
    if (item && cJSON_IsString(item)) return item->valuestring;
    return "";
}

quint8 GetCompositeSchedule::scheduleUnit() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return RATE_UNIT_W;
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return RATE_UNIT_W;
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return RATE_UNIT_W;
    cJSON *item = cJSON_GetObjectItem(schedule, "chargingRateUnit");
    if (item && cJSON_IsString(item)) return stringToUnit(item->valuestring);
    return RATE_UNIT_W;
}

const char* GetCompositeSchedule::scheduleUnitString() const
{
    return unitToString(scheduleUnit());
}

qreal GetCompositeSchedule::minChargingRate() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return 0;
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return 0;
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return 0;
    cJSON *item = cJSON_GetObjectItem(schedule, "minChargingRate");
    if (item && cJSON_IsNumber(item)) return item->valuedouble;
    return 0;
}

int GetCompositeSchedule::schedulePeriodCount() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return 0;
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return 0;
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return 0;
    cJSON *periods = cJSON_GetObjectItem(schedule, "chargingSchedulePeriod");
    if (!periods || !cJSON_IsArray(periods)) return 0;
    return cJSON_GetArraySize(periods);
}

bool GetCompositeSchedule::getSchedulePeriod(int index, qint32 &startPeriod, qreal &limit, qint32 &numberPhases) const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return false;
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;
    cJSON *schedule = cJSON_GetObjectItem(payload, "chargingSchedule");
    if (!schedule) return false;
    cJSON *periods = cJSON_GetObjectItem(schedule, "chargingSchedulePeriod");
    if (!periods || !cJSON_IsArray(periods)) return false;

    cJSON *period = cJSON_GetArrayItem(periods, index);
    if (!period || !cJSON_IsObject(period)) return false;

    cJSON *sp = cJSON_GetObjectItem(period, "startPeriod");
    cJSON *lim = cJSON_GetObjectItem(period, "limit");
    cJSON *np = cJSON_GetObjectItem(period, "numberPhases");

    if (!sp || !cJSON_IsNumber(sp) || !lim || !cJSON_IsNumber(lim)) return false;

    startPeriod = sp->valueint;
    limit = lim->valuedouble;
    numberPhases = (np && cJSON_IsNumber(np)) ? np->valueint : 0;

    return true;
}

bool GetCompositeSchedule::isCall() const
{
    return m_type == OCPP_CALL;
}

bool GetCompositeSchedule::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool GetCompositeSchedule::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool GetCompositeSchedule::parse(const char *value)
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

bool GetCompositeSchedule::parse(cJSON *obj)
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

    qint32 connectorId = 0;
    qint32 duration = 0;
    quint8 rateUnit = RATE_UNIT_W;
    quint8 status = COMPOSITE_ACCEPTED;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "GetCompositeSchedule") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        cJSON *connJson = cJSON_GetObjectItem(payload, "connectorId");
        if (connJson && cJSON_IsNumber(connJson)) {
            connectorId = connJson->valueint;
        }

        cJSON *durJson = cJSON_GetObjectItem(payload, "duration");
        if (durJson && cJSON_IsNumber(durJson)) {
            duration = durJson->valueint;
        }

        cJSON *unitJson = cJSON_GetObjectItem(payload, "chargingRateUnit");
        if (unitJson && cJSON_IsString(unitJson)) {
            rateUnit = stringToUnit(unitJson->valuestring);
        }
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;

        cJSON *statusJson = cJSON_GetObjectItem(payload, "status");
        if (statusJson && cJSON_IsString(statusJson)) {
            status = stringToStatus(statusJson->valuestring);
        }
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_status = status;
    m_connectorId = connectorId;
    m_duration = duration;
    m_rateUnit = rateUnit;

    return true;
}

// ===== Serialize =====

char* GetCompositeSchedule::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void GetCompositeSchedule::print() const
{
    char *data = toJson();
    if (data) {
        printf("[GetCompositeSchedule] %s\n", data);
        free(data);
    }
}
#if 0
setChargingScheduleDuration(seconds);
setChargingScheduleStart(timeStr);
setChargingScheduleUnit(RATE_UNIT_A);
setMinChargingRate(6.0);
addSchedulePeriod(0, 32.0, 3);    // 添加一个调度周期
clearSchedulePeriods();            // 清空所有周期

// 读取
int count = schedulePeriodCount();
getSchedulePeriod(0, sp, limit, np);  // 引用参数返回
#endif

