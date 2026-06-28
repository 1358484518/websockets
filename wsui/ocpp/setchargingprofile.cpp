#include "SetChargingProfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

SetChargingProfile::SetChargingProfile()
    : m_root(nullptr), m_type(0), m_respStatus(SET_PROFILE_REJECTED)
{}

SetChargingProfile::SetChargingProfile(const char *value)
    : m_root(nullptr), m_type(0), m_respStatus(SET_PROFILE_REJECTED)
{
    if (value && *value != '\0')
        parse(value);
}

SetChargingProfile::SetChargingProfile(cJSON *obj)
    : m_root(nullptr), m_type(0), m_respStatus(SET_PROFILE_REJECTED)
{
    if (obj && !parse(obj))
        cJSON_Delete(obj);
}

SetChargingProfile::~SetChargingProfile()
{
    clear();
}

SetChargingProfile::SetChargingProfile(SetChargingProfile&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_respStatus(other.m_respStatus)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_respStatus = SET_PROFILE_REJECTED;
}

SetChargingProfile& SetChargingProfile::operator=(SetChargingProfile&& other) noexcept
{
    if (this != &other)
    {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_respStatus = other.m_respStatus;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_respStatus = SET_PROFILE_REJECTED;
    }
    return *this;
}

void SetChargingProfile::clear()
{
    if (m_root)
    {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_respStatus = SET_PROFILE_REJECTED;
}

bool SetChargingProfile::ensurePayload()
{
    if (!m_root || !cJSON_IsArray(m_root))
        return false;
    int payIdx = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *pay = cJSON_GetArrayItem(m_root, payIdx);
    if (pay && cJSON_IsObject(pay))
        return true;
    if (pay)
        cJSON_DeleteItemFromArray(m_root, payIdx);
    cJSON *newPay = cJSON_CreateObject();
    if (!newPay)
        return false;
    cJSON_InsertItemInArray(m_root, payIdx, newPay);
    return true;
}

cJSON* SetChargingProfile::getChargingProfile() const
{
    if (m_type != OCPP_CALL || !m_root)
        return nullptr;
    cJSON *pay = cJSON_GetArrayItem(m_root, 3);
    if (!pay)
        return nullptr;
    cJSON *prof = cJSON_GetObjectItem(pay, "chargingProfile");
    if (!prof || !cJSON_IsObject(prof))
        return nullptr;
    return prof;
}

cJSON* SetChargingProfile::ensureChargingProfile()
{
    if (m_type != OCPP_CALL)
        return nullptr;
    if (!ensurePayload())
        return nullptr;
    cJSON *pay = cJSON_GetArrayItem(m_root, 3);
    cJSON *prof = cJSON_GetObjectItem(pay, "chargingProfile");
    if (prof && cJSON_IsObject(prof))
        return prof;
    if (prof)
        cJSON_DeleteItemFromObject(pay, "chargingProfile");
    prof = cJSON_CreateObject();
    if (!prof)
        return nullptr;
    cJSON_AddItemToObject(pay, "chargingProfile", prof);
    return prof;
}

cJSON* SetChargingProfile::getChargingSchedule() const
{
    cJSON *prof = getChargingProfile();
    if (!prof)
        return nullptr;
    cJSON *sch = cJSON_GetObjectItem(prof, "chargingSchedule");
    if (!sch || !cJSON_IsObject(sch))
        return nullptr;
    return sch;
}

cJSON* SetChargingProfile::ensureChargingSchedule()
{
    cJSON *prof = ensureChargingProfile();
    if (!prof)
        return nullptr;
    cJSON *sch = cJSON_GetObjectItem(prof, "chargingSchedule");
    if (sch && cJSON_IsObject(sch))
        return sch;
    if (sch)
        cJSON_DeleteItemFromObject(prof, "chargingSchedule");
    sch = cJSON_CreateObject();
    if (!sch)
        return nullptr;
    cJSON_AddItemToObject(prof, "chargingSchedule", sch);
    return sch;
}

cJSON* SetChargingProfile::getSchedulePeriodArray() const
{
    cJSON *sch = getChargingSchedule();
    if (!sch)
        return nullptr;
    cJSON *arr = cJSON_GetObjectItem(sch, "chargingSchedulePeriod");
    if (!arr || !cJSON_IsArray(arr))
        return nullptr;
    return arr;
}

cJSON* SetChargingProfile::ensureSchedulePeriodArray()
{
    cJSON *sch = ensureChargingSchedule();
    if (!sch)
        return nullptr;
    cJSON *arr = cJSON_GetObjectItem(sch, "chargingSchedulePeriod");
    if (arr && cJSON_IsArray(arr))
        return arr;
    if (arr)
        cJSON_DeleteItemFromObject(sch, "chargingSchedulePeriod");
    arr = cJSON_CreateArray();
    if (!arr)
        return nullptr;
    cJSON_AddItemToObject(sch, "chargingSchedulePeriod", arr);
    return arr;
}

const char* SetChargingProfile::statusToString(quint8 s)
{
    switch (s)
    {
    case SET_PROFILE_ACCEPTED: return "Accepted";
    case SET_PROFILE_NOT_SUPPORTED: return "NotSupported";
    case SET_PROFILE_REJECTED:
    default: return "Rejected";
    }
}

quint8 SetChargingProfile::stringToStatus(const char *s)
{
    if (!s) return SET_PROFILE_REJECTED;
    if (strcmp(s, "Accepted") == 0) return SET_PROFILE_ACCEPTED;
    if (strcmp(s, "NotSupported") == 0) return SET_PROFILE_NOT_SUPPORTED;
    return SET_PROFILE_REJECTED;
}

bool SetChargingProfile::buildReq()
{
    clear();
    m_root = cJSON_CreateArray();
    if (!m_root) return false;
    cJSON_AddItemToArray(m_root, cJSON_CreateNumber(OCPP_CALL));
    cJSON_AddItemToArray(m_root, cJSON_CreateString("0"));
    cJSON_AddItemToArray(m_root, cJSON_CreateString("SetChargingProfile"));
    cJSON_AddItemToArray(m_root, cJSON_CreateObject());
    m_type = OCPP_CALL;
    return true;
}

bool SetChargingProfile::buildConf()
{
    clear();
    m_root = cJSON_CreateArray();
    if (!m_root) return false;
    cJSON_AddItemToArray(m_root, cJSON_CreateNumber(OCPP_CALLRESULT));
    cJSON_AddItemToArray(m_root, cJSON_CreateString("0"));
    cJSON_AddItemToArray(m_root, cJSON_CreateObject());
    m_type = OCPP_CALLRESULT;
    return true;
}

bool SetChargingProfile::setMsgSeq(quint64 i)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    char buf[32];
    snprintf(buf, sizeof(buf), "%llu", (unsigned long long)i);
    cJSON *newId = cJSON_CreateString(buf);
    if (!newId) return false;
    cJSON *old = cJSON_GetArrayItem(m_root, 1);
    if (old)
        cJSON_ReplaceItemInArray(m_root, 1, newId);
    else
        cJSON_InsertItemInArray(m_root, 1, newId);
    return true;
}


