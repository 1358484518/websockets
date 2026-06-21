#include "sendlocallist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

SendLocalList::SendLocalList()
    : m_root(nullptr), m_type(0), m_respStatus(SEND_LIST_FAILED)
{}

SendLocalList::SendLocalList(const char *value)
    : m_root(nullptr), m_type(0), m_respStatus(SEND_LIST_FAILED)
{
    if (value && *value != '\0') parse(value);
}

SendLocalList::SendLocalList(cJSON *obj)
    : m_root(nullptr), m_type(0), m_respStatus(SEND_LIST_FAILED)
{
    if (obj && !parse(obj)) cJSON_Delete(obj);
}

SendLocalList::~SendLocalList()
{
    clear();
}

SendLocalList::SendLocalList(SendLocalList&& other) noexcept
    : m_root(other.m_root), m_type(other.m_type), m_respStatus(other.m_respStatus)
{
    other.m_root = nullptr;
    other.m_type = 0;
    other.m_respStatus = SEND_LIST_FAILED;
}

SendLocalList& SendLocalList::operator=(SendLocalList&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        m_type = other.m_type;
        m_respStatus = other.m_respStatus;
        other.m_root = nullptr;
        other.m_type = 0;
        other.m_respStatus = SEND_LIST_FAILED;
    }
    return *this;
}

void SendLocalList::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
    m_type = 0;
    m_respStatus = SEND_LIST_FAILED;
}

bool SendLocalList::ensurePayload()
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    int idx = (m_type == OCPP_CALL) ? 3 : 2;
    cJSON *pay = cJSON_GetArrayItem(m_root, idx);
    if (pay && cJSON_IsObject(pay)) return true;
    if (pay) cJSON_DeleteItemFromArray(m_root, idx);
    cJSON *obj = cJSON_CreateObject();
    if (!obj) return false;
    cJSON_InsertItemInArray(m_root, idx, obj);
    return true;
}

cJSON* SendLocalList::getAuthArray() const
{
    if (m_type != OCPP_CALL || !m_root) return nullptr;
    cJSON *pay = cJSON_GetArrayItem(m_root, 3);
    if (!pay) return nullptr;
    cJSON *arr = cJSON_GetObjectItem(pay, "localAuthorizationList");
    if (!arr || !cJSON_IsArray(arr)) return nullptr;
    return arr;
}

cJSON* SendLocalList::ensureAuthArray()
{
    if (m_type != OCPP_CALL) return nullptr;
    if (!ensurePayload()) return nullptr;
    cJSON *pay = cJSON_GetArrayItem(m_root, 3);
    cJSON *arr = cJSON_GetObjectItem(pay, "localAuthorizationList");
    if (arr && cJSON_IsArray(arr)) return arr;
    if (arr) cJSON_DeleteItemFromObject(pay, "localAuthorizationList");
    arr = cJSON_CreateArray();
    if (!arr) return nullptr;
    cJSON_AddItemToObject(pay, "localAuthorizationList", arr);
    return arr;
}

const char* SendLocalList::updateTypeToString(quint8 t)
{
    switch(t) {
        case UPDATE_DIFFERENTIAL: return "Differential";
        case UPDATE_FULL: return "Full";
        default: return "Full";
    }
}
quint8 SendLocalList::stringToUpdateType(const char *s)
{
    if (!s) return UPDATE_FULL;
    if (strcmp(s,"Differential")==0) return UPDATE_DIFFERENTIAL;
    return UPDATE_FULL;
}

const char* SendLocalList::sendListStatusToString(quint8 s)
{
    switch(s) {
        case SEND_LIST_ACCEPTED: return "Accepted";
        case SEND_LIST_FAILED: return "Failed";
        case SEND_LIST_NOT_SUPPORTED: return "NotSupported";
        case SEND_LIST_VERSION_MISMATCH: return "VersionMismatch";
        default: return "Failed";
    }
}
quint8 SendLocalList::stringToSendListStatus(const char *s)
{
    if (!s) return SEND_LIST_FAILED;
    if(strcmp(s,"Accepted")==0) return SEND_LIST_ACCEPTED;
    if(strcmp(s,"Failed")==0) return SEND_LIST_FAILED;
    if(strcmp(s,"NotSupported")==0) return SEND_LIST_NOT_SUPPORTED;
    if(strcmp(s,"VersionMismatch")==0) return SEND_LIST_VERSION_MISMATCH;
    return SEND_LIST_FAILED;
}

