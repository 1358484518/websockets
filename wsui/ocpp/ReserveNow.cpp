#include "ReserveNow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

ReserveNow::ReserveNow()
    : m_root(nullptr), m_type(0), m_status(RESERVE_REJECTED)
{
}

ReserveNow::ReserveNow(const char *value)
    : m_root(nullptr), m_type(0), m_status(RESERVE_REJECTED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

ReserveNow::ReserveNow(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(RESERVE_REJECTED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

ReserveNow::~ReserveNow()
{
    clear();
}

ReserveNow::ReserveNow(ReserveNow&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = RESERVE_REJECTED;
}

ReserveNow& ReserveNow::operator=(ReserveNow&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = RESERVE_REJECTED;
    }
    return *this;
}

void ReserveNow::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = RESERVE_REJECTED;
}

bool ReserveNow::ensurePayload()
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

bool ReserveNow::setStringField(const char *key, const char *value)
{
    if (!key || !value) return false;
    if (!ensurePayload()) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return false;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(value);
        if (!newItem) return false;
        cJSON_ReplaceItemInObject(payload, key, newItem);
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, key);
        cJSON *newItem = cJSON_CreateString(value);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, key, newItem);
    }

    return true;
}

bool ReserveNow::setNumberField(const char *key, qreal value)
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
    } else {
        if (item) cJSON_DeleteItemFromObject(payload, key);
        cJSON *newItem = cJSON_CreateNumber(value);
        if (!newItem) return false;
        cJSON_AddItemToObject(payload, key, newItem);
    }

    return true;
}

bool ReserveNow::clearField(const char *key)
{
    if (!key) return false;
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return false;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (!item) return true;

    cJSON_DeleteItemFromObject(payload, key);
    return true;
}

const char* ReserveNow::getStringField(const char *key) const
{
    if (!key || !m_root || !cJSON_IsArray(m_root)) return "";

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return "";

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (!item || !cJSON_IsString(item)) return "";

    return item->valuestring;
}

qreal ReserveNow::getNumberField(const char *key) const
{
    if (!key || !m_root || !cJSON_IsArray(m_root)) return 0;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return 0;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (!item || !cJSON_IsNumber(item)) return 0;

    return item->valuedouble;
}

bool ReserveNow::hasField(const char *key) const
{
    if (!key || !m_root || !cJSON_IsArray(m_root)) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload)) return false;

    return cJSON_GetObjectItem(payload, key) != nullptr;
}

const char* ReserveNow::statusToString(quint8 status)
{
    switch (status) {
        case RESERVE_ACCEPTED: return "Accepted";
        case RESERVE_FAULTED: return "Faulted";
        case RESERVE_OCCUPIED: return "Occupied";
        case RESERVE_REJECTED: return "Rejected";
        case RESERVE_UNAVAILABLE: return "Unavailable";
        default: return "Rejected";
    }
}

quint8 ReserveNow::stringToStatus(const char *str)
{
    if (!str) return RESERVE_REJECTED;
    if (strcmp(str, "Accepted") == 0) return RESERVE_ACCEPTED;
    if (strcmp(str, "Faulted") == 0) return RESERVE_FAULTED;
    if (strcmp(str, "Occupied") == 0) return RESERVE_OCCUPIED;
    if (strcmp(str, "Unavailable") == 0) return RESERVE_UNAVAILABLE;
    return RESERVE_REJECTED;
}

bool ReserveNow::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("ReserveNow");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool ReserveNow::buildConf()
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

bool ReserveNow::setMsgSeq(quint64 i)
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

bool ReserveNow::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("ReserveNow");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request setters =====

bool ReserveNow::setConnectorId(qint32 connectorId)
{
    if (m_type != OCPP_CALL) return false;
    return setNumberField("connectorId", (qreal)connectorId);
}

bool ReserveNow::setExpiryDate(const char *expiryDate)
{
    if (m_type != OCPP_CALL) return false;
    if (!expiryDate || *expiryDate == '\0') return false;
    return setStringField("expiryDate", expiryDate);
}