bool SetChargingProfile::setMsgSeq(QString i)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    cJSON *newId = cJSON_CreateString(i.toUtf8().constData());
    if (!newId) return false;
    cJSON *old = cJSON_GetArrayItem(m_root, 1);
    if (old)
        cJSON_ReplaceItemInArray(m_root, 1, newId);
    else
        cJSON_InsertItemInArray(m_root, 1, newId);
    return true;
}

bool SetChargingProfile::setType(quint8 type)
{
    if (!m_root || (type != OCPP_CALL && type != OCPP_CALLRESULT))
        return false;
    if (m_type == type) return true;
    cJSON *typeItem = cJSON_GetArrayItem(m_root, 0);
    if (!typeItem) return false;
    if (m_type == OCPP_CALL && type == OCPP_CALLRESULT)
    {
        cJSON *act = cJSON_GetArrayItem(m_root, 2);
        if (act)
        {
            cJSON_DetachItemFromArray(m_root, 2);
            cJSON_Delete(act);
        }
    }
    else if (m_type == OCPP_CALLRESULT && type == OCPP_CALL)
    {
        // 指定下标2插入action字段
        cJSON_InsertItemInArray(m_root, 2, cJSON_CreateString("SetChargingProfile"));
    }
    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;
    return true;
}

// Request root field: connectorId
bool SetChargingProfile::setConnectorId(qint32 id)
{
    if (m_type != OCPP_CALL || !ensurePayload()) return false;
    cJSON *pay = cJSON_GetArrayItem(m_root, 3);
    cJSON *it = cJSON_GetObjectItem(pay, "connectorId");
    if (it && cJSON_IsNumber(it))
    {
        it->valueint = id;
        it->valuedouble = id;
    }
    else
    {
        if (it) cJSON_DeleteItemFromObject(pay, "connectorId");
        cJSON_AddItemToObject(pay, "connectorId", cJSON_CreateNumber(id));
    }
    return true;
}