const char* SendLocalList::authStatusToString(quint8 s)
{
    switch(s) {
        case AUTH_ACCEPTED: return "Accepted";
        case AUTH_BLOCKED: return "Blocked";
        case AUTH_EXPIRED: return "Expired";
        case AUTH_INVALID: return "Invalid";
        case AUTH_CONCURRENT_TX: return "ConcurrentTx";
        default: return "Invalid";
    }
}
quint8 SendLocalList::stringToAuthStatus(const char *s)
{
    if (!s) return AUTH_INVALID;
    if(strcmp(s,"Accepted")==0) return AUTH_ACCEPTED;
    if(strcmp(s,"Blocked")==0) return AUTH_BLOCKED;
    if(strcmp(s,"Expired")==0) return AUTH_EXPIRED;
    if(strcmp(s,"ConcurrentTx")==0) return AUTH_CONCURRENT_TX;
    return AUTH_INVALID;
}

bool SendLocalList::buildReq()
{
    clear();
    m_root = cJSON_CreateArray();
    if (!m_root) return false;
    cJSON_AddItemToArray(m_root, cJSON_CreateNumber(OCPP_CALL));
    cJSON_AddItemToArray(m_root, cJSON_CreateString("0"));
    cJSON_AddItemToArray(m_root, cJSON_CreateString("SendLocalList"));
    cJSON_AddItemToArray(m_root, cJSON_CreateObject());
    m_type = OCPP_CALL;
    return true;
}

bool SendLocalList::buildConf()
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

bool SendLocalList::setMsgSeq(quint64 i)
{
    if (!m_root || !cJSON_IsArray(m_root)) return false;
    char buf[32];
    snprintf(buf,sizeof(buf),"%llu",(unsigned long long)i);
    cJSON *newItem = cJSON_CreateString(buf);
    if (!newItem) return false;
    cJSON *old = cJSON_GetArrayItem(m_root,1);
    if (old) cJSON_ReplaceItemInArray(m_root,1,newItem);
    else cJSON_InsertItemInArray(m_root,1,newItem);
    return true;
}
bool SendLocalList::setType(quint8 type)
{
    if (!m_root || (type != OCPP_CALL && type != OCPP_CALLRESULT))
        return false;
    if (m_type == type) return true;
    cJSON *tItem = cJSON_GetArrayItem(m_root,0);
    if (!tItem) return false;

    if (m_type == OCPP_CALL && type == OCPP_CALLRESULT)
    {
        cJSON *act = cJSON_GetArrayItem(m_root,2);
        if (act)
        {
            cJSON_DetachItemFromArray(m_root,2);
            cJSON_Delete(act);
        }
    }
    else if (m_type == OCPP_CALLRESULT && type == OCPP_CALL)
    {
        // 修复：使用Insert指定下标2插入
        cJSON_InsertItemInArray(m_root, 2, cJSON_CreateString("SendLocalList"));
    }

    tItem->valueint = type;
    tItem->valuedouble = type;
    m_type = type;
    return true;
}

// Request setters
bool SendLocalList::setListVersion(qint32 ver)
{
    if (m_type != OCPP_CALL || !ensurePayload()) return false;
    cJSON *pay = cJSON_GetArrayItem(m_root,3);
    cJSON *it = cJSON_GetObjectItem(pay,"listVersion");
    if (it && cJSON_IsNumber(it)) {
        it->valuedouble = ver;
        it->valueint = ver;
    } else {
        if (it) cJSON_DeleteItemFromObject(pay,"listVersion");
        cJSON_AddItemToObject(pay,"listVersion",cJSON_CreateNumber(ver));
    }
    return true;
}

bool SendLocalList::setUpdateType(quint8 type)
{
    if (m_type != OCPP_CALL || !ensurePayload()) return false;
    cJSON *pay = cJSON_GetArrayItem(m_root,3);
    const char *txt = updateTypeToString(type);
    cJSON *it = cJSON_GetObjectItem(pay,"updateType");
    if (it && cJSON_IsString(it)) {
        cJSON_ReplaceItemInObject(pay,"updateType",cJSON_CreateString(txt));
    } else {
        if (it) cJSON_DeleteItemFromObject(pay,"updateType");
        cJSON_AddItemToObject(pay,"updateType",cJSON_CreateString(txt));
    }
    return true;
}

int SendLocalList::addAuthorization(const char *idTag, quint8 authStatus,
                                    const char *expiryDate, const char *parentIdTag)
{
    if (m_type != OCPP_CALL || !idTag || *idTag=='\0') return -1;
    cJSON *arr = ensureAuthArray();
    if (!arr) return -1;
    cJSON *obj = cJSON_CreateObject();
    if (!obj) return -1;
    cJSON_AddItemToObject(obj,"idTag",cJSON_CreateString(idTag));
    cJSON_AddItemToObject(obj,"status",cJSON_CreateString(authStatusToString(authStatus)));
    if (expiryDate && *expiryDate!='\0')
        cJSON_AddItemToObject(obj,"expiryDate",cJSON_CreateString(expiryDate));
    if (parentIdTag && *parentIdTag!='\0')
        cJSON_AddItemToObject(obj,"parentIdTag",cJSON_CreateString(parentIdTag));
    int idx = cJSON_GetArraySize(arr);
    cJSON_AddItemToArray(arr,obj);
    return idx;
}

