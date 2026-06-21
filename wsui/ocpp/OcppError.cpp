#include "ocpperror.h"
#include <QDebug>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ===== Helper: error code to string =====

const char* OcppError::codeToString(OcppErrorCode code)
{
    switch (code) {
        case OcppErrorCode::NotImplemented:        return "NotImplemented";
        case OcppErrorCode::NotSupported:          return "NotSupported";
        case OcppErrorCode::InternalError:         return "InternalError";
        case OcppErrorCode::ProtocolError:         return "ProtocolError";
        case OcppErrorCode::SecurityError:         return "SecurityError";
        case OcppErrorCode::FormationViolation:    return "FormationViolation";
        case OcppErrorCode::PropertyConstraintViolation: return "PropertyConstraintViolation";
        case OcppErrorCode::OccurrenceConstraintViolation: return "OccurrenceConstraintViolation";
        case OcppErrorCode::TypeConstraintViolation: return "TypeConstraintViolation";
        case OcppErrorCode::GenericError:          return "GenericError";
        default: return "GenericError";
    }
}

const char* OcppError::defaultDescription(OcppErrorCode code)
{
    switch (code) {
        case OcppErrorCode::NotImplemented:        return "Requested action is not implemented";
        case OcppErrorCode::NotSupported:          return "Requested action is not supported";
        case OcppErrorCode::InternalError:         return "Internal error";
        case OcppErrorCode::ProtocolError:         return "Protocol error";
        case OcppErrorCode::SecurityError:         return "Security error";
        case OcppErrorCode::FormationViolation:    return "Message formation violation";
        case OcppErrorCode::PropertyConstraintViolation: return "Property constraint violation";
        case OcppErrorCode::OccurrenceConstraintViolation: return "Occurrence constraint violation";
        case OcppErrorCode::TypeConstraintViolation: return "Type constraint violation";
        case OcppErrorCode::GenericError:          return "Generic error";
        default: return "Generic error";
    }
}

// ===== Constructors =====

OcppError::OcppError() : m_root(nullptr)
{
}

OcppError::OcppError(const char *jsonStr) : m_root(nullptr)
{
    if (jsonStr && *jsonStr != '\0') {
        m_root = cJSON_Parse(jsonStr);
    }
}

OcppError::OcppError(const char *msgId, const char *code, const char *desc) : m_root(nullptr)
{
    create(msgId, code, desc);
}

OcppError::OcppError(const char *msgId, OcppErrorCode code, const char *desc) : m_root(nullptr)
{
    create(msgId, code, desc);
}

OcppError::~OcppError()
{
    clear();
}

// ===== Move semantics =====

OcppError::OcppError(OcppError&& other) noexcept : m_root(other.m_root)
{
    other.m_root = nullptr;
}

OcppError& OcppError::operator=(OcppError&& other) noexcept
{
    if (this != &other) {
        clear();
        m_root = other.m_root;
        other.m_root = nullptr;
    }
    return *this;
}

// ===== Clear / reset =====

void OcppError::clear()
{
    if (m_root) {
        cJSON_Delete(m_root);
        m_root = nullptr;
    }
}

// ===== Create error manually =====

void OcppError::create(const char *msgId, const char *code, const char *desc)
{
    clear();

    if (!msgId)  msgId = "";
    if (!code)   code = "";
    if (!desc)   desc = "";

    cJSON *arr = cJSON_CreateArray();
    if (!arr) return;

    cJSON *typeItem = cJSON_CreateNumber(4);
    if (!typeItem) { cJSON_Delete(arr); return; }
    cJSON_AddItemToArray(arr, typeItem);

    cJSON *idItem = cJSON_CreateString(msgId);
    if (!idItem) { cJSON_Delete(arr); return; }
    cJSON_AddItemToArray(arr, idItem);

    cJSON *codeItem = cJSON_CreateString(code);
    if (!codeItem) { cJSON_Delete(arr); return; }
    cJSON_AddItemToArray(arr, codeItem);

    cJSON *descItem = cJSON_CreateString(desc);
    if (!descItem) { cJSON_Delete(arr); return; }
    cJSON_AddItemToArray(arr, descItem);

    cJSON *detailsItem = cJSON_CreateObject();
    if (!detailsItem) { cJSON_Delete(arr); return; }
    cJSON_AddItemToArray(arr, detailsItem);

    m_root = arr;
}

void OcppError::create(const char *msgId, OcppErrorCode code, const char *desc)
{
    const char *descStr = desc ? desc : defaultDescription(code);
    create(msgId, codeToString(code), descStr);
}