// chargingProfile simple fields
bool SetChargingProfile::setChargingProfileId(qint32 id)
{
    cJSON *prof = ensureChargingProfile();
    if (!prof) return false;
    cJSON *it = cJSON_GetObjectItem(prof, "chargingProfileId");
    if (it && cJSON_IsNumber(it))
    {
        it->valueint = id;
        it->valuedouble = id;
    }
    else
    {
        if (it) cJSON_DeleteItemFromObject(prof, "chargingProfileId");
        cJSON_AddItemToObject(prof, "chargingProfileId", cJSON_CreateNumber(id));
    }
    return true;
}

bool SetChargingProfile::setTransactionId(qint32 txId)
{
    cJSON *prof = ensureChargingProfile();
    if (!prof) return false;
    cJSON *it = cJSON_GetObjectItem(prof, "transactionId");
    if (it && cJSON_IsNumber(it))
    {
        it->valueint = txId;
        it->valuedouble = txId;
    }
    else
    {
        if (it) cJSON_DeleteItemFromObject(prof, "transactionId");
        cJSON_AddItemToObject(prof, "transactionId", cJSON_CreateNumber(txId));
    }
    return true;
}

bool SetChargingProfile::clearTransactionId()
{
    cJSON *prof = getChargingProfile();
    if (!prof) return false;
    cJSON_DeleteItemFromObject(prof, "transactionId");
    return true;
}

bool SetChargingProfile::setStackLevel(qint32 level)
{
    cJSON *prof = ensureChargingProfile();
    if (!prof) return false;
    cJSON *it = cJSON_GetObjectItem(prof, "stackLevel");
    if (it && cJSON_IsNumber(it))
    {
        it->valueint = level;
        it->valuedouble = level;
    }
    else
    {
        if (it) cJSON_DeleteItemFromObject(prof, "stackLevel");
        cJSON_AddItemToObject(prof, "stackLevel", cJSON_CreateNumber(level));
    }
    return true;
}

bool SetChargingProfile::setChargingProfilePurpose(const char *purpose)
{
    if (!purpose || *purpose == '\0') return false;
    cJSON *prof = ensureChargingProfile();
    if (!prof) return false;
    cJSON *it = cJSON_GetObjectItem(prof, "chargingProfilePurpose");
    if (it && cJSON_IsString(it))
        cJSON_ReplaceItemInObject(prof, "chargingProfilePurpose", cJSON_CreateString(purpose));
    else
    {
        if (it) cJSON_DeleteItemFromObject(prof, "chargingProfilePurpose");
        cJSON_AddItemToObject(prof, "chargingProfilePurpose", cJSON_CreateString(purpose));
    }
    return true;
}

bool SetChargingProfile::setChargingProfileKind(const char *kind)
{
    if (!kind || *kind == '\0') return false;
    cJSON *prof = ensureChargingProfile();
    if (!prof) return false;
    cJSON *it = cJSON_GetObjectItem(prof, "chargingProfileKind");
    if (it && cJSON_IsString(it))
        cJSON_ReplaceItemInObject(prof, "chargingProfileKind", cJSON_CreateString(kind));
    else
    {
        if (it) cJSON_DeleteItemFromObject(prof, "chargingProfileKind");
        cJSON_AddItemToObject(prof, "chargingProfileKind", cJSON_CreateString(kind));
    }
    return true;
}

bool SetChargingProfile::setRecurrencyKind(const char *recur)
{
    if (!recur || *recur == '\0') return false;
    cJSON *prof = ensureChargingProfile();
    if (!prof) return false;
    cJSON *it = cJSON_GetObjectItem(prof, "recurrencyKind");
    if (it && cJSON_IsString(it))
        cJSON_ReplaceItemInObject(prof, "recurrencyKind", cJSON_CreateString(recur));
    else
    {
        if (it) cJSON_DeleteItemFromObject(prof, "recurrencyKind");
        cJSON_AddItemToObject(prof, "recurrencyKind", cJSON_CreateString(recur));
    }
    return true;
}

