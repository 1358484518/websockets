#include "reset.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
Reset::Reset()
    : m_root(nullptr), m_type(0), m_resetType(RESET_TYPE_HARD), m_status(RESET_STATUS_REJECTED)
{
}
Reset::Reset(const char *value)
    : m_root(nullptr), m_type(0), m_resetType(RESET_TYPE_HARD), m_status(RESET_STATUS_REJECTED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}
Reset::Reset(cJSON *obj)
    : m_root(nullptr), m_type(0), m_resetType(RESET_TYPE_HARD), m_status(RESET_STATUS_REJECTED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}
Reset::~Reset()
{
    clear();
}
Reset::Reset(Reset&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_resetType(other.m_resetType), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_resetType = RESET_TYPE_HARD;
    other.m_status = RESET_STATUS_REJECTED;
}
Reset& Reset::operator=(Reset&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_resetType = other.m_resetType;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_resetType = RESET_TYPE_HARD;
        other.m_status = RESET_STATUS_REJECTED;
    }
    return *this;
}
void Reset::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_resetType = RESET_TYPE_HARD;
    m_status = RESET_STATUS_REJECTED;
}
bool Reset::ensurePayload()
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
const char* Reset::resetTypeToString(quint8 resetType)
{
    switch (resetType) {
        case RESET_TYPE_HARD: return "Hard";
        case RESET_TYPE_SOFT: return "Soft";
        default: return "Hard";
    }
}
quint8 Reset::stringToResetType(const char *str)
{
    if (!str) return RESET_TYPE_HARD;
    if (strcmp(str, "Hard") == 0) return RESET_TYPE_HARD;
    if (strcmp(str, "Soft") == 0) return RESET_TYPE_SOFT;
    return RESET_TYPE_HARD;
}
const char* Reset::statusToString(quint8 status)
{
    switch (status) {
        case RESET_STATUS_ACCEPTED: return "Accepted";
        case RESET_STATUS_REJECTED: return "Rejected";
        default: return "Rejected";
    }
}
quint8 Reset::stringToStatus(const char *str)
{
    if (!str) return RESET_STATUS_REJECTED;
    if (strcmp(str, "Accepted") == 0) return RESET_STATUS_ACCEPTED;
    return RESET_STATUS_REJECTED;
}
bool Reset::buildReq()
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
    cJSON *actionItem = cJSON_CreateString("Reset");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);
    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);
    m_type = OCPP_CALL;
    return true;
}
bool Reset::buildConf()
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
bool Reset::setMsgSeq(quint64 i)
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
bool Reset::setMsgSeq(QString i)
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
bool Reset::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("Reset");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }
    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;
    return true;
}
// ===== Request setters =====
bool Reset::setResetType(quint8 resetType)
{
    if (m_type != OCPP_CALL) return false;
    if (!ensurePayload()) return false;
    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;
    const char *typeStr = resetTypeToString(resetType);
    cJSON *item = cJSON_GetObjectItem(payload, "type");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(typeStr);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(payload, "type", newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, "type");
        cJSON *newItem = cJSON_CreateString(typeStr);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, "type", newItem);
    }
    m_resetType = resetType;
    return true;
}
// ===== Response setters =====
bool Reset::setStatus(quint8 status)
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
quint64 Reset::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;
    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;
    return strtoull(idItem->valuestring, nullptr, 10);
}
quint8 Reset::type() const
{
    return m_type;
}
quint8 Reset::resetType() const
{
    return m_resetType;
}
const char* Reset::resetTypeString() const
{
    return resetTypeToString(m_resetType);
}
quint8 Reset::status() const
{
    return m_status;
}
const char* Reset::statusString() const
{
    return statusToString(m_status);
}
bool Reset::isCall() const
{
    return m_type == OCPP_CALL;
}
bool Reset::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}
bool Reset::isValid() const
{
    return m_root != nullptr;
}
// ===== Parse =====
bool Reset::parse(const char *value)
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
bool Reset::parse(cJSON *obj)
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
    quint8 parsedResetType = RESET_TYPE_HARD;
    quint8 parsedStatus = RESET_STATUS_REJECTED;
    if (msgType == OCPP_CALL) {
        if (size < 4) return false;
        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "Reset") != 0) return false;
        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;
        // type is required
        cJSON *typePayloadItem = cJSON_GetObjectItem(payload, "type");
        if (!typePayloadItem || !cJSON_IsString(typePayloadItem)) return false;
        parsedResetType = stringToResetType(typePayloadItem->valuestring);
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
    m_resetType = parsedResetType;
    m_status = parsedStatus;
    return true;
}
// ===== Serialize =====
char* Reset::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}
void Reset::print() const
{
    char *data = toJson();
    if (data) {
        printf("[Reset] %s\n", data);
        free(data);
    }
}
