#include "getconfiguration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GetConfiguration::GetConfiguration()
    : m_root(nullptr), m_type(0)
{
}

GetConfiguration::GetConfiguration(const char *value)
    : m_root(nullptr), m_type(0)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

GetConfiguration::GetConfiguration(cJSON *obj)
    : m_root(nullptr), m_type(0)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);
        }
    }
}

GetConfiguration::~GetConfiguration()
{
    clear();
}

GetConfiguration::GetConfiguration(GetConfiguration&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type)
{
    other.m_root = nullptr;
    other.m_type = 0;
}

GetConfiguration& GetConfiguration::operator=(GetConfiguration&& other) noexcept
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

void GetConfiguration::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
}

bool GetConfiguration::ensurePayload()
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

bool GetConfiguration::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("GetConfiguration");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

bool GetConfiguration::buildConf()
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

    cJSON *keysArray = cJSON_CreateArray();
    if (!keysArray) { clear(); return false; }
    cJSON_AddItemToObject(payload, "configurationKey", keysArray);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALLRESULT;
    return true;
}

bool GetConfiguration::setMsgSeq(quint64 i)
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

bool GetConfiguration::setMsgSeq(QString i)
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

bool GetConfiguration::setType(quint8 type)
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
        cJSON *actionItem = cJSON_CreateString("GetConfiguration");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

// ===== Request setters =====

bool GetConfiguration::addKey(const char *key)
{
    if (m_type != OCPP_CALL) return false;
    if (!key || *key == '\0') return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON *keysArray = cJSON_GetObjectItem(payload, "key");
    if (!keysArray || !cJSON_IsArray(keysArray)) {
        if (keysArray) cJSON_DeleteItemFromObject(payload, "key");
        keysArray = cJSON_CreateArray();
        if (!keysArray) return false;
        cJSON_AddItemToObject(payload, "key", keysArray);
    }

    cJSON *keyItem = cJSON_CreateString(key);
    if (!keyItem) return false;

    cJSON_AddItemToArray(keysArray, keyItem);
    return true;
}

bool GetConfiguration::clearKeys()
{
    if (m_type != OCPP_CALL) return false;
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    cJSON_DeleteItemFromObject(payload, "key");
    return true;
}

// ===== Response setters =====

bool GetConfiguration::addConfigKey(const char *key, bool readonly, const char *value)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!key || *key == '\0') return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON *keysArray = cJSON_GetObjectItem(payload, "configurationKey");
    if (!keysArray || !cJSON_IsArray(keysArray)) {
        if (keysArray) cJSON_DeleteItemFromObject(payload, "configurationKey");
        keysArray = cJSON_CreateArray();
        if (!keysArray) return false;
        cJSON_AddItemToObject(payload, "configurationKey", keysArray);
    }

    cJSON *keyObj = cJSON_CreateObject();
    if (!keyObj) return false;

    cJSON *keyItem = cJSON_CreateString(key);
    if (!keyItem) { cJSON_Delete(keyObj); return false; }
    cJSON_AddItemToObject(keyObj, "key", keyItem);

    cJSON *roItem = cJSON_CreateBool(readonly);
    if (!roItem) { cJSON_Delete(keyObj); return false; }
    cJSON_AddItemToObject(keyObj, "readonly", roItem);

    if (value && *value != '\0') {
        cJSON *valItem = cJSON_CreateString(value);
        if (!valItem) { cJSON_Delete(keyObj); return false; }
        cJSON_AddItemToObject(keyObj, "value", valItem);
    }

    cJSON_AddItemToArray(keysArray, keyObj);
    return true;
}

bool GetConfiguration::clearConfigKeys()
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON_DeleteItemFromObject(payload, "configurationKey");
    return true;
}

bool GetConfiguration::addUnknownKey(const char *key)
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!key || *key == '\0') return false;
    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON *keysArray = cJSON_GetObjectItem(payload, "unknownKey");
    if (!keysArray || !cJSON_IsArray(keysArray)) {
        if (keysArray) cJSON_DeleteItemFromObject(payload, "unknownKey");
        keysArray = cJSON_CreateArray();
        if (!keysArray) return false;
        cJSON_AddItemToObject(payload, "unknownKey", keysArray);
    }

    cJSON *keyItem = cJSON_CreateString(key);
    if (!keyItem) return false;

    cJSON_AddItemToArray(keysArray, keyItem);
    return true;
}

bool GetConfiguration::clearUnknownKeys()
{
    if (m_type != OCPP_CALLRESULT) return false;
    if (!m_root || !cJSON_IsArray(m_root)) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON_DeleteItemFromObject(payload, "unknownKey");
    return true;
}

// ===== Getters =====