bool SetChargingProfile::clearRecurrencyKind()
{
    cJSON *prof = getChargingProfile();
    if (!prof) return false;
    cJSON_DeleteItemFromObject(prof, "recurrencyKind");
    return true;
}

bool SetChargingProfile::setValidFrom(const char *time)
{
    if (!time || *time == '\0') return false;
    cJSON *prof = ensureChargingProfile();
    if (!prof) return false;
    cJSON *it = cJSON_GetObjectItem(prof, "validFrom");
    if (it && cJSON_IsString(it))
        cJSON_ReplaceItemInObject(prof, "validFrom", cJSON_CreateString(time));
    else
    {
        if (it) cJSON_DeleteItemFromObject(prof, "validFrom");
        cJSON_AddItemToObject(prof, "validFrom", cJSON_CreateString(time));
    }
    return true;
}

bool SetChargingProfile::clearValidFrom()
{
    cJSON *prof = getChargingProfile();
    if (!prof) return false;
    cJSON_DeleteItemFromObject(prof, "validFrom");
    return true;
}

bool SetChargingProfile::setValidTo(const char *time)
{
    if (!time || *time == '\0') return false;
    cJSON *prof = ensureChargingProfile();
    if (!prof) return false;
    cJSON *it = cJSON_GetObjectItem(prof, "validTo");
    if (it && cJSON_IsString(it))
        cJSON_ReplaceItemInObject(prof, "validTo", cJSON_CreateString(time));
    else
    {
        if (it) cJSON_DeleteItemFromObject(prof, "validTo");
        cJSON_AddItemToObject(prof, "validTo", cJSON_CreateString(time));
    }
    return true;
}

bool SetChargingProfile::clearValidTo()
{
    cJSON *prof = getChargingProfile();
    if (!prof) return false;
    cJSON_DeleteItemFromObject(prof, "validTo");
    return true;
}

// ChargingSchedule fields
bool SetChargingProfile::setScheduleDuration(qint32 sec)
{
    cJSON *sch = ensureChargingSchedule();
    if (!sch) return false;
    cJSON *it = cJSON_GetObjectItem(sch, "duration");
    if (it && cJSON_IsNumber(it))
    {
        it->valueint = sec;
        it->valuedouble = sec;
    }
    else
    {
        if (it) cJSON_DeleteItemFromObject(sch, "duration");
        cJSON_AddItemToObject(sch, "duration", cJSON_CreateNumber(sec));
    }
    return true;
}

bool SetChargingProfile::clearScheduleDuration()
{
    cJSON *sch = getChargingSchedule();
    if (!sch) return false;
    cJSON_DeleteItemFromObject(sch, "duration");
    return true;
}

bool SetChargingProfile::setStartSchedule(const char *isoTime)
{
    if (!isoTime || *isoTime == '\0') return false;
    cJSON *sch = ensureChargingSchedule();
    if (!sch) return false;
    cJSON *it = cJSON_GetObjectItem(sch, "startSchedule");
    if (it && cJSON_IsString(it))
        cJSON_ReplaceItemInObject(sch, "startSchedule", cJSON_CreateString(isoTime));
    else
    {
        if (it) cJSON_DeleteItemFromObject(sch, "startSchedule");
        cJSON_AddItemToObject(sch, "startSchedule", cJSON_CreateString(isoTime));
    }
    return true;
}

bool SetChargingProfile::clearStartSchedule()
{
    cJSON *sch = getChargingSchedule();
    if (!sch) return false;
    cJSON_DeleteItemFromObject(sch, "startSchedule");
    return true;
}

bool SetChargingProfile::setChargingRateUnit(const char *unit)
{
    if (!unit || *unit == '\0') return false;
    cJSON *sch = ensureChargingSchedule();
    if (!sch) return false;
    cJSON *it = cJSON_GetObjectItem(sch, "chargingRateUnit");
    if (it && cJSON_IsString(it))
        cJSON_ReplaceItemInObject(sch, "chargingRateUnit", cJSON_CreateString(unit));
    else
    {
        if (it) cJSON_DeleteItemFromObject(sch, "chargingRateUnit");
        cJSON_AddItemToObject(sch, "chargingRateUnit", cJSON_CreateString(unit));
    }
    return true;
}

