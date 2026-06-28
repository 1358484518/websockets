#include "UpdateFirmware.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <utility>


// ===== Constructors & Destructor =====

UpdateFirmware::UpdateFirmware()
    : m_root(nullptr), m_type(0)
{
}

UpdateFirmware::UpdateFirmware(const char *value)
    : m_root(nullptr), m_type(0)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

UpdateFirmware::UpdateFirmware(cJSON *obj)
    : m_root(nullptr), m_type(0)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

UpdateFirmware::~UpdateFirmware()
{
    clear();
}

// ===== Move Semantics =====

UpdateFirmware::UpdateFirmware(UpdateFirmware&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type)
{
    other.m_root = nullptr;
    other.m_type = 0;
}

UpdateFirmware& UpdateFirmware::operator=(UpdateFirmware&& other) noexcept
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

// ===== Clear =====

void UpdateFirmware::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
}

// ===== Internal Helpers =====

bool UpdateFirmware::ensurePayload()
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

bool UpdateFirmware::setStringField(const char *key, const char *value)
{
    if (!key)
        return false;
    if (!ensurePayload())
        return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (item && cJSON_IsString(item)) {
        cJSON *newItem = cJSON_CreateString(value);
        if (!newItem)
            return false;
        cJSON_ReplaceItemInObject(payload, key, newItem);
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, key);
        cJSON *newItem = cJSON_CreateString(value);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, key, newItem);
    }

    return true;
}

bool UpdateFirmware::setNumberField(const char *key, qreal value)
{
    if (!key)
        return false;
    if (!ensurePayload())
        return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload)
        return false;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (item && cJSON_IsNumber(item)) {
        item->valuedouble = value;
        item->valueint = (int)value;
    } else {
        if (item)
            cJSON_DeleteItemFromObject(payload, key);
        cJSON *newItem = cJSON_CreateNumber(value);
        if (!newItem)
            return false;
        cJSON_AddItemToObject(payload, key, newItem);
    }

    return true;
}

void UpdateFirmware::clearField(const char *key)
{
    if (!key || !m_root || !cJSON_IsArray(m_root))
        return;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload))
        return;

    cJSON_DeleteItemFromObject(payload, key);
}

const char* UpdateFirmware::getStringField(const char *key) const
{
    if (!key || !m_root || !cJSON_IsArray(m_root))
        return nullptr;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload))
        return nullptr;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (!item || !cJSON_IsString(item))
        return nullptr;

    return item->valuestring;
}

qreal UpdateFirmware::getNumberField(const char *key) const
{
    if (!key || !m_root || !cJSON_IsArray(m_root))
        return 0;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload))
        return 0;

    cJSON *item = cJSON_GetObjectItem(payload, key);
    if (!item || !cJSON_IsNumber(item))
        return 0;

    return item->valuedouble;
}

bool UpdateFirmware::hasField(const char *key) const
{
    if (!key || !m_root || !cJSON_IsArray(m_root))
        return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload || !cJSON_IsObject(payload))
        return false;

    return cJSON_GetObjectItem(payload, key) != nullptr;
}

// ===== Build =====

bool UpdateFirmware::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("UpdateFirmware");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool UpdateFirmware::buildConf()
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

bool UpdateFirmware::setMsgSeq(quint64 i)
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

bool UpdateFirmware::setMsgSeq(QString i)
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

bool UpdateFirmware::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("UpdateFirmware");
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

bool UpdateFirmware::setLocation(const char *location)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!location || *location == '\0')
        return false;

    return setStringField("location", location);
}

bool UpdateFirmware::setRetries(qint32 retries)
{
    if (m_type != OCPP_CALL)
        return false;

    return setNumberField("retries", (qreal)retries);
}

void UpdateFirmware::clearRetries()
{
    clearField("retries");
}

bool UpdateFirmware::setRetryInterval(qint32 interval)
{
    if (m_type != OCPP_CALL)
        return false;

    return setNumberField("retryInterval", (qreal)interval);
}

void UpdateFirmware::clearRetryInterval()
{
    clearField("retryInterval");
}

