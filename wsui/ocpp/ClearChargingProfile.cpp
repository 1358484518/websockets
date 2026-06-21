#include "clearchargingprofile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Purpose Conversion =====

const char* ClearChargingProfile::purposeToString(quint8 p) const
{
    switch (p) {
        case CHARGE_POINT_MAX_PROFILE: return "ChargePointMaxProfile";
        case TX_DEFAULT_PROFILE:       return "TxDefaultProfile";
        case TX_PROFILE:               return "TxProfile";
        default: return "ChargePointMaxProfile";
    }
}

quint8 ClearChargingProfile::stringToPurpose(const char *str) const
{
    if (!str) return CHARGE_POINT_MAX_PROFILE;
    if (strcmp(str, "ChargePointMaxProfile") == 0) return CHARGE_POINT_MAX_PROFILE;
    if (strcmp(str, "TxDefaultProfile") == 0)       return TX_DEFAULT_PROFILE;
    if (strcmp(str, "TxProfile") == 0)               return TX_PROFILE;
    return CHARGE_POINT_MAX_PROFILE;
}

// ===== Status Conversion =====

const char* ClearChargingProfile::statusToString(quint8 s) const
{
    switch (s) {
        case CLEAR_PROFILE_ACCEPTED: return "Accepted";
        case CLEAR_PROFILE_UNKNOWN:  return "Unknown";
        default: return "Unknown";
    }
}

quint8 ClearChargingProfile::stringToStatus(const char *str) const
{
    if (!str) return CLEAR_PROFILE_UNKNOWN;
    if (strcmp(str, "Accepted") == 0) return CLEAR_PROFILE_ACCEPTED;
    if (strcmp(str, "Unknown") == 0)  return CLEAR_PROFILE_UNKNOWN;
    return CLEAR_PROFILE_UNKNOWN;
}

// ===== Construction / Destruction =====

ClearChargingProfile::ClearChargingProfile()
    : m_root(nullptr), m_type(0), m_status(CLEAR_PROFILE_UNKNOWN),
      m_id(-1), m_connectorId(-1), m_purpose(CHARGE_POINT_MAX_PROFILE) {}

ClearChargingProfile::ClearChargingProfile(const char *value)
    : m_root(nullptr), m_type(0), m_status(CLEAR_PROFILE_UNKNOWN),
      m_id(-1), m_connectorId(-1), m_purpose(CHARGE_POINT_MAX_PROFILE)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

ClearChargingProfile::ClearChargingProfile(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(CLEAR_PROFILE_UNKNOWN),
      m_id(-1), m_connectorId(-1), m_purpose(CHARGE_POINT_MAX_PROFILE)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

ClearChargingProfile::~ClearChargingProfile() { clear(); }

ClearChargingProfile::ClearChargingProfile(ClearChargingProfile&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status),
      m_id(other.m_id), m_connectorId(other.m_connectorId), m_purpose(other.m_purpose)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = CLEAR_PROFILE_UNKNOWN;
    other.m_id = -1;
    other.m_connectorId = -1;
    other.m_purpose = CHARGE_POINT_MAX_PROFILE;
}

ClearChargingProfile& ClearChargingProfile::operator=(ClearChargingProfile&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        m_id = other.m_id;
        m_connectorId = other.m_connectorId;
        m_purpose = other.m_purpose;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = CLEAR_PROFILE_UNKNOWN;
        other.m_id = -1;
        other.m_connectorId = -1;
        other.m_purpose = CHARGE_POINT_MAX_PROFILE;
    }
    return *this;
}

void ClearChargingProfile::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = CLEAR_PROFILE_UNKNOWN;
    m_id = -1;
    m_connectorId = -1;
    m_purpose = CHARGE_POINT_MAX_PROFILE;
}

// ===== Internal Helpers =====

bool ClearChargingProfile::ensurePayload()
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

bool ClearChargingProfile::setStringField(const char *key, const char *value)
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

bool ClearChargingProfile::setNumberField(const char *key, double value)
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

const char* ClearChargingProfile::getStringField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return "";
    if (!key) return "";

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return "";

    cJSON *item = cJSON_GetObjectItem(payload, key);
    return item && cJSON_IsString(item) ? item->valuestring : "";
}

double ClearChargingProfile::getNumberField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;
    if (!key) return 0;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    return item && cJSON_IsNumber(item) ? item->valuedouble : 0;
}

bool ClearChargingProfile::hasField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    if (!key) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return false;

    return cJSON_GetObjectItem(payload, key) != nullptr;
}