bool SetChargingProfile::setMinChargingRate(qreal rate)
{
    cJSON *sch = ensureChargingSchedule();
    if (!sch) return false;
    cJSON *it = cJSON_GetObjectItem(sch, "minChargingRate");
    if (it && cJSON_IsNumber(it))
        it->valuedouble = rate;
    else
    {
        if (it) cJSON_DeleteItemFromObject(sch, "minChargingRate");
        cJSON_AddItemToObject(sch, "minChargingRate", cJSON_CreateNumber(rate));
    }
    return true;
}

bool SetChargingProfile::clearMinChargingRate()
{
    cJSON *sch = getChargingSchedule();
    if (!sch) return false;
    cJSON_DeleteItemFromObject(sch, "minChargingRate");
    return true;
}

// Schedule Period array
int SetChargingProfile::addSchedulePeriod(qint32 startPeriod, qreal limit, qint32 numberPhases)
{
    if (m_type != OCPP_CALL) return -1;
    cJSON *arr = ensureSchedulePeriodArray();
    if (!arr) return -1;
    cJSON *obj = cJSON_CreateObject();
    if (!obj) return -1;
    cJSON_AddItemToObject(obj, "startPeriod", cJSON_CreateNumber(startPeriod));
    cJSON_AddItemToObject(obj, "limit", cJSON_CreateNumber(limit));
    if (numberPhases > 0)
        cJSON_AddItemToObject(obj, "numberPhases", cJSON_CreateNumber(numberPhases));
    int idx = cJSON_GetArraySize(arr);
    cJSON_AddItemToArray(arr, obj);
    return idx;
}

bool SetChargingProfile::clearSchedulePeriods()
{
    if (m_type != OCPP_CALL) return false;
    cJSON *arr = getSchedulePeriodArray();
    if (!arr) return true;
    cJSON_Delete(arr);
    ensureSchedulePeriodArray();
    return true;
}

// Response status
bool SetChargingProfile::setStatus(quint8 status)
{
    if (m_type != OCPP_CALLRESULT || !ensurePayload()) return false;
    cJSON *pay = cJSON_GetArrayItem(m_root, 2);
    const char *str = statusToString(status);
    cJSON *it = cJSON_GetObjectItem(pay, "status");
    if (it && cJSON_IsString(it))
        cJSON_ReplaceItemInObject(pay, "status", cJSON_CreateString(str));
    else
    {
        if (it) cJSON_DeleteItemFromObject(pay, "status");
        cJSON_AddItemToObject(pay, "status", cJSON_CreateString(str));
    }
    m_respStatus = status;
    return true;
}

// Getters
quint64 SetChargingProfile::msgSeq() const
{
    if (!m_root) return 0;
    cJSON *mid = cJSON_GetArrayItem(m_root, 1);
    if (!mid || !cJSON_IsString(mid)) return 0;
    return strtoull(mid->valuestring, nullptr, 10);
}

quint8 SetChargingProfile::type() const { return m_type; }

qint32 SetChargingProfile::connectorId() const
{
    if (m_type != OCPP_CALL || !m_root) return 0;
    cJSON *pay = cJSON_GetArrayItem(m_root, 3);
    cJSON *it = cJSON_GetObjectItem(pay, "connectorId");
    if (!it || !cJSON_IsNumber(it)) return 0;
    return (qint32)it->valuedouble;
}

qint32 SetChargingProfile::chargingProfileId() const
{
    cJSON *prof = getChargingProfile();
    if (!prof) return 0;
    cJSON *it = cJSON_GetObjectItem(prof, "chargingProfileId");
    if (!it || !cJSON_IsNumber(it)) return 0;
    return (qint32)it->valuedouble;
}

qint32 SetChargingProfile::transactionId() const
{
    cJSON *prof = getChargingProfile();
    if (!prof) return 0;
    cJSON *it = cJSON_GetObjectItem(prof, "transactionId");
    if (!it || !cJSON_IsNumber(it)) return 0;
    return (qint32)it->valuedouble;
}