bool UpdateFirmware::setRetrieveDate(const char *date)
{
    if (m_type != OCPP_CALL)
        return false;
    if (!date || *date == '\0')
        return false;

    return setStringField("retrieveDate", date);
}

// ===== Getters =====

quint64 UpdateFirmware::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root))
        return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem))
        return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 UpdateFirmware::type() const
{
    return m_type;
}

const char* UpdateFirmware::location() const
{
    if (m_type != OCPP_CALL)
        return nullptr;
    return getStringField("location");
}

bool UpdateFirmware::hasRetries() const
{
    if (m_type != OCPP_CALL)
        return false;
    return hasField("retries");
}

qint32 UpdateFirmware::retries() const
{
    if (m_type != OCPP_CALL)
        return 0;
    return (qint32)getNumberField("retries");
}

bool UpdateFirmware::hasRetryInterval() const
{
    if (m_type != OCPP_CALL)
        return false;
    return hasField("retryInterval");
}

qint32 UpdateFirmware::retryInterval() const
{
    if (m_type != OCPP_CALL)
        return 0;
    return (qint32)getNumberField("retryInterval");
}

const char* UpdateFirmware::retrieveDate() const
{
    if (m_type != OCPP_CALL)
        return nullptr;
    return getStringField("retrieveDate");
}

bool UpdateFirmware::isCall() const
{
    return m_type == OCPP_CALL;
}

bool UpdateFirmware::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool UpdateFirmware::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool UpdateFirmware::parse(const char *value)
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

bool UpdateFirmware::parse(cJSON *obj)
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

    if (msgType == OCPP_CALL) {
        if (size < 4)
            return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem))
            return false;
        if (strcmp(actionItem->valuestring, "UpdateFirmware") != 0)
            return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload))
            return false;

        // Required: location
        cJSON *locItem = cJSON_GetObjectItem(payload, "location");
        if (!locItem || !cJSON_IsString(locItem))
            return false;

        // Required: retrieveDate
        cJSON *dateItem = cJSON_GetObjectItem(payload, "retrieveDate");
        if (!dateItem || !cJSON_IsString(dateItem))
            return false;

        // Optional: retries (must be number if present)
        cJSON *retriesItem = cJSON_GetObjectItem(payload, "retries");
        if (retriesItem && !cJSON_IsNumber(retriesItem))
            return false;

        // Optional: retryInterval (must be number if present)
        cJSON *intervalItem = cJSON_GetObjectItem(payload, "retryInterval");
        if (intervalItem && !cJSON_IsNumber(intervalItem))
            return false;
    }

    if (msgType == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload))
            return false;
    }

    clear();
    m_root = obj;
    m_type = msgType;

    return true;
}

// ===== Serialize =====

