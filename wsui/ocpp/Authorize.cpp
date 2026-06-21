#include "authorize.h"
#include <QDebug>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ===== Helper: status string conversion =====

const char* Authorize::statusToString(quint8 s) const
{
    switch (s) {
        case AUTH_ACCEPTED:      return "Accepted";
        case AUTH_BLOCKED:       return "Blocked";
        case AUTH_EXPIRED:       return "Expired";
        case AUTH_INVALID:       return "Invalid";
        case AUTH_CONCURRENT_TX: return "ConcurrentTx";
        default: return "Invalid";
    }
}

quint8 Authorize::stringToStatus(const char *str) const
{
    if (!str) return AUTH_INVALID;
    if (strcmp(str, "Accepted") == 0) return AUTH_ACCEPTED;
    if (strcmp(str, "Blocked") == 0) return AUTH_BLOCKED;
    if (strcmp(str, "Expired") == 0) return AUTH_EXPIRED;
    if (strcmp(str, "Invalid") == 0) return AUTH_INVALID;
    if (strcmp(str, "ConcurrentTx") == 0) return AUTH_CONCURRENT_TX;
    return AUTH_INVALID;
}

// ===== Constructors =====

Authorize::Authorize()
    : m_root(nullptr)
    , m_type(0)
    , m_status(AUTH_INVALID)
{
}

Authorize::Authorize(const char *value)
    : m_root(nullptr)
    , m_type(0)
    , m_status(AUTH_INVALID)
{
    if (value && *value != '\0') {
        parse(value);
    }
}

Authorize::Authorize(cJSON *obj)
    : m_root(nullptr)
    , m_type(0)
    , m_status(AUTH_INVALID)
{
    if (obj) {
        if (!parse(obj)) {
            cJSON_Delete(obj);  // ✅ 失败时释放，避免泄漏
        }
    }
}

Authorize::~Authorize()
{
    clear();
}

// ===== Move semantics =====

Authorize::Authorize(Authorize&& other) noexcept
    : m_root(other.m_root)
    , m_type(other.m_type)
    , m_status(other.m_status)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_status = AUTH_INVALID;
}

Authorize& Authorize::operator=(Authorize&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_status = other.m_status;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_status = AUTH_INVALID;
    }
    return *this;
}

// ===== Clear =====

void Authorize::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_status = AUTH_INVALID;
}

// ===== Helper: set string field in object =====

bool Authorize::setStringField(cJSON *obj, const char *key, const char *value)
{
    if (!obj || !key || !value) return false;

    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (item && cJSON_IsString(item)) {
        char *newStr = strdup(value);
        if (!newStr) return false;
        free(item->valuestring);
        item->valuestring = newStr;
        return true;
    }

    if (item) {
        cJSON_DeleteItemFromObject(obj, key);
    }

    cJSON *newItem = cJSON_CreateString(value);
    if (!newItem) return false;

    cJSON_AddItemToObject(obj, key, newItem);
    return true;
}

// ===== Helper: ensure payload exists =====

bool Authorize::ensurePayload()
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

bool Authorize::ensureIdTagInfo()
{
    if (!ensurePayload()) return false;

    int payloadIndex = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *payload = cJSON_GetArrayItem(m_root, payloadIndex);
    if (!payload) return false;

    cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
    if (idTagInfo && cJSON_IsObject(idTagInfo)) {
        return true;
    }

    if (idTagInfo) {
        cJSON_DeleteItemFromObject(payload, "idTagInfo");
    }

    cJSON *newInfo = cJSON_CreateObject();
    if (!newInfo) return false;

    cJSON_AddItemToObject(payload, "idTagInfo", newInfo);
    return true;
}

// ===== Build request =====

bool Authorize::buildReq()
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

    cJSON *actionItem = cJSON_CreateString("Authorize");
    if (!actionItem) { clear(); return false; }
    cJSON_AddItemToArray(m_root, actionItem);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) { clear(); return false; }

    cJSON *idTagItem = cJSON_CreateString("");
    if (!idTagItem) { clear(); return false; }
    cJSON_AddItemToObject(payload, "idTag", idTagItem);

    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALL;
    return true;
}

// ===== Build response =====