bool SetChargingProfile::hasTransactionId() const
{
    cJSON *prof = getChargingProfile();
    if (!prof) return false;
    return cJSON_GetObjectItem(prof, "transactionId") != nullptr;
}

qint32 SetChargingProfile::stackLevel() const
{
    cJSON *prof = getChargingProfile();
    if (!prof) return 0;
    cJSON *it = cJSON_GetObjectItem(prof, "stackLevel");
    if (!it || !cJSON_IsNumber(it)) return 0;
    return (qint32)it->valuedouble;
}

const char* SetChargingProfile::chargingProfilePurpose() const
{
    cJSON *prof = getChargingProfile();
    if (!prof) return "";
    cJSON *it = cJSON_GetObjectItem(prof, "chargingProfilePurpose");
    if (!it || !cJSON_IsString(it)) return "";
    return it->valuestring;
}

const char* SetChargingProfile::chargingProfileKind() const
{
    cJSON *prof = getChargingProfile();
    if (!prof) return "";
    cJSON *it = cJSON_GetObjectItem(prof, "chargingProfileKind");
    if (!it || !cJSON_IsString(it)) return "";
    return it->valuestring;
}

const char* SetChargingProfile::recurrencyKind() const
{
    cJSON *prof = getChargingProfile();
    if (!prof) return "";
    cJSON *it = cJSON_GetObjectItem(prof, "recurrencyKind");
    if (!it || !cJSON_IsString(it)) return "";
    return it->valuestring;
}

const char* SetChargingProfile::validFrom() const
{
    cJSON *prof = getChargingProfile();
    if (!prof) return "";
    cJSON *it = cJSON_GetObjectItem(prof, "validFrom");
    if (!it || !cJSON_IsString(it)) return "";
    return it->valuestring;
}

const char* SetChargingProfile::validTo() const
{
    cJSON *prof = getChargingProfile();
    if (!prof) return "";
    cJSON *it = cJSON_GetObjectItem(prof, "validTo");
    if (!it || !cJSON_IsString(it)) return "";
    return it->valuestring;
}

bool SetChargingProfile::hasChargingProfile() const
{
    return getChargingProfile() != nullptr;
}

qint32 SetChargingProfile::scheduleDuration() const
{
    cJSON *sch = getChargingSchedule();
    if (!sch) return 0;
    cJSON *it = cJSON_GetObjectItem(sch, "duration");
    if (!it || !cJSON_IsNumber(it)) return 0;
    return (qint32)it->valuedouble;
}

bool SetChargingProfile::hasScheduleDuration() const
{
    cJSON *sch = getChargingSchedule();
    if (!sch) return false;
    return cJSON_GetObjectItem(sch, "duration") != nullptr;
}

const char* SetChargingProfile::startSchedule() const
{
    cJSON *sch = getChargingSchedule();
    if (!sch) return "";
    cJSON *it = cJSON_GetObjectItem(sch, "startSchedule");
    if (!it || !cJSON_IsString(it)) return "";
    return it->valuestring;
}

const char* SetChargingProfile::chargingRateUnit() const
{
    cJSON *sch = getChargingSchedule();
    if (!sch) return "";
    cJSON *it = cJSON_GetObjectItem(sch, "chargingRateUnit");
    if (!it || !cJSON_IsString(it)) return "";
    return it->valuestring;
}

qreal SetChargingProfile::minChargingRate() const
{
    cJSON *sch = getChargingSchedule();
    if (!sch) return 0.0;
    cJSON *it = cJSON_GetObjectItem(sch, "minChargingRate");
    if (!it || !cJSON_IsNumber(it)) return 0.0;
    return it->valuedouble;
}

bool SetChargingProfile::hasMinChargingRate() const
{
    cJSON *sch = getChargingSchedule();
    if (!sch) return false;
    return cJSON_GetObjectItem(sch, "minChargingRate") != nullptr;
}

int SetChargingProfile::schedulePeriodCount() const
{
    cJSON *arr = getSchedulePeriodArray();
    return arr ? cJSON_GetArraySize(arr) : 0;
}