bool ReserveNow::setIdTag(const char *idTag)
{
    if (m_type != OCPP_CALL) return false;
    if (!idTag || *idTag == '\0') return false;
    return setStringField("idTag", idTag);
}

bool ReserveNow::setReservationId(qint32 reservationId)
{
    if (m_type != OCPP_CALL) return false;
    return setNumberField("reservationId", (qreal)reservationId);
}

bool ReserveNow::setParentIdTag(const char *parentIdTag)
{
    if (m_type != OCPP_CALL) return false;
    if (!parentIdTag || *parentIdTag == '\0') return false;
    return setStringField("parentIdTag", parentIdTag);
}

bool ReserveNow::clearParentIdTag()
{
    if (m_type != OCPP_CALL) return false;
    return clearField("parentIdTag");
}

// ===== Response setters =====

bool ReserveNow::setStatus(quint8 status)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!ensurePayload()) return false;

    int payloadIndex = 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
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

quint64 ReserveNow::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 ReserveNow::type() const
{
    return m_type;
}

qint32 ReserveNow::connectorId() const
{
    if (m_type != OCPP_CALL) return 0;
    return (qint32)getNumberField("connectorId");
}

const char* ReserveNow::expiryDate() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("expiryDate");
}

const char* ReserveNow::idTag() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("idTag");
}

qint32 ReserveNow::reservationId() const
{
    if (m_type != OCPP_CALL) return 0;
    return (qint32)getNumberField("reservationId");
}

const char* ReserveNow::parentIdTag() const
{
    if (m_type != OCPP_CALL) return "";
    return getStringField("parentIdTag");
}

bool ReserveNow::hasParentIdTag() const
{
    if (m_type != OCPP_CALL) return false;
    return hasField("parentIdTag");
}

quint8 ReserveNow::status() const
{
    return m_status;
}

const char* ReserveNow::statusString() const
{
    return statusToString(m_status);
}

bool ReserveNow::isCall() const
{
    return m_type == OCPP_CALL;
}

bool ReserveNow::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool ReserveNow::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool ReserveNow::parse(const char *value)
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

bool ReserveNow::parse(cJSON *obj)
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

    quint8 parsedStatus = RESERVE_REJECTED;

    if (msgType == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "ReserveNow") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;

        // Required fields: connectorId, expiryDate, idTag, reservationId
        cJSON *connItem = cJSON_GetObjectItem(payload, "connectorId");
        if (!connItem || !cJSON_IsNumber(connItem)) return false;

        cJSON *expiryItem = cJSON_GetObjectItem(payload, "expiryDate");
        if (!expiryItem || !cJSON_IsString(expiryItem)) return false;

        cJSON *idTagItem = cJSON_GetObjectItem(payload, "idTag");
        if (!idTagItem || !cJSON_IsString(idTagItem)) return false;

        cJSON *resItem = cJSON_GetObjectItem(payload, "reservationId");
        if (!resItem || !cJSON_IsNumber(resItem)) return false;
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

char* ReserveNow::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void ReserveNow::print() const
{
    char *data = toJson();
    if (data) {
        printf("[ReserveNow] %s\n", data);
        free(data);
    }
}
#if 0
#include <stdio.h>
#include <stdlib.h>
#include "reservenow.h"