char* UpdateFirmware::toJson() const
{
    if (!m_root)
        return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void UpdateFirmware::print() const
{
    char *data = toJson();
    if (data) {
        printf("[UpdateFirmware] %s\n", data);
        free(data);
    }
}
#if 0
#include <stdio.h>
#include <stdlib.h>
#include "updatefirmware.h"

// ===== Example 1: Server sends firmware update request =====
void example_server_send_request()
{
    printf("=== Example 1: Server sends firmware update request ===\n");

    UpdateFirmware req;
    req.buildReq();
    req.setMsgSeq(7001);
    req.setLocation("https://firmware.example.com/v2.3.1.bin");
    req.setRetries(3);
    req.setRetryInterval(60);
    req.setRetrieveDate("2023-06-15T02:00:00Z");

    char *json = req.toJson();
    printf("Send: %s\n", json);
    free(json);

    printf("  location: %s\n", req.location());
    printf("  retrieveDate: %s\n", req.retrieveDate());
    printf("  retries: %d\n", req.retries());
    printf("  retryInterval: %d seconds\n", req.retryInterval());
    printf("\n");
}

// ===== Example 2: Charge point responds (empty payload) =====
void example_cp_respond()
{
    printf("=== Example 2: Charge point responds (empty payload) ===\n");

    UpdateFirmware conf;
    conf.buildConf();
    conf.setMsgSeq(7001);

    char *json = conf.toJson();
    printf("Response: %s\n", json);
    free(json);
    printf("\n");
}

// ===== Example 3: Parse incoming firmware update request =====
void example_parse_request()
{
    printf("=== Example 3: Parse incoming firmware update request ===\n");

    const char *incoming = "[2,\"8888\",\"UpdateFirmware\",{\"location\":\"http://192.168.1.100/fw.bin\",\"retrieveDate\":\"2023-07-01T00:00:00Z\",\"retries\":5,\"retryInterval\":120}]";
    printf("Incoming: %s\n", incoming);

    UpdateFirmware msg;
    if (msg.parse(incoming)) {
        printf("Parse OK\n");
        printf("  type: %s\n", msg.isCall() ? "CALL (request)" : "other");
        printf("  msgId: %llu\n", (unsigned long long)msg.msgSeq());
        printf("  location: %s\n", msg.location());
        printf("  retrieveDate: %s\n", msg.retrieveDate());

        if (msg.hasRetries()) {
            printf("  retries: %d\n", msg.retries());
        } else {
            printf("  retries: not specified\n");
        }

        if (msg.hasRetryInterval()) {
            printf("  retryInterval: %d seconds\n", msg.retryInterval());
        } else {
            printf("  retryInterval: not specified\n");
        }

        // Process the firmware update...
        printf("  -> Scheduling firmware download at %s\n", msg.retrieveDate());
    } else {
        printf("Parse FAILED\n");
    }
    printf("\n");
}

// ===== Example 4: Parse incoming response =====
void example_parse_response()
{
    printf("=== Example 4: Parse incoming response ===\n");

    const char *incoming = "[3,\"8888\",{}]";
    printf("Incoming: %s\n", incoming);

    UpdateFirmware msg;
    if (msg.parse(incoming)) {
        printf("Parse OK\n");
        printf("  type: %s\n", msg.isCallResult() ? "CALLRESULT (response)" : "other");
        printf("  msgId: %llu\n", (unsigned long long)msg.msgSeq());
        printf("  -> Charge point accepted the firmware update request\n");
    } else {
        printf("Parse FAILED\n");
    }
    printf("\n");
}

// ===== Example 5: Full workflow - request then response =====
void example_full_workflow()
{
    printf("=== Example 5: Full workflow ===\n");

    // Step 1: Server builds and sends request
    UpdateFirmware req;
    req.buildReq();
    req.setMsgSeq(5001);
    req.setLocation("https://firmware.example.com/v3.0.0.bin");
    req.setRetrieveDate("2023-12-25T03:00:00Z");
    // No retries specified - use defaults

    char *reqJson = req.toJson();
    printf("[Server] Send: %s\n", reqJson);

    // Step 2: Charge point receives and parses request
    UpdateFirmware received;
    if (received.parse(reqJson)) {
        printf("[CP] Received firmware update request\n");
        printf("[CP]   Download URL: %s\n", received.location());
        printf("[CP]   Scheduled at: %s\n", received.retrieveDate());

        if (received.hasRetries()) {
            printf("[CP]   Retries: %d\n", received.retries());
        } else {
            printf("[CP]   Retries: not specified (use default)\n");
        }

        // Step 3: Charge point builds response
        UpdateFirmware resp;
        resp.buildConf();
        resp.setMsgSeq(received.msgSeq());  // Echo back the message ID

        char *respJson = resp.toJson();
        printf("[CP] Respond: %s\n", respJson);

        // Step 4: Server receives and parses response
        UpdateFirmware serverResp;
        if (serverResp.parse(respJson)) {
            printf("[Server] Response received (empty payload = accepted)\n");
            printf("[Server]   msgId: %llu\n", (unsigned long long)serverResp.msgSeq());
        }

        free(respJson);
    }

    free(reqJson);
    printf("\n");
}

// ===== Example 6: Request with only required fields =====
void example_minimal_request()
{
    printf("=== Example 6: Minimal request (only required fields) ===\n");

    UpdateFirmware req;
    req.buildReq();
    req.setMsgSeq(9001);
    req.setLocation("ftp://firmware.local/fw.bin");
    req.setRetrieveDate("2023-08-01T00:00:00Z");
    // No retries, no retryInterval - both are optional

    char *json = req.toJson();
    printf("JSON: %s\n", json);
    free(json);

    printf("  hasRetries: %s\n", req.hasRetries() ? "yes" : "no");
    printf("  hasRetryInterval: %s\n", req.hasRetryInterval() ? "yes" : "no");
    printf("\n");
}

// ===== Example 7: Clear optional fields =====
void example_clear_fields()
{
    printf("=== Example 7: Clear optional fields ===\n");

    UpdateFirmware req;
    req.buildReq();
    req.setMsgSeq(9002);
    req.setLocation("http://test.com/fw.bin");
    req.setRetrieveDate("2023-09-01T00:00:00Z");
    req.setRetries(5);
    req.setRetryInterval(300);

    printf("Before clear:\n");
    printf("  hasRetries: %s, retries: %d\n", req.hasRetries() ? "yes" : "no", req.retries());
    printf("  hasRetryInterval: %s, retryInterval: %d\n", req.hasRetryInterval() ? "yes" : "no", req.retryInterval());

    // Clear retries
    req.clearRetries();
    req.clearRetryInterval();

    printf("After clear:\n");
    printf("  hasRetries: %s\n", req.hasRetries() ? "yes" : "no");
    printf("  hasRetryInterval: %s\n", req.hasRetryInterval() ? "yes" : "no");

    char *json = req.toJson();
    printf("  JSON: %s\n", json);
    free(json);
    printf("\n");
}

int main()
{
    example_server_send_request();
    example_cp_respond();
    example_parse_request();
    example_parse_response();
    example_full_workflow();
    example_minimal_request();
    example_clear_fields();

    printf("All examples completed.\n");
    return 0;
}

=== Example 1: Server sends firmware update request ===
Send: [2,"7001","UpdateFirmware",{"location":"https://firmware.example.com/v2.3.1.bin","retries":3,"retryInterval":60,"retrieveDate":"2023-06-15T02:00:00Z"}]
  location: https://firmware.example.com/v2.3.1.bin
  retrieveDate: 2023-06-15T02:00:00Z
  retries: 3
  retryInterval: 60 seconds

=== Example 2: Charge point responds (empty payload) ===
Response: [3,"7001",{}]

=== Example 3: Parse incoming firmware update request ===
Incoming: [2,"8888","UpdateFirmware",{"location":"http://192.168.1.100/fw.bin","retrieveDate":"2023-07-01T00:00:00Z","retries":5,"retryInterval":120}]
Parse OK
  type: CALL (request)
  msgId: 8888
  location: http://192.168.1.100/fw.bin
  retrieveDate: 2023-07-01T00:00:00Z
  retries: 5
  retryInterval: 120 seconds
  -> Scheduling firmware download at 2023-07-01T00:00:00Z

=== Example 4: Parse incoming response ===
Incoming: [3,"8888",{}]
Parse OK
  type: CALLRESULT (response)
  msgId: 8888
  -> Charge point accepted the firmware update request

=== Example 5: Full workflow ===
[Server] Send: [2,"5001","UpdateFirmware",{"location":"https://firmware.example.com/v3.0.0.bin","retrieveDate":"2023-12-25T03:00:00Z"}]
[CP] Received firmware update request
[CP]   Download URL: https://firmware.example.com/v3.0.0.bin
[CP]   Scheduled at: 2023-12-25T03:00:00Z
[CP]   Retries: not specified (use default)
[CP] Respond: [3,"5001",{}]
[Server] Response received (empty payload = accepted)
[Server]   msgId: 5001

=== Example 6: Minimal request (only required fields) ===
JSON: [2,"9001","UpdateFirmware",{"location":"ftp://firmware.local/fw.bin","retrieveDate":"2023-08-01T00:00:00Z"}]
  hasRetries: no
  hasRetryInterval: no

=== Example 7: Clear optional fields ===
Before clear:
  hasRetries: yes, retries: 5
  hasRetryInterval: yes, retryInterval: 300
After clear:
  hasRetries: no
  hasRetryInterval: no
  JSON: [2,"9002","UpdateFirmware",{"location":"http://test.com/fw.bin","retrieveDate":"2023-09-01T00:00:00Z"}]
#endif