bool SetChargingProfile::getSchedulePeriod(int idx, qint32 &startPeriod, qreal &limit, qint32 &numberPhases) const
{
    cJSON *arr = getSchedulePeriodArray();
    if (!arr || idx < 0 || idx >= cJSON_GetArraySize(arr)) return false;
    cJSON *obj = cJSON_GetArrayItem(arr, idx);
    if (!obj || !cJSON_IsObject(obj)) return false;
    cJSON *sp = cJSON_GetObjectItem(obj, "startPeriod");
    cJSON *lim = cJSON_GetObjectItem(obj, "limit");
    if (!sp || !lim || !cJSON_IsNumber(sp) || !cJSON_IsNumber(lim))
        return false;
    startPeriod = (qint32)sp->valuedouble;
    limit = lim->valuedouble;
    cJSON *np = cJSON_GetObjectItem(obj, "numberPhases");
    numberPhases = (np && cJSON_IsNumber(np)) ? (qint32)np->valuedouble : 0;
    return true;
}

quint8 SetChargingProfile::status() const { return m_respStatus; }
const char* SetChargingProfile::statusString() const { return statusToString(m_respStatus); }

bool SetChargingProfile::isCall() const { return m_type == OCPP_CALL; }
bool SetChargingProfile::isCallResult() const { return m_type == OCPP_CALLRESULT; }
bool SetChargingProfile::isValid() const { return m_root != nullptr; }

// Parse
bool SetChargingProfile::parse(const char *value)
{
    if (!value || *value == '\0') return false;
    cJSON *obj = cJSON_Parse(value);
    if (!obj) return false;
    bool ok = parse(obj);
    if (!ok) cJSON_Delete(obj);
    return ok;
}