bool SendLocalList::clearAuthorizations()
{
    if (m_type != OCPP_CALL) return false;
    cJSON *arr = getAuthArray();
    if (!arr) return true;
    cJSON_Delete(arr);
    ensureAuthArray();
    return true;
}

// Response setter
bool SendLocalList::setStatus(quint8 status)
{
    if (m_type != OCPP_CALLRESULT || !ensurePayload()) return false;
    cJSON *pay = cJSON_GetArrayItem(m_root,2);
    const char *txt = sendListStatusToString(status);
    cJSON *it = cJSON_GetObjectItem(pay,"status");
    if (it && cJSON_IsString(it)) {
        cJSON_ReplaceItemInObject(pay,"status",cJSON_CreateString(txt));
    } else {
        if (it) cJSON_DeleteItemFromObject(pay,"status");
        cJSON_AddItemToObject(pay,"status",cJSON_CreateString(txt));
    }
    m_respStatus = status;
    return true;
}

// Getters
quint64 SendLocalList::msgSeq() const
{
    if (!m_root) return 0;
    cJSON *id = cJSON_GetArrayItem(m_root,1);
    if (!id || !cJSON_IsString(id)) return 0;
    return strtoull(id->valuestring,nullptr,10);
}
quint8 SendLocalList::type() const { return m_type; }

qint32 SendLocalList::listVersion() const
{
    if (m_type != OCPP_CALL || !m_root) return 0;
    cJSON *pay = cJSON_GetArrayItem(m_root,3);
    cJSON *it = cJSON_GetObjectItem(pay,"listVersion");
    if (!it || !cJSON_IsNumber(it)) return 0;
    return (qint32)it->valuedouble;
}
quint8 SendLocalList::updateType() const
{
    if (m_type != OCPP_CALL || !m_root) return UPDATE_FULL;
    cJSON *pay = cJSON_GetArrayItem(m_root,3);
    cJSON *it = cJSON_GetObjectItem(pay,"updateType");
    if (!it || !cJSON_IsString(it)) return UPDATE_FULL;
    return stringToUpdateType(it->valuestring);
}
const char* SendLocalList::updateTypeString() const
{
    return updateTypeToString(updateType());
}

int SendLocalList::authorizationCount() const
{
    cJSON *arr = getAuthArray();
    return arr ? cJSON_GetArraySize(arr) : 0;
}

bool SendLocalList::getAuthorization(int idx,
    const char *&idTag, quint8 &authStatus,
    const char *&expiryDate, const char *&parentIdTag) const
{
    cJSON *arr = getAuthArray();
    if (!arr || idx<0 || idx>=cJSON_GetArraySize(arr)) return false;
    cJSON *obj = cJSON_GetArrayItem(arr,idx);
    if (!obj || !cJSON_IsObject(obj)) return false;
    cJSON *itId = cJSON_GetObjectItem(obj,"idTag");
    cJSON *itSt = cJSON_GetObjectItem(obj,"status");
    if (!itId || !cJSON_IsString(itId) || !itSt || !cJSON_IsString(itSt)) return false;
    idTag = itId->valuestring;
    authStatus = stringToAuthStatus(itSt->valuestring);
    cJSON *ed = cJSON_GetObjectItem(obj,"expiryDate");
    expiryDate = (ed && cJSON_IsString(ed)) ? ed->valuestring : "";
    cJSON *pt = cJSON_GetObjectItem(obj,"parentIdTag");
    parentIdTag = (pt && cJSON_IsString(pt)) ? pt->valuestring : "";
    return true;
}

quint8 SendLocalList::status() const { return m_respStatus; }
const char* SendLocalList::statusString() const { return sendListStatusToString(m_respStatus); }

bool SendLocalList::isCall() const { return m_type == OCPP_CALL; }
bool SendLocalList::isCallResult() const { return m_type == OCPP_CALLRESULT; }
bool SendLocalList::isValid() const { return m_root != nullptr; }

// Parse
bool SendLocalList::parse(const char *value)
{
    if (!value || *value=='\0') return false;
    cJSON *obj = cJSON_Parse(value);
    if (!obj) return false;
    bool ok = parse(obj);
    if (!ok) cJSON_Delete(obj);
    return ok;
}