// ===== Parse from JSON string =====

bool OcppError::parse(const char *jsonStr)
{
    if (!jsonStr || *jsonStr == '\0') {
        return false;
    }

    cJSON *obj = cJSON_Parse(jsonStr);
    if (!obj) {
        return false;
    }

    clear();
    m_root = obj;

    return isValid();
}

// ===== Validation =====

bool OcppError::isValid() const
{
    if (!m_root || !cJSON_IsArray(m_root)) {
        return false;
    }

    if (cJSON_GetArraySize(m_root) != 5) {
        return false;
    }

    cJSON *type = cJSON_GetArrayItem(m_root, 0);
    if (!type || !cJSON_IsNumber(type) || type->valueint != 4) {
        return false;
    }

    cJSON *id = cJSON_GetArrayItem(m_root, 1);
    if (!id || !cJSON_IsString(id)) return false;

    cJSON *code = cJSON_GetArrayItem(m_root, 2);
    if (!code || !cJSON_IsString(code)) return false;

    cJSON *desc = cJSON_GetArrayItem(m_root, 3);
    if (!desc || !cJSON_IsString(desc)) return false;

    cJSON *details = cJSON_GetArrayItem(m_root, 4);
    if (!details || !cJSON_IsObject(details)) return false;

    return true;
}

// ===== Getters =====

const char* OcppError::messageId() const
{
    if (!isValid()) return "";
    return cJSON_GetArrayItem(m_root, 1)->valuestring;
}

const char* OcppError::errorCode() const
{
    if (!isValid()) return "";
    return cJSON_GetArrayItem(m_root, 2)->valuestring;
}

const char* OcppError::description() const
{
    if (!isValid()) return "";
    return cJSON_GetArrayItem(m_root, 3)->valuestring;
}

cJSON* OcppError::details() const
{
    if (!isValid()) return nullptr;
    return cJSON_GetArrayItem(m_root, 4);
}

OcppErrorCode OcppError::errorCodeEnum() const
{
    if (!isValid()) return OcppErrorCode::GenericError;

    const char *code = errorCode();
    if (strcmp(code, "NotImplemented") == 0) return OcppErrorCode::NotImplemented;
    if (strcmp(code, "NotSupported") == 0) return OcppErrorCode::NotSupported;
    if (strcmp(code, "InternalError") == 0) return OcppErrorCode::InternalError;
    if (strcmp(code, "ProtocolError") == 0) return OcppErrorCode::ProtocolError;
    if (strcmp(code, "SecurityError") == 0) return OcppErrorCode::SecurityError;
    if (strcmp(code, "FormationViolation") == 0) return OcppErrorCode::FormationViolation;
    if (strcmp(code, "PropertyConstraintViolation") == 0) return OcppErrorCode::PropertyConstraintViolation;
    if (strcmp(code, "OccurrenceConstraintViolation") == 0) return OcppErrorCode::OccurrenceConstraintViolation;
    if (strcmp(code, "TypeConstraintViolation") == 0) return OcppErrorCode::TypeConstraintViolation;
    return OcppErrorCode::GenericError;
}

// ===== Add error details =====

void OcppError::addDetail(const char *key, const char *value)
{
    if (!isValid() || !key || !value) return;
    cJSON *det = cJSON_GetArrayItem(m_root, 4);
    if (det && cJSON_IsObject(det)) {
        cJSON_AddStringToObject(det, key, value);
    }
}

void OcppError::addDetail(const char *key, int value)
{
    if (!isValid() || !key) return;
    cJSON *det = cJSON_GetArrayItem(m_root, 4);
    if (det && cJSON_IsObject(det)) {
        cJSON_AddNumberToObject(det, key, value);
    }
}

// ===== Serialize =====

char* OcppError::toJson() const
{
    if (!m_root) return nullptr;
    return cJSON_PrintUnformatted(m_root);
}

// ===== Debug print =====

void OcppError::print() const
{
    char *data = toJson();
    if (data) {
        qDebug() << "[OcppError]" << data;
        free(data);
    }
}

// ===== Static factory methods =====

OcppError OcppError::makeNotImplemented(const char *msgId, const char *action)
{
    OcppError err(msgId, OcppErrorCode::NotImplemented);
    if (action && *action) {
        err.addDetail("action", action);
    }
    return err;
}