bool Authorize::buildConf()
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

    cJSON *idTagInfo = cJSON_CreateObject();
    if (!idTagInfo) { clear(); return false; }

    cJSON *statusItem = cJSON_CreateString("Accepted");
    if (!statusItem) { clear(); return false; }
    cJSON_AddItemToObject(idTagInfo, "status", statusItem);

    cJSON_AddItemToObject(payload, "idTagInfo", idTagInfo);
    cJSON_AddItemToArray(m_root, payload);

    m_type = OCPP_CALLRESULT;
    m_status = AUTH_ACCEPTED;
    return true;
}

// ===== Setters =====

bool Authorize::setMsgSeq(quint64 i)
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

bool Authorize::setType(quint8 type)
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
        // Switching CALL -> CALLRESULT: remove action element at index 2
        cJSON *actionItem = cJSON_GetArrayItem(m_root, 2);
        if (actionItem) {
            cJSON_DetachItemFromArray(m_root, 2);
            cJSON_Delete(actionItem);
        }
    } else if (m_type == OCPP_CALLRESULT && type == OCPP_CALL) {
        // Switching CALLRESULT -> CALL: insert action element at index 2
        cJSON *actionItem = cJSON_CreateString("Authorize");
        if (!actionItem) return false;
        cJSON_InsertItemInArray(m_root, 2, actionItem);
    }

    typeItem->valueint = type;
    typeItem->valuedouble = type;
    m_type = type;

    return true;
}

bool Authorize::setState(quint8 s)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    if (m_type != OCPP_CALLRESULT) return false;

    if (!ensureIdTagInfo()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
    if (!idTagInfo) return false;

    if (!setStringField(idTagInfo, "status", statusToString(s))) {
        return false;
    }

    m_status = s;  // ✅ 所有操作成功后再更新成员变量
    return true;
}

bool Authorize::setIdTag(const char *idTag)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    if (m_type != OCPP_CALL) return false;

    if (!ensurePayload()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload) return false;

    return setStringField(payload, "idTag", idTag ? idTag : "");
}

bool Authorize::setExpiryDate(const char *date)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    if (m_type != OCPP_CALLRESULT) return false;

    if (!ensureIdTagInfo()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
    if (!idTagInfo) return false;

    if (date && *date) {
        return setStringField(idTagInfo, "expiryDate", date);
    } else {
        cJSON_DeleteItemFromObject(idTagInfo, "expiryDate");
        return true;
    }
}

bool Authorize::setParentIdTag(const char *parentId)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    if (m_type != OCPP_CALLRESULT) return false;

    if (!ensureIdTagInfo()) return false;

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload) return false;

    cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
    if (!idTagInfo) return false;

    if (parentId && *parentId) {
        return setStringField(idTagInfo, "parentIdTag", parentId);
    } else {
        cJSON_DeleteItemFromObject(idTagInfo, "parentIdTag");
        return true;
    }
}

// ===== Getters =====

quint64 Authorize::msgSeq() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return 0;

    cJSON *idItem = cJSON_GetArrayItem(m_root, 1);
    if (!idItem || !cJSON_IsString(idItem)) return 0;

    return strtoull(idItem->valuestring, nullptr, 10);
}

quint8 Authorize::type() const
{
    return m_type;
}

quint8 Authorize::state() const
{
    return m_status;
}

const char* Authorize::idTag() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return "";
    if (m_type != OCPP_CALL) return "";

    cJSON *payload = cJSON_GetArrayItem(m_root, 3);
    if (!payload || !cJSON_IsObject(payload)) return "";

    cJSON *item = cJSON_GetObjectItem(payload, "idTag");
    return item && cJSON_IsString(item) ? item->valuestring : "";
}

const char* Authorize::status() const
{
    return statusToString(m_status);
}

const char* Authorize::expiryDate() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return "";
    if (m_type != OCPP_CALLRESULT) return "";

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload || !cJSON_IsObject(payload)) return "";

    cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
    if (!idTagInfo || !cJSON_IsObject(idTagInfo)) return "";

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "expiryDate");
    return item && cJSON_IsString(item) ? item->valuestring : "";
}

const char* Authorize::parentIdTag() const
{
    if (!m_root || !cJSON_IsArray(m_root)) return "";
    if (m_type != OCPP_CALLRESULT) return "";

    cJSON *payload = cJSON_GetArrayItem(m_root, 2);
    if (!payload || !cJSON_IsObject(payload)) return "";

    cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
    if (!idTagInfo || !cJSON_IsObject(idTagInfo)) return "";

    cJSON *item = cJSON_GetObjectItem(idTagInfo, "parentIdTag");
    return item && cJSON_IsString(item) ? item->valuestring : "";
}

