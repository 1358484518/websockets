#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <utility>
#include "UnlockConnector.h"

// ===== Constructors & Destructor =====

UnlockConnector::UnlockConnector()
    : m_root(nullptr), m_type(0), m_status(UNLOCK_UNLOCKED)
{
}

UnlockConnector::UnlockConnector(const char *value)
    : m_root(nullptr), m_type(0), m_status(UNLOCK_UNLOCKED)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

UnlockConnector::UnlockConnector(cJSON *obj)
    : m_root(nullptr), m_type(0), m_status(UNLOCK_UNLOCKED)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

UnlockConnector::~UnlockConnector()
{
    clear();
}

// ===== Move Semantics =====

UnlockConnector::UnlockConnector(UnlockConnector&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = UNLOCK_UNLOCKED;
}

UnlockConnector& UnlockConnector::operator=(UnlockConnector&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = UNLOCK_UNLOCKED;
    }
    return *this;
}

// ===== Clear =====

void UnlockConnector::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = UNLOCK_UNLOCKED;
}

// ===== Internal Helpers =====

bool UnlockConnector::ensurePayload()
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

// ===== Enum Conversions =====

const char* UnlockConnector::statusToString(quint8 status)
{
    switch (status) {
        case UNLOCK_UNLOCKED:       return "Unlocked";
        case UNLOCK_UNLOCK_FAILED:  return "UnlockFailed";
        case UNLOCK_NOT_SUPPORTED:  return "NotSupported";
        default:                    return "Unlocked";
    }
}

quint8 UnlockConnector::stringToStatus(const char *str)
{
    if (!str)
        return UNLOCK_UNLOCKED;
    if (strcmp(str, "Unlocked") == 0)
        return UNLOCK_UNLOCKED;
    if (strcmp(str, "UnlockFailed") == 0)
        return UNLOCK_UNLOCK_FAILED;
    if (strcmp(str, "NotSupported") == 0)
        return UNLOCK_NOT_SUPPORTED;
    return UNLOCK_UNLOCKED;
}

// ===== Build =====

bool UnlockConnector::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("UnlockConnector");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool UnlockConnector::buildConf()
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

// ===== Common Setters =====

bool UnlockConnector::setMsgSeq(quint64 i)
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

bool UnlockConnector::setMsgSeq(QString i)
{
    if (!m_root || !cJSON_IsArray(m_root))
        return false;

    cJSON *newItem = cJSON_CreateString(i.toUtf8().constData());
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

bool UnlockConnector::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("UnlockConnector");
        if (!actionItem)
            return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request Setters =====

bool UnlockConnector::setConnectorId(qint32 connectorId)
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

// ===== Response Setters =====

bool UnlockConnector::setStatus(quint8 status)
{
    if (m_type != OCPP_CALLRESULT)
        return false;
    if (!ensurePayload())
        return false;

    const char *statusStr = statusToString(status);
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, "status");
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(payload, "status", newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, "status");
        cJSON *newItem = cJSON_CreateString(statusStr);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, "status", newItem);
    }

    m_status = status;
    return true;
}

// ===== Getters =====

quint64 UnlockConnector::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem))
        return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 UnlockConnector::type() const
{
    return m_type;
}

qint32 UnlockConnector::connectorId() const
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

quint8 UnlockConnector::status() const
{
    return m_status;
}

const char* UnlockConnector::statusString() const
{
    return statusToString(m_status);
}

bool UnlockConnector::isCall() const
{
    return m_type == OCPP_CALL;
}

bool UnlockConnector::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool UnlockConnector::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool UnlockConnector::parse(const char *value)
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