OcppError OcppError::makeNotSupported(const char *msgId, const char *action)
{
    OcppError err(msgId, OcppErrorCode::NotSupported);
    if (action && *action) {
        err.addDetail("action", action);
    }
    return err;
}

OcppError OcppError::makeFormationViolation(const char *msgId, const char *detail)
{
    OcppError err(msgId, OcppErrorCode::FormationViolation);
    if (detail && *detail) {
        err.addDetail("detail", detail);
    }
    return err;
}

OcppError OcppError::makeOccurrenceViolation(const char *msgId, const char *field)
{
    OcppError err(msgId, OcppErrorCode::OccurrenceConstraintViolation);
    if (field && *field) {
        err.addDetail("field", field);
    }
    return err;
}

OcppError OcppError::makePropertyViolation(const char *msgId, const char *field, const char *value)
{
    OcppError err(msgId, OcppErrorCode::PropertyConstraintViolation);
    if (field && *field) {
        err.addDetail("field", field);
    }
    if (value && *value) {
        err.addDetail("value", value);
    }
    return err;
}

OcppError OcppError::makeTypeViolation(const char *msgId, const char *field)
{
    OcppError err(msgId, OcppErrorCode::TypeConstraintViolation);
    if (field && *field) {
        err.addDetail("field", field);
    }
    return err;
}

OcppError OcppError::makeInternalError(const char *msgId, const char *detail)
{
    OcppError err(msgId, OcppErrorCode::InternalError);
    if (detail && *detail) {
        err.addDetail("detail", detail);
    }
    return err;
}

OcppError OcppError::makeProtocolError(const char *msgId, const char *detail)
{
    OcppError err(msgId, OcppErrorCode::ProtocolError);
    if (detail && *detail) {
        err.addDetail("detail", detail);
    }
    return err;
}

OcppError OcppError::makeSecurityError(const char *msgId, const char *detail)
{
    OcppError err(msgId, OcppErrorCode::SecurityError);
    if (detail && *detail) {
        err.addDetail("detail", detail);
    }
    return err;
}

OcppError OcppError::makeGenericError(const char *msgId, const char *detail)
{
    OcppError err(msgId, OcppErrorCode::GenericError);
    if (detail && *detail) {
        err.addDetail("detail", detail);
    }
    return err;
}


#if 0
// 1. Action 未实现
OcppError err = OcppError::makeNotImplemented(msgId, "UnknownAction");
// 生成: [4, "msgId", "NotImplemented", "Requested action is not implemented", {"action":"UnknownAction"}]

// 2. 必填字段缺失
OcppError err = OcppError::makeOccurrenceViolation(msgId, "idTag");
// 生成: [4, "msgId", "OccurrenceConstraintViolation", "...", {"field":"idTag"}]

// 3. JSON 格式错误
OcppError err = OcppError::makeFormationViolation(msgId, "Invalid JSON");
// 生成: [4, "msgId", "FormationViolation", "...", {"detail":"Invalid JSON"}]

// 4. 字段值非法
OcppError err = OcppError::makePropertyViolation(msgId, "connectorId", "999");
// 生成: [4, "msgId", "PropertyConstraintViolation", "...", {"field":"connectorId","value":"999"}]

// 5. 内部错误
OcppError err = OcppError::makeInternalError(msgId, "Database connection failed");

// 6. 用枚举创建
OcppError err(msgId, OcppErrorCode::NotSupported);
#endif
#if 0
工厂方法                    错误码                     适用场景
makeNotImplemented      NotImplemented              Action 没实现
makeNotSupported        NotSupported                Action 不支持
makeFormationViolation	FormationViolation          JSON 格式不对
makeOccurrenceViolation	OccurrenceConstraintViolation	必填字段缺失
makePropertyViolation	PropertyConstraintViolation	字段值超范围 / 非法
makeTypeViolation       TypeConstraintViolation     字段类型不对
makeInternalError       InternalError               内部异常
makeProtocolError       ProtocolError               协议层错误
makeSecurityError       SecurityError               安全相关错误
makeGenericError        GenericError                其他通用错误


#	错误码                         分类
1	NotImplemented              功能未实现
2	NotSupported                功能不支持
3	InternalError               内部错误
4	ProtocolError               协议错误
5	SecurityError               安全错误
6	FormationViolation          格式违反
7	PropertyConstraintViolation	属性约束违反
8	OccurrenceConstraintViolation	出现约束违反
9	TypeConstraintViolation     类型约束违反
10	GenericError                通用错误（兜底）
#endif