bool Authorize::isCall() const
{
    return m_type == OCPP_CALL;
}

bool Authorize::isCallResult() const
{
    return m_type == OCPP_CALLRESULT;
}

bool Authorize::isValid() const
{
    return m_root != nullptr;
}

// ===== Parse =====

bool Authorize::parse(const char *value)
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

bool Authorize::parse(cJSON *obj)
{
    if (!obj || !cJSON_IsArray(obj)) {
        return false;
    }

    int size = cJSON_GetArraySize(obj);
    if (size < 3) {
        return false;
    }

    cJSON *typeItem = cJSON_GetArrayItem(obj, 0);
    if (!typeItem || !cJSON_IsNumber(typeItem)) {
        return false;
    }

    quint8 type = (quint8)typeItem->valueint;
    if (type != OCPP_CALL && type != OCPP_CALLRESULT) {
        return false;
    }

    cJSON *idItem = cJSON_GetArrayItem(obj, 1);
    if (!idItem || !cJSON_IsString(idItem)) {
        return false;
    }

    quint8 status = AUTH_INVALID;

    if (type == OCPP_CALL) {
        if (size < 4) return false;

        cJSON *actionItem = cJSON_GetArrayItem(obj, 2);
        if (!actionItem || !cJSON_IsString(actionItem)) return false;
        if (strcmp(actionItem->valuestring, "Authorize") != 0) return false;

        cJSON *payload = cJSON_GetArrayItem(obj, 3);
        if (!payload || !cJSON_IsObject(payload)) return false;
    }

    if (type == OCPP_CALLRESULT) {
        cJSON *payload = cJSON_GetArrayItem(obj, 2);
        if (!payload || !cJSON_IsObject(payload)) return false;

        cJSON *idTagInfo = cJSON_GetObjectItem(payload, "idTagInfo");
        if (idTagInfo && cJSON_IsObject(idTagInfo)) {
            cJSON *statusItem = cJSON_GetObjectItem(idTagInfo, "status");
            if (statusItem && cJSON_IsString(statusItem)) {
                status = stringToStatus(statusItem->valuestring);
            }
        }
    }

    // ✅ 先清理，再接管，最后设置状态
    clear();
    m_root = obj;
    m_type = type;
    m_status = status;

    return true;
}

// ===== Serialize =====

char* Authorize::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

// ===== Debug print =====

void Authorize::print() const
{
    char *data = toJson();
    if (data) {
        qDebug() << "[Authorize]" << data;
        free(data);
    }
}
#if 0
//1. 构建并发送请求
Authorize req;
req.buildReq();
req.setMsgSeq(1001);
req.setIdTag("ABC1234567890");
req.print();

char *json = req.toJson();
sendWebSocket(json);
free(json);
[2, "1001", "Authorize", {"idTag": "ABC1234567890"}]

//2. 构建并发送响应
Authorize conf;
conf.buildConf();
conf.setMsgSeq(1001);
conf.setState(AUTH_ACCEPTED);
conf.setExpiryDate("2024-12-31T23:59:59Z");
conf.setParentIdTag("PARENT_001");
conf.print();

[3, "1001", {"idTagInfo": {"status": "Accepted", "expiryDate": "2024-12-31T23:59:59Z", "parentIdTag": "PARENT_001"}}]

//3. 解析收到的消息
Authorize msg(receivedJson);

if (msg.isCall()) {
    qDebug() << "收到请求，idTag:" << msg.idTag();
    // 处理请求...
} else if (msg.isCallResult()) {
    qDebug() << "收到响应，状态:" << msg.status();
    qDebug() << "过期时间:" << msg.expiryDate();
    qDebug() << "父标签:" << msg.parentIdTag();
}

//用枚举判断状态
if (msg.state() == AUTH_ACCEPTED) {
    qDebug() << "授权通过";
} else if (msg.state() == AUTH_BLOCKED) {
    qDebug() << "卡被冻结";
} else if (msg.state() == AUTH_EXPIRED) {
    qDebug() << "卡已过期";
} else if (msg.state() == AUTH_INVALID) {
    qDebug() << "无效卡";
}

//枚举值                   字符串                   说明
//AUTH_ACCEPTED         Accepted                授权通过
//AUTH_BLOCKED          Blocked                 卡被冻结
//AUTH_EXPIRED          Expired                 卡已过期
//AUTH_INVALID          Invalid                 无效卡
//AUTH_CONCURRENT_TX	ConcurrentTx            并发交易冲突
#endif