quint64 GetConfiguration::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 GetConfiguration::type() const
{
    return m_type;
}

int GetConfiguration::keyCount() const
{
    if (m_type != OCPP_CALL || !m_root) return 0;
    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return 0;
    cJSON *keysArray = cJSON_GetObjectItem(payload, "key");
    if (!keysArray || !cJSON_IsArray(keysArray)) return 0;
    return cJSON_GetArraySize(keysArray);
}

const char* GetConfiguration::getKey(int index) const
{
    if (m_type != OCPP_CALL || !m_root) return "";
    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return "";
    cJSON *keysArray = cJSON_GetObjectItem(payload, "key");
    if (!keysArray || !cJSON_IsArray(keysArray)) return "";

    cJSON *keyItem = cJSON_GetArrayItem(keysArray, index);
    if (!keyItem || !cJSON_IsString(keyItem)) return "";

    return keyItem->valuestring;
}

int GetConfiguration::configKeyCount() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return 0;
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return 0;
    cJSON *keysArray = cJSON_GetObjectItem(payload, "configurationKey");
    if (!keysArray || !cJSON_IsArray(keysArray)) return 0;
    return cJSON_GetArraySize(keysArray);
}

bool GetConfiguration::getConfigKey(int index, const char *&key, bool &readonly, const char *&value) const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return false;
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;
    cJSON *keysArray = cJSON_GetObjectItem(payload, "configurationKey");
    if (!keysArray || !cJSON_IsArray(keysArray)) return false;

    cJSON *keyObj = cJSON_GetArrayItem(keysArray, index);
    if (!keyObj || !cJSON_IsObject(keyObj)) return false;

    cJSON *k = cJSON_GetObjectItem(keyObj, "key");
    cJSON *ro = cJSON_GetObjectItem(keyObj, "readonly");
    cJSON *v = cJSON_GetObjectItem(keyObj, "value");

    if (!k || !cJSON_IsString(k) || !ro || !cJSON_IsBool(ro)) return false;

    key = k->valuestring;
    readonly = cJSON_IsTrue(ro);
    value = (v && cJSON_IsString(v)) ? v->valuestring : "";

    return true;
}

int GetConfiguration::unknownKeyCount() const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return 0;
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return 0;
    cJSON *keysArray = cJSON_GetObjectItem(payload, "unknownKey");
    if (!keysArray || !cJSON_IsArray(keysArray)) return 0;
    return cJSON_GetArraySize(keysArray);
}

const char* GetConfiguration::getUnknownKey(int index) const
{
    if (m_type != OCPP_CALLRESULT || !m_root) return "";
    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return "";
    cJSON *keysArray = cJSON_GetObjectItem(payload, "unknownKey");
    if (!keysArray || !cJSON_IsArray(keysArray)) return "";

    cJSON *keyItem = cJSON_GetArrayItem(keysArray, index);
    if (!keyItem || !cJSON_IsString(keyItem)) return "";

    return keyItem->valuestring;
}

bool GetConfiguration::isCall() const
{
    return m_type == OCPP_CALL;
}

bool GetConfiguration::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool GetConfiguration::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool GetConfiguration::parse(const char *value)
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

bool GetConfiguration::parse(cJSON *obj)
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
        if (strcmp(actionItem->valuestring, "GetConfiguration") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;
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

char* GetConfiguration::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

void GetConfiguration::print() const
{
    char *data = toJson();
    if (data) {
        printf("[GetConfiguration] %s\n", data);
        free(data);
    }
}
#if 0

构建请求
GetConfiguration gc;
gc.buildReq();
gc.setMsgSeq(1001);
gc.addKey("HeartbeatInterval");
gc.addKey("ConnectionTimeout");
char *json = gc.toJson();
// [2,"1001","GetConfiguration",{"key":["HeartbeatInterval","ConnectionTimeout"]}]
free(json);

构建响应
GetConfiguration gc;
gc.buildConf();
gc.setMsgSeq(1001);
gc.addConfigKey("HeartbeatInterval", false, "300");
gc.addConfigKey("ConnectionTimeout", true, "60");
gc.addUnknownKey("UnknownConfig");
char *json = gc.toJson();
// [3,"1001",{"configurationKey":[...], "unknownKey":["UnknownConfig"]}]
free(json);

解析响应并遍历
GetConfiguration gc;
if (gc.parse(jsonString)) {
    int count = gc.configKeyCount();
    for (int i = 0; i < count; i++) {
        const char *key;
        bool readonly;
        const char *value;
        if (gc.getConfigKey(i, key, readonly, value)) {
            printf("%s: %s (readonly: %d)\n", key, value, readonly);
        }
    }
}

#endif