int main()
{
    // ===== 示例1：构建 ReserveNow 请求（充电桩接收服务器预约请求） =====
    printf("=== Example 1: Build ReserveNow Request ===\n");
    ReserveNow req;
    req.buildReq();
    req.setMsgSeq(1001);
    req.setConnectorId(1);
    req.setExpiryDate("2024-12-31T23:59:59Z");
    req.setIdTag("USER_12345");
    req.setReservationId(50001);
    req.setParentIdTag("PARENT_GROUP_A");
    req.print();

    // 获取字段
    printf("connectorId: %d\n", req.connectorId());
    printf("expiryDate: %s\n", req.expiryDate());
    printf("idTag: %s\n", req.idTag());
    printf("reservationId: %d\n", req.reservationId());
    printf("hasParentIdTag: %s\n", req.hasParentIdTag() ? "yes" : "no");
    printf("parentIdTag: %s\n", req.parentIdTag());

    // 清除可选字段
    req.clearParentIdTag();
    printf("\nAfter clearParentIdTag:\n");
    printf("hasParentIdTag: %s\n", req.hasParentIdTag() ? "yes" : "no");
    req.print();

    // ===== 示例2：构建 ReserveNow 响应（充电桩回复预约结果） =====
    printf("\n=== Example 2: Build ReserveNow Response ===\n");
    ReserveNow conf;
    conf.buildConf();
    conf.setMsgSeq(1001);
    conf.setStatus(RESERVE_ACCEPTED);
    conf.print();

    printf("status: %d (%s)\n", conf.status(), conf.statusString());

    // 测试所有状态值
    printf("\nAll status values:\n");
    ReserveNow statusTest;
    statusTest.buildConf();

    statusTest.setStatus(RESERVE_ACCEPTED);
    printf("  ACCEPTED: %s\n", statusTest.statusString());

    statusTest.setStatus(RESERVE_FAULTED);
    printf("  FAULTED: %s\n", statusTest.statusString());

    statusTest.setStatus(RESERVE_OCCUPIED);
    printf("  OCCUPIED: %s\n", statusTest.statusString());

    statusTest.setStatus(RESERVE_REJECTED);
    printf("  REJECTED: %s\n", statusTest.statusString());

    statusTest.setStatus(RESERVE_UNAVAILABLE);
    printf("  UNAVAILABLE: %s\n", statusTest.statusString());

    // ===== 示例3：解析 JSON 字符串 =====
    printf("\n=== Example 3: Parse JSON String ===\n");
    const char *jsonReq = "[2,\"2001\",\"ReserveNow\",{"
        "\"connectorId\":2,"
        "\"expiryDate\":\"2024-06-30T12:00:00Z\","
        "\"idTag\":\"TAG_999\","
        "\"reservationId\":99999}]";

    ReserveNow parsedReq;
    if (parsedReq.parse(jsonReq)) {
        printf("Parse success!\n");
        printf("  msgSeq: %llu\n", (unsigned long long)parsedReq.msgSeq());
        printf("  isCall: %s\n", parsedReq.isCall() ? "yes" : "no");
        printf("  connectorId: %d\n", parsedReq.connectorId());
        printf("  expiryDate: %s\n", parsedReq.expiryDate());
        printf("  idTag: %s\n", parsedReq.idTag());
        printf("  reservationId: %d\n", parsedReq.reservationId());
    } else {
        printf("Parse failed!\n");
    }

    // 解析响应
    const char *jsonConf = "[3,\"2001\",{\"status\":\"Occupied\"}]";
    ReserveNow parsedConf;
    if (parsedConf.parse(jsonConf)) {
        printf("\nParse response success!\n");
        printf("  status: %s\n", parsedConf.statusString());
    }

    // ===== 示例4：序列化与反序列化（Round-trip） =====
    printf("\n=== Example 4: Round-trip Serialization ===\n");
    ReserveNow original;
    original.buildReq();
    original.setMsgSeq(42);
    original.setConnectorId(3);
    original.setExpiryDate("2025-01-15T08:30:00Z");
    original.setIdTag("ROUND_TRIP_TEST");
    original.setReservationId(77777);
    original.setParentIdTag("PARENT_777");

    char *jsonStr = original.toJson();
    printf("Original: %s\n", jsonStr);

    ReserveNow restored;
    if (restored.parse(jsonStr)) {
        printf("Restored successfully!\n");
        printf("  msgSeq match: %s\n", (original.msgSeq() == restored.msgSeq()) ? "yes" : "no");
        printf("  connectorId match: %s\n", (original.connectorId() == restored.connectorId()) ? "yes" : "no");
        printf("  idTag match: %s\n", (strcmp(original.idTag(), restored.idTag()) == 0) ? "yes" : "no");
        printf("  reservationId match: %s\n", (original.reservationId() == restored.reservationId()) ? "yes" : "no");
        printf("  parentIdTag match: %s\n", (strcmp(original.parentIdTag(), restored.parentIdTag()) == 0) ? "yes" : "no");
    }

    free(jsonStr);

    // ===== 示例5：切换消息类型（CALL ↔ CALLRESULT） =====
    printf("\n=== Example 5: Switch Message Type ===\n");
    ReserveNow msg;
    msg.buildReq();
    msg.setConnectorId(1);
    msg.setIdTag("test");
    printf("As CALL (size=%d): ", cJSON_GetArraySize(msg.root()));
    msg.print();

    msg.setType(OCPP_CALLRESULT);
    msg.setStatus(RESERVE_ACCEPTED);
    printf("As CALLRESULT (size=%d): ", cJSON_GetArraySize(msg.root()));
    msg.print();

    msg.setType(OCPP_CALL);
    printf("Back to CALL (size=%d): ", cJSON_GetArraySize(msg.root()));
    msg.print();

    // ===== 示例6：错误解析测试 =====
    printf("\n=== Example 6: Invalid Parse Cases ===\n");
    ReserveNow invalid;

    // 缺少必填字段
    printf("Missing required fields: %s\n",
        invalid.parse("[2,\"1\",\"ReserveNow\",{\"connectorId\":1}]") ? "parsed (wrong!)" : "rejected (correct)");

    // 错误的 action 名称
    printf("Wrong action name: %s\n",
        invalid.parse("[2,\"1\",\"WrongAction\",{\"connectorId\":1}]") ? "parsed (wrong!)" : "rejected (correct)");

    // 空字符串
    printf("Empty string: %s\n",
        invalid.parse("") ? "parsed (wrong!)" : "rejected (correct)");

    // 无效 JSON
    printf("Invalid JSON: %s\n",
        invalid.parse("not json at all") ? "parsed (wrong!)" : "rejected (correct)");

    printf("\nAll examples completed!\n");
    return 0;
}
=== Example 1: Build ReserveNow Request ===
[ReserveNow] [2,"1001","ReserveNow",{"connectorId":1,"expiryDate":"2024-12-31T23:59:59Z","idTag":"USER_12345","reservationId":50001,"parentIdTag":"PARENT_GROUP_A"}]
connectorId: 1
expiryDate: 2024-12-31T23:59:59Z
idTag: USER_12345
reservationId: 50001
hasParentIdTag: yes
parentIdTag: PARENT_GROUP_A

