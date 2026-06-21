#include "CancelReservation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== Status Conversion =====

const char* CancelReservation::statusToString(quint8 s) const
{
    switch (s) {
        case CANCEL_ACCEPTED: return "Accepted";
        case CANCEL_REJECTED: return "Rejected";
        default: return "Rejected";
    }
}

quint8 CancelReservation::stringToStatus(const char *str) const
{
    if (!str) return CANCEL_REJECTED;
    if (strcmp(str, "Accepted") == 0) return CANCEL_ACCEPTED;
    if (strcmp(str, "Rejected") == 0) return CANCEL_REJECTED;
    return CANCEL_REJECTED;
}

// ===== Construction / Destruction =====

CancelReservation::CancelReservation()
    : m_root(nullptr), m_type(0), m_status(CANCEL_REJECTED), m_reservationId(0) {}

CancelReservation::CancelReservation(const char *value)
    : m_root(nullptr), m_type(0), m_status(CANCEL_REJECTED), m_reservationId(0)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

CancelReservation::CancelReservation(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(CANCEL_REJECTED), m_reservationId(0)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

CancelReservation::~CancelReservation()
{
    clear();
}

CancelReservation::CancelReservation(CancelReservation&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status), m_reservationId(other.m_reservationId)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = CANCEL_REJECTED;
    other.m_reservationId = 0;
}

CancelReservation& CancelReservation::operator=(CancelReservation&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        m_reservationId = other.m_reservationId;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = CANCEL_REJECTED;
        other.m_reservationId = 0;
    }
    return *this;
}

void CancelReservation::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = CANCEL_REJECTED;
    m_reservationId = 0;
}

// ===== Internal Helpers =====

bool CancelReservation::ensurePayload()
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

bool CancelReservation::setStringField(const char *key, const char *value)
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

bool CancelReservation::setNumberField(const char *key, double value)
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

const char* CancelReservation::getStringField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return "";
    if (!key) return "";

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return "";

    cJSON *item = cJSON_GetObjectItem(payload, key);
    return item && cJSON_IsString(item) ? item->valuestring : "";
}

double CancelReservation::getNumberField(const char *key) const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;
    if (!key) return 0;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    return item && cJSON_IsNumber(item) ? item->valuedouble : 0;
}

// ===== Build =====

bool CancelReservation::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("CancelReservation");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }

    cJSON *resIdItem = cJSON_CreateNumber(0);
    if (!resIdItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "reservationId", resIdItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    m_reservationId = 0;
    return true;
}

bool CancelReservation::buildConf()
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
    m_status = CANCEL_ACCEPTED;
    return true;
}

// ===== Common Setters =====

bool CancelReservation::setMsgSeq(quint64 i)
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

bool CancelReservation::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("CancelReservation");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool CancelReservation::setReservationId(qint32 id)
{
    if (m_type != OCPP_CALL) return false;
    if (!setNumberField("reservationId", (double)id)) return false;
    m_reservationId = id;
    return true;
}

// ===== Response Setters =====

bool CancelReservation::setStatus(quint8 s)
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

quint64 CancelReservation::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 CancelReservation::type() const
{
    return m_type;
}

qint32 CancelReservation::reservationId() const
{
    if (m_type != OCPP_CALL) return 0;
    return m_reservationId;
}

quint8 CancelReservation::status() const
{
    return m_status;
}

const char* CancelReservation::statusString() const
{
    return statusToString(m_status);
}

bool CancelReservation::isCall() const
{
    return m_type == OCPP_CALL;
}

bool CancelReservation::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool CancelReservation::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool CancelReservation::parse(const char *value)
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

bool CancelReservation::parse(cJSON *obj)
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

    quint8 status = CANCEL_REJECTED;
    qint32 reservationId = 0;

    if (type == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "CancelReservation") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // Required field
        cJSON *resId = cJSON_GetObjectItem(payload, "reservationId");
        if (!resId || !cJSON_IsNumber(resId)) return false;
        reservationId = (qint32)resId->valueint;
    }

    if (type == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;

        cJSON *statusItem = cJSON_GetObjectItem(payload, "status");
        if (statusItem && cJSON_IsString(statusItem)) {
            status = stringToStatus(statusItem->valuestring);
        }
    }

    clear();
    m_root = obj;
    m_type = type;
    m_status = status;
    m_reservationId = reservationId;

    return true;
}

// ===== Serialize =====

char* CancelReservation::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void CancelReservation::print() const
{
    char *data = toJson();
    if (data) {
        printf("[CancelReservation] %s\n", data);
        free(data);
    }
}




#if 0
// 解析收到的取消预约请求
CancelReservation req;
if (req.parse(receivedJson)) {
    qint32 resId = req.reservationId();
    printf("Cancel reservation: %d\n", resId);

    // 构建响应
    CancelReservation conf;
    conf.buildConf();
    conf.setMsgSeq(req.msgSeq());

    if (cancelSuccess) {
        conf.setStatus(CANCEL_ACCEPTED);
    } else {
        conf.setStatus(CANCEL_REJECTED);
    }

    char *response = conf.toJson();
    // 发送响应...
    free(response);
}
#endif