bool UnlockConnector::parse(cJSON *obj)
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

    quint8 parsedStatus = UNLOCK_UNLOCKED;

    if (msgType == OCPP_CALL) {
        if (size < 4)
            return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem))
            return false;
        if (strcmp(actionItem->valuestring, "UnlockConnector") != 0)
            return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Required: connectorId
        cJSON *connItem = cJSON_GetObjectItem(payload, "connectorId");
        if (!connItem || !cJSON_IsNumber(connItem))
            return false;
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Required: status
        cJSON *statusItem = cJSON_GetObjectItem(payload, "status");
        if (!statusItem || !cJSON_IsString(statusItem))
            return false;

        parsedStatus = stringToStatus(statusItem->valuestring);
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_status = parsedStatus;

    return true;
}

// ===== Serialize =====

char* UnlockConnector::toJson() const
{
    if (!m_root)
        return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void UnlockConnector::print() const
{
    char *data = toJson();
    if (data) {
        printf("[UnlockConnector] %s\n", data);
        free(data);
    }
}
#if 0
#include <stdio.h>
#include <stdlib.h>
#include "unlockconnector.h"

// ===== Example 1: Server sends unlock request =====
void example_server_send_request()
{
    printf("=== Example 1: Server sends unlock request ===\n");

    UnlockConnector req;
    req.buildReq();
    req.setMsgSeq(1001);
    req.setConnectorId(1);

    char *json = req.toJson();
    printf("Send: %s\n", json);
    free(json);

    printf("  connectorId: %d\n", req.connectorId());
    printf("\n");
}

// ===== Example 2: Charge point responds Unlocked =====
void example_cp_respond_unlocked()
{
    printf("=== Example 2: Charge point responds Unlocked ===\n");

    UnlockConnector conf;
    conf.buildConf();
    conf.setMsgSeq(1001);
    conf.setStatus(UNLOCK_UNLOCKED);

    char *json = conf.toJson();
    printf("Response: %s\n", json);
    free(json);

    printf("  status: %s\n", conf.statusString());
    printf("\n");
}

// ===== Example 3: Charge point responds UnlockFailed =====
void example_cp_respond_failed()
{
    printf("=== Example 3: Charge point responds UnlockFailed ===\n");

    UnlockConnector conf;
    conf.buildConf();
    conf.setMsgSeq(1002);
    conf.setStatus(UNLOCK_UNLOCK_FAILED);

    char *json = conf.toJson();
    printf("Response: %s\n", json);
    free(json);
    printf("\n");
}

// ===== Example 4: Charge point responds NotSupported =====
void example_cp_respond_not_supported()
{
    printf("=== Example 4: Charge point responds NotSupported ===\n");

    UnlockConnector conf;
    conf.buildConf();
    conf.setMsgSeq(1003);
    conf.setStatus(UNLOCK_NOT_SUPPORTED);

    char *json = conf.toJson();
    printf("Response: %s\n", json);
    free(json);
    printf("\n");
}

// ===== Example 5: Parse incoming unlock request =====
void example_parse_request()
{
    printf("=== Example 5: Parse incoming unlock request ===\n");

    const char *incoming = "[2,\"9999\",\"UnlockConnector\",{\"connectorId\":2}]";
    printf("Incoming: %s\n", incoming);

    UnlockConnector msg;
    if (msg.parse(incoming)) {
        printf("Parse OK\n");
        printf("  type: %s\n", msg.isCall() ? "CALL (request)" : "other");
        printf("  msgId: %llu\n", (unsigned long long)msg.msgSeq());
        printf("  connectorId: %d\n", msg.connectorId());

        // Process the unlock request...
        if (msg.connectorId() == 2) {
            printf("  -> Unlocking connector 2...\n");
        }
    } else {
        printf("Parse FAILED\n");
    }
    printf("\n");
}

// ===== Example 6: Parse incoming response =====
void example_parse_response()
{
    printf("=== Example 6: Parse incoming response ===\n");

    const char *incoming = "[3,\"9999\",{\"status\":\"Unlocked\"}]";
    printf("Incoming: %s\n", incoming);

    UnlockConnector msg;
    if (msg.parse(incoming)) {
        printf("Parse OK\n");
        printf("  type: %s\n", msg.isCallResult() ? "CALLRESULT (response)" : "other");
        printf("  msgId: %llu\n", (unsigned long long)msg.msgSeq());
        printf("  status: %s\n", msg.statusString());

        // Check status
        switch (msg.status()) {
            case UNLOCK_UNLOCKED:
                printf("  -> Unlock successful!\n");
                break;
            case UNLOCK_UNLOCK_FAILED:
                printf("  -> Unlock failed, check connector lock\n");
                break;
            case UNLOCK_NOT_SUPPORTED:
                printf("  -> Remote unlock not supported\n");
                break;
        }
    } else {
        printf("Parse FAILED\n");
    }
    printf("\n");
}

// ===== Example 7: Full workflow - request then response =====
void example_full_workflow()
{
    printf("=== Example 7: Full workflow ===\n");

    // Step 1: Server builds and sends request
    UnlockConnector req;
    req.buildReq();
    req.setMsgSeq(5001);
    req.setConnectorId(1);

    char *reqJson = req.toJson();
    printf("[Server] Send: %s\n", reqJson);

    // Step 2: Charge point receives and parses request
    UnlockConnector received;
    if (received.parse(reqJson)) {
        printf("[CP] Received unlock request for connector %d\n", received.connectorId());

        // Step 3: Charge point builds response
        UnlockConnector resp;
        resp.buildConf();
        resp.setMsgSeq(received.msgSeq());  // Echo back the message ID

        // Simulate unlock result
        bool unlockSuccess = true;
        if (unlockSuccess) {
            resp.setStatus(UNLOCK_UNLOCKED);
        } else {
            resp.setStatus(UNLOCK_UNLOCK_FAILED);
        }

        char *respJson = resp.toJson();
        printf("[CP] Respond: %s\n", respJson);

        // Step 4: Server receives and parses response
        UnlockConnector serverResp;
        if (serverResp.parse(respJson)) {
            printf("[Server] Response status: %s\n", serverResp.statusString());
        }

        free(respJson);
    }

    free(reqJson);
    printf("\n");
}

int main()
{
    example_server_send_request();
    example_cp_respond_unlocked();
    example_cp_respond_failed();
    example_cp_respond_not_supported();
    example_parse_request();
    example_parse_response();
    example_full_workflow();

    printf("All examples completed.\n");
    return 0;
}
=== Example 1: Server sends unlock request ===
Send: [2,"1001","UnlockConnector",{"connectorId":1}]
  connectorId: 1

=== Example 2: Charge point responds Unlocked ===
Response: [3,"1001",{"status":"Unlocked"}]
  status: Unlocked

=== Example 3: Charge point responds UnlockFailed ===
Response: [3,"1002",{"status":"UnlockFailed"}]

=== Example 4: Charge point responds NotSupported ===
Response: [3,"1003",{"status":"NotSupported"}]

=== Example 5: Parse incoming unlock request ===
Incoming: [2,"9999","UnlockConnector",{"connectorId":2}]
Parse OK
  type: CALL (request)
  msgId: 9999
  connectorId: 2
  -> Unlocking connector 2...

=== Example 6: Parse incoming response ===
Incoming: [3,"9999",{"status":"Unlocked"}]
Parse OK
  type: CALLRESULT (response)
  msgId: 9999
  status: Unlocked
  -> Unlock successful!

=== Example 7: Full workflow ===
[Server] Send: [2,"5001","UnlockConnector",{"connectorId":1}]
[CP] Received unlock request for connector 1
[CP] Respond: [3,"5001",{"status":"Unlocked"}]
[Server] Response status: Unlocked


状态	枚举值	含义
UNLOCK_UNLOCKED	0	解锁成功
UNLOCK_UNLOCK_FAILED	1	解锁失败（如机械锁故障）
UNLOCK_NOT_SUPPORTED	2	不支持远程解锁

#endif