After clearParentIdTag:
hasParentIdTag: no
[ReserveNow] [2,"1001","ReserveNow",{"connectorId":1,"expiryDate":"2024-12-31T23:59:59Z","idTag":"USER_12345","reservationId":50001}]

=== Example 2: Build ReserveNow Response ===
[ReserveNow] [3,"1001",{"status":"Accepted"}]
status: 0 (Accepted)

All status values:
  ACCEPTED: Accepted
  FAULTED: Faulted
  OCCUPIED: Occupied
  REJECTED: Rejected
  UNAVAILABLE: Unavailable

=== Example 3: Parse JSON String ===
Parse success!
  msgSeq: 2001
  isCall: yes
  connectorId: 2
  expiryDate: 2024-06-30T12:00:00Z
  idTag: TAG_999
  reservationId: 99999

Parse response success!
  status: Occupied

=== Example 4: Round-trip Serialization ===
Original: [2,"42","ReserveNow",{"connectorId":3,"expiryDate":"2025-01-15T08:30:00Z","idTag":"ROUND_TRIP_TEST","reservationId":77777,"parentIdTag":"PARENT_777"}]
Restored successfully!
  msgSeq match: yes
  connectorId match: yes
  idTag match: yes
  reservationId match: yes
  parentIdTag match: yes

=== Example 5: Switch Message Type ===
As CALL (size=4): [ReserveNow] [2,"0","ReserveNow",{"connectorId":1,"idTag":"test"}]
As CALLRESULT (size=3): [ReserveNow] [3,"0",{"status":"Accepted"}]
Back to CALL (size=4): [ReserveNow] [2,"0","ReserveNow",{}]

=== Example 6: Invalid Parse Cases ===
Missing required fields: rejected (correct)
Wrong action name: rejected (correct)
Empty string: rejected (correct)
Invalid JSON: rejected (correct)

All examples completed!
#endif