bool SetChargingProfile::parse(cJSON *obj)
{
    if (!obj || !cJSON_IsArray(obj)) return false;
    int sz = cJSON_GetArraySize(obj);
    if (sz < 3) return false;
    cJSON *tItem = cJSON_GetArrayItem(obj, 0);
    if (!tItem || !cJSON_IsNumber(tItem)) return false;
    quint8 msgType = (quint8)tItem->valueint;
    if (msgType != OCPP_CALL && msgType != OCPP_CALLRESULT) return false;
    cJSON *msgId = cJSON_GetArrayItem(obj, 1);
    if (!msgId || !cJSON_IsString(msgId)) return false;

    quint8 respStat = SET_PROFILE_REJECTED;
    if (msgType == OCPP_CALL)
    {
        if (sz < 4) return false;
        cJSON *act = cJSON_GetArrayItem(obj, 2);
        if (!act || !cJSON_IsString(act) || strcmp(act->valuestring, "SetChargingProfile") != 0)
            return false;
        cJSON *pay = cJSON_GetArrayItem(obj, 3);
        if (!pay || !cJSON_IsObject(pay)) return false;
        // 顶层必填 connectorId
        cJSON *connId = cJSON_GetObjectItem(pay, "connectorId");
        if (!connId || !cJSON_IsNumber(connId)) return false;
        // chargingProfile 必填
        cJSON *prof = cJSON_GetObjectItem(pay, "chargingProfile");
        if (!prof || !cJSON_IsObject(prof)) return false;
        // profile 内部必填字段
        cJSON *pid = cJSON_GetObjectItem(prof, "chargingProfileId");
        cJSON *stack = cJSON_GetObjectItem(prof, "stackLevel");
        cJSON *purp = cJSON_GetObjectItem(prof, "chargingProfilePurpose");
        cJSON *kind = cJSON_GetObjectItem(prof, "chargingProfileKind");
        if (!pid || !stack || !purp || !kind ||
            !cJSON_IsNumber(pid) || !cJSON_IsNumber(stack) ||
            !cJSON_IsString(purp) || !cJSON_IsString(kind))
            return false;
        // chargingSchedule 可选，存在则校验 chargingRateUnit
        cJSON *sch = cJSON_GetObjectItem(prof, "chargingSchedule");
        if (sch && cJSON_IsObject(sch))
        {
            cJSON *rateUnit = cJSON_GetObjectItem(sch, "chargingRateUnit");
            if (!rateUnit || !cJSON_IsString(rateUnit)) return false;
            // schedulePeriod 数组可选
            cJSON *periodArr = cJSON_GetObjectItem(sch, "chargingSchedulePeriod");
            if (periodArr && cJSON_IsArray(periodArr))
            {
                cJSON *per;
                cJSON_ArrayForEach(per, periodArr)
                {
                    if (!cJSON_IsObject(per)) return false;
                    cJSON *sp = cJSON_GetObjectItem(per, "startPeriod");
                    cJSON *lim = cJSON_GetObjectItem(per, "limit");
                    if (!sp || !lim || !cJSON_IsNumber(sp) || !cJSON_IsNumber(lim))
                        return false;
                }
            }
        }
    }
    else
    {
        cJSON *pay = cJSON_GetArrayItem(obj, 2);
        if (!pay || !cJSON_IsObject(pay)) return false;
        cJSON *stat = cJSON_GetObjectItem(pay, "status");
        if (!stat || !cJSON_IsString(stat)) return false;
        respStat = stringToStatus(stat->valuestring);
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_respStatus = respStat;
    return true;
}

char* SetChargingProfile::toJson() const
{
    return m_root ? cJSON_PrintUnformatted(m_root) : nullptr;
}

void SetChargingProfile::print() const
{
    char *s = toJson();
    if (s)
    {
        printf("[SetChargingProfile] %s\n", s);
        free(s);
    }
}

#if 0
#include <stdio.h>
#include <stdlib.h>
#include "setchargingprofile.h"

int main()
{
    // 1. 构造下发充电配置请求
    SetChargingProfile req;
    req.buildReq();
    req.setMsgSeq(7001);
    req.setConnectorId(1);

    // 填充 chargingProfile 基础字段
    req.setChargingProfileId(1001);
    req.setStackLevel(1);
    req.setChargingProfilePurpose(PROFILE_PURPOSE_TX);
    req.setChargingProfileKind(PROFILE_KIND_ABSOLUTE);
    req.setValidFrom("2026-06-21T00:00:00Z");

    // 填充 chargingSchedule
    req.setScheduleDuration(3600);
    req.setChargingRateUnit(RATE_UNIT_A);
    req.setMinChargingRate(3.0);

    // 添加两段功率周期
    req.addSchedulePeriod(0, 32.0, 3);
    req.addSchedulePeriod(1800, 16.0, 3);

    printf("=== SetChargingProfile Request ===\n");
    req.print();
    printf("connectorId: %d\n", req.connectorId());
    printf("profileId: %d stack:%d purpose:%s\n",
           req.chargingProfileId(), req.stackLevel(), req.chargingProfilePurpose());
    printf("schedule unit:%s minRate:%.1f periods:%d\n",
           req.chargingRateUnit(), req.minChargingRate(), req.schedulePeriodCount());

    // 遍历调度周期
    for (int i = 0; i < req.schedulePeriodCount(); i++)
    {
        qint32 sp, np;
        qreal lim;
        if (req.getSchedulePeriod(i, sp, lim, np))
            printf("period%d start:%d limit:%.1f phases:%d\n", i, sp, lim, np);
    }

    // 2. 充电桩回复响应
    SetChargingProfile conf;
    conf.buildConf();
    conf.setMsgSeq(7001);
    conf.setStatus(SET_PROFILE_ACCEPTED);
    printf("\n=== Response ===\n");
    conf.print();
    printf("response status: %s\n", conf.statusString());

    // 3. 解析JSON字符串
    const char *jsonReq = "[2,\"7002\",\"SetChargingProfile\",{"
        "\"connectorId\":2,"
        "\"chargingProfile\":{"
            "\"chargingProfileId\":1002,"
            "\"stackLevel\":2,"
            "\"chargingProfilePurpose\":\"TxDefaultProfile\","
            "\"chargingProfileKind\":\"Recurring\","
            "\"chargingSchedule\":{"
                "\"chargingRateUnit\":\"W\","
                "\"chargingSchedulePeriod\":[{\"startPeriod\":0,\"limit\":11000}]"
            "}"
        "}}]";
    SetChargingProfile parseReq;
    if (parseReq.parse(jsonReq))
    {
        printf("\n=== Parse Request OK ===\n");
        printf("connId:%d profileId:%d unit:%s\n",
            parseReq.connectorId(), parseReq.chargingProfileId(), parseReq.chargingRateUnit());
    }

    const char *jsonConf = "[3,\"7002\",{\"status\":\"NotSupported\"}]";
    SetChargingProfile parseConf;
    if (parseConf.parse(jsonConf))
    {
        printf("\n=== Parse Response OK ===\n");
        printf("status:%s\n", parseConf.statusString());
    }

    return 0;
}
#endif