bool SendLocalList::parse(cJSON *obj)
{
    if (!obj || !cJSON_IsArray(obj)) return false;
    int sz = cJSON_GetArraySize(obj);
    if (sz < 3) return false;
    cJSON *tItem = cJSON_GetArrayItem(obj,0);
    if (!tItem || !cJSON_IsNumber(tItem)) return false;
    quint8 msgType = (quint8)tItem->valueint;
    if (msgType != OCPP_CALL && msgType != OCPP_CALLRESULT) return false;
    cJSON *msgId = cJSON_GetArrayItem(obj,1);
    if (!msgId || !cJSON_IsString(msgId)) return false;

    quint8 stat = SEND_LIST_FAILED;
    if (msgType == OCPP_CALL) {
        if (sz < 4) return false;
        cJSON *act = cJSON_GetArrayItem(obj,2);
        if (!act || !cJSON_IsString(act) || strcmp(act->valuestring,"SendLocalList")!=0)
            return false;
        cJSON *pay = cJSON_GetArrayItem(obj,3);
        if (!pay || !cJSON_IsObject(pay)) return false;
        // 必填字段校验
        cJSON *ver = cJSON_GetObjectItem(pay,"listVersion");
        cJSON *ut = cJSON_GetObjectItem(pay,"updateType");
        if (!ver || !cJSON_IsNumber(ver) || !ut || !cJSON_IsString(ut))
            return false;
    } else {
        cJSON *pay = cJSON_GetArrayItem(obj,2);
        if (!pay || !cJSON_IsObject(pay)) return false;
        cJSON *st = cJSON_GetObjectItem(pay,"status");
        if (!st || !cJSON_IsString(st)) return false;
        stat = stringToSendListStatus(st->valuestring);
    }

    clear();
    m_root = obj;
    m_type = msgType;
    m_respStatus = stat;
    return true;
}

char* SendLocalList::toJson() const
{
    return m_root ? cJSON_PrintUnformatted(m_root) : nullptr;
}

void SendLocalList::print() const
{
    char *s = toJson();
    if (s) {
        printf("[SendLocalList] %s\n", s);
        free(s);
    }
}

#if 0
#include <stdio.h>
#include <stdlib.h>
#include "sendlocallist.h"

int main()
{
    // 1. 构造下发本地白名单请求
    SendLocalList req;
    req.buildReq();
    req.setMsgSeq(6001);
    req.setListVersion(2);
    req.setUpdateType(UPDATE_DIFFERENTIAL);

    // 添加两条卡片
    req.addAuthorization("RFID001", AUTH_ACCEPTED, "2030-01-01T00:00:00Z", "PARENT_01");
    req.addAuthorization("RFID002", AUTH_BLOCKED, nullptr, nullptr);

    printf("=== SendLocalList Request ===\n");
    req.print();
    printf("listVersion: %d\n", req.listVersion());
    printf("updateType: %s\n", req.updateTypeString());
    printf("total auth count: %d\n", req.authorizationCount());

    // 遍历所有卡片
    for(int i=0; i<req.authorizationCount(); i++){
        const char *tag, *exp, *parent;
        quint8 st;
        if(req.getAuthorization(i, tag, st, exp, parent)){
            printf("idx%d | idTag:%s status:%d exp:%s parent:%s\n",i,tag,st,exp,parent);
        }
    }

    // 2. 构造充电桩回复响应
    SendLocalList conf;
    conf.buildConf();
    conf.setMsgSeq(6001);
    conf.setStatus(SEND_LIST_ACCEPTED);
    printf("\n=== Response ===\n");
    conf.print();
    printf("resp status: %s\n", conf.statusString());

    // 3. 解析JSON字符串
    const char *jsonReq = "[2,\"6002\",\"SendLocalList\",{"
        "\"listVersion\":3,"
        "\"updateType\":\"Full\","
        "\"localAuthorizationList\":["
            "{\"idTag\":\"TEST01\",\"status\":\"Accepted\"}"
        "]}]";
    SendLocalList parseReq;
    if(parseReq.parse(jsonReq)){
        printf("\n=== Parse Request OK ===\n");
        printf("ver:%d type:%s count:%d\n",
            parseReq.listVersion(), parseReq.updateTypeString(), parseReq.authorizationCount());
    }

    const char *jsonConf = "[3,\"6002\",{\"status\":\"VersionMismatch\"}]";
    SendLocalList parseConf;
    if(parseConf.parse(jsonConf)){
        printf("\n=== Parse Response OK ===\n");
        printf("status:%s\n", parseConf.statusString());
    }

    return 0;
}
#endif