// ===== Build =====

bool ClearChargingProfile::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("ClearChargingProfile");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    m_id = -1;
    m_connectorId = -1;
    return true;
}

bool ClearChargingProfile::buildConf()
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
    m_status = CLEAR_PROFILE_ACCEPTED;
    return true;
}

// ===== Common Setters =====

bool ClearChargingProfile::setMsgSeq(quint64 i)
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

bool ClearChargingProfile::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("ClearChargingProfile");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool ClearChargingProfile::setId(qint32 id)
{
    if (m_type != OCPP_CALL) return false;
    if (!setNumberField("id", (double)id)) return false;
    m_id = id;
    return true;
}

bool ClearChargingProfile::setConnectorId(qint32 id)
{
    if (m_type != OCPP_CALL) return false;
    if (!setNumberField("connectorId", (double)id)) return false;
    m_connectorId = id;
    return true;
}

bool ClearChargingProfile::setChargingProfilePurpose(quint8 purpose)
{
    if (m_type != OCPP_CALL) return false;
    if (purpose > TX_PROFILE) return false;

    if (!setStringField("chargingProfilePurpose", purposeToString(purpose))) return false;
    m_purpose = purpose;
    return true;
}

// ===== Response Setters =====

bool ClearChargingProfile::setStatus(quint8 s)
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

quint64 ClearChargingProfile::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 ClearChargingProfile::type() const { return m_type; }

qint32 ClearChargingProfile::id() const
{
    if (m_type != OCPP_CALL) return -1;
    return m_id;
}

qint32 ClearChargingProfile::connectorId() const
{
    if (m_type != OCPP_CALL) return -1;
    return m_connectorId;
}

quint8 ClearChargingProfile::chargingProfilePurpose() const
{
    if (m_type != OCPP_CALL) return CHARGE_POINT_MAX_PROFILE;
    return m_purpose;
}

const char* ClearChargingProfile::chargingProfilePurposeString() const
{
    return purposeToString(m_purpose);
}

quint8 ClearChargingProfile::status() const { return m_status; }
const char* ClearChargingProfile::statusString() const { return statusToString(m_status); }

bool ClearChargingProfile::isCall() const { return m_type == OCPP_CALL; }
bool ClearChargingProfile::isCallResult() const { return m_type == OCPP_CALLRESULT; }
bool ClearChargingProfile::isValid() const { return m_root != nullptr; }

// ===== Parse =====

bool ClearChargingProfile::parse(const char *value)
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

bool ClearChargingProfile::parse(cJSON *obj)
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

    quint8 status = CLEAR_PROFILE_UNKNOWN;
    qint32 reqId = -1;
    qint32 connId = -1;
    quint8 purpose = CHARGE_POINT_MAX_PROFILE;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "ClearChargingProfile") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // Optional fields
        cJSON *idJson = cJSON_GetObjectItem(payload, "id");
        if (idJson && cJSON_IsNumber(idJson)) {
            reqId = (qint32)idJson->valueint;
        }

        cJSON *connJson = cJSON_GetObjectItem(payload, "connectorId");
        if (connJson && cJSON_IsNumber(connJson)) {
            connId = (qint32)connJson->valueint;
        }

        cJSON *purposeJson = cJSON_GetObjectItem(payload, "chargingProfilePurpose");
        if (purposeJson && cJSON_IsString(purposeJson)) {
            purpose = stringToPurpose(purposeJson->valuestring);
        }
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
    m_id = reqId;
    m_connectorId = connId;
    m_purpose = purpose;

    return true;
}

// ===== Serialize =====

char* ClearChargingProfile::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void ClearChargingProfile::print() const
{
    char *data = toJson();
    if (data) {
        printf("[ClearChargingProfile] %s\n", data);
        free(data);
    }
}

#if 0
// 解析收到的清除配置请求
ClearChargingProfile req;
if (req.parse(receivedJson)) {
    // 根据筛选条件清除配置文件
    bool cleared = clearChargingProfiles(
        req.id(),
        req.connectorId(),
        req.chargingProfilePurpose()
    );

    // 构建响应
    ClearChargingProfile conf;
    conf.buildConf();
    conf.setMsgSeq(req.msgSeq());

    if (cleared) {
        conf.setStatus(CLEAR_PROFILE_ACCEPTED);
    } else {
        conf.setStatus(CLEAR_PROFILE_UNKNOWN);
    }

    char *response = conf.toJson();
    // 发送响应...
    free(response);
}
#endif

