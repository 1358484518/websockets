#ifndef OCPP_ERROR_CONSTANTS_H
#define OCPP_ERROR_CONSTANTS_H


// ============================================================
// OCPP Error Code Strings
// ============================================================

#define OCPP_ERR_NOT_IMPLEMENTED          "NotImplemented"
#define OCPP_ERR_NOT_SUPPORTED            "NotSupported"
#define OCPP_ERR_INTERNAL_ERROR           "InternalError"
#define OCPP_ERR_PROTOCOL_ERROR           "ProtocolError"
#define OCPP_ERR_SECURITY_ERROR           "SecurityError"
#define OCPP_ERR_FORMATION_VIOLATION      "FormationViolation"
#define OCPP_ERR_PROPERTY_VIOLATION       "PropertyConstraintViolation"
#define OCPP_ERR_OCCURRENCE_VIOLATION     "OccurrenceConstraintViolation"
#define OCPP_ERR_TYPE_VIOLATION           "TypeConstraintViolation"
#define OCPP_ERR_GENERIC_ERROR            "GenericError"

// ============================================================
// Default Error Descriptions
// ============================================================

#define OCPP_ERR_DESC_NOT_IMPLEMENTED     "Requested action is not implemented"
#define OCPP_ERR_DESC_NOT_SUPPORTED       "Requested action is not supported"
#define OCPP_ERR_DESC_INTERNAL_ERROR      "Internal error"
#define OCPP_ERR_DESC_PROTOCOL_ERROR      "Protocol error"
#define OCPP_ERR_DESC_SECURITY_ERROR      "Security error"
#define OCPP_ERR_DESC_FORMATION_VIOLATION "Message formation violation"
#define OCPP_ERR_DESC_PROPERTY_VIOLATION  "Property constraint violation"
#define OCPP_ERR_DESC_OCCURRENCE_VIOLATION "Occurrence constraint violation"
#define OCPP_ERR_DESC_TYPE_VIOLATION      "Type constraint violation"
#define OCPP_ERR_DESC_GENERIC_ERROR       "Generic error"

// ============================================================
// Complete Error Message Templates (ready to use)
// 用法: QString::fromUtf8(OCPP_ERR_TPL_NOT_IMPLEMENTED).arg(msgId).arg(action)
// ============================================================

// [4, msgId, NotImplemented, desc, {action: xxx}]
#define OCPP_ERR_TPL_NOT_IMPLEMENTED \
    "[4,\"%1\",\"NotImplemented\",\"Requested action is not implemented\",{\"action\":\"%2\"}]"

// [4, msgId, NotSupported, desc, {action: xxx}]
#define OCPP_ERR_TPL_NOT_SUPPORTED \
    "[4,\"%1\",\"NotSupported\",\"Requested action is not supported\",{\"action\":\"%2\"}]"

// [4, msgId, FormationViolation, desc, {detail: xxx}]
#define OCPP_ERR_TPL_FORMATION_VIOLATION \
    "[4,\"%1\",\"FormationViolation\",\"Message formation violation\",{\"detail\":\"%2\"}]"

// [4, msgId, OccurrenceConstraintViolation, desc, {field: xxx}]
#define OCPP_ERR_TPL_OCCURRENCE_VIOLATION \
    "[4,\"%1\",\"OccurrenceConstraintViolation\",\"Occurrence constraint violation\",{\"field\":\"%2\"}]"

// [4, msgId, PropertyConstraintViolation, desc, {field: xxx, value: xxx}]
#define OCPP_ERR_TPL_PROPERTY_VIOLATION \
    "[4,\"%1\",\"PropertyConstraintViolation\",\"Property constraint violation\",{\"field\":\"%2\",\"value\":\"%3\"}]"

// [4, msgId, TypeConstraintViolation, desc, {field: xxx}]
#define OCPP_ERR_TPL_TYPE_VIOLATION \
    "[4,\"%1\",\"TypeConstraintViolation\",\"Type constraint violation\",{\"field\":\"%2\"}]"

// [4, msgId, InternalError, desc, {detail: xxx}]
#define OCPP_ERR_TPL_INTERNAL_ERROR \
    "[4,\"%1\",\"InternalError\",\"Internal error\",{\"detail\":\"%2\"}]"

// [4, msgId, ProtocolError, desc, {detail: xxx}]
#define OCPP_ERR_TPL_PROTOCOL_ERROR \
    "[4,\"%1\",\"ProtocolError\",\"Protocol error\",{\"detail\":\"%2\"}]"

// [4, msgId, SecurityError, desc, {detail: xxx}]
#define OCPP_ERR_TPL_SECURITY_ERROR \
    "[4,\"%1\",\"SecurityError\",\"Security error\",{\"detail\":\"%2\"}]"

// [4, msgId, GenericError, desc, {detail: xxx}]
#define OCPP_ERR_TPL_GENERIC_ERROR \
    "[4,\"%1\",\"GenericError\",\"Generic error\",{\"detail\":\"%2\"}]"

// ============================================================
// Static Examples (fixed msgId, for testing)
// ============================================================

#define OCPP_ERR_EXAMPLE_NOT_IMPLEMENTED \
    "[4,\"msg_001\",\"NotImplemented\",\"Requested action is not implemented\",{\"action\":\"DataTransfer\"}]"

#define OCPP_ERR_EXAMPLE_NOT_SUPPORTED \
    "[4,\"msg_002\",\"NotSupported\",\"Requested action is not supported\",{\"action\":\"ReserveNow\"}]"

#define OCPP_ERR_EXAMPLE_FORMATION_VIOLATION \
    "[4,\"msg_003\",\"FormationViolation\",\"Message formation violation\",{\"detail\":\"Invalid JSON syntax\"}]"

#define OCPP_ERR_EXAMPLE_OCCURRENCE_VIOLATION \
    "[4,\"msg_004\",\"OccurrenceConstraintViolation\",\"Occurrence constraint violation\",{\"field\":\"idTag\"}]"

#define OCPP_ERR_EXAMPLE_PROPERTY_VIOLATION \
    "[4,\"msg_005\",\"PropertyConstraintViolation\",\"Property constraint violation\",{\"field\":\"connectorId\",\"value\":\"999\"}]"

#define OCPP_ERR_EXAMPLE_TYPE_VIOLATION \
    "[4,\"msg_006\",\"TypeConstraintViolation\",\"Type constraint violation\",{\"field\":\"connectorId\"}]"

#define OCPP_ERR_EXAMPLE_INTERNAL_ERROR \
    "[4,\"msg_007\",\"InternalError\",\"Internal error\",{\"detail\":\"Database connection failed\"}]"

#define OCPP_ERR_EXAMPLE_PROTOCOL_ERROR \
    "[4,\"msg_008\",\"ProtocolError\",\"Protocol error\",{\"detail\":\"MessageId not found\"}]"

#define OCPP_ERR_EXAMPLE_SECURITY_ERROR \
    "[4,\"msg_009\",\"SecurityError\",\"Security error\",{\"detail\":\"Unauthorized access\"}]"

#define OCPP_ERR_EXAMPLE_GENERIC_ERROR \
    "[4,\"msg_010\",\"GenericError\",\"Generic error\",{\"detail\":\"Something went wrong\"}]"

#if 0
//使用示例
#include "ocpp_error_constants.h"
#include <QString>

// 生成 NotImplemented 错误
QString err = QString::fromUtf8(OCPP_ERR_TPL_NOT_IMPLEMENTED)
                  .arg("msg_123")
                  .arg("UnknownAction");

sendWebSocket(err.toUtf8().constData());
/******************************************************/
// 生成字段缺失错误
QString err = QString::fromUtf8(OCPP_ERR_TPL_OCCURRENCE_VIOLATION)
                  .arg("msg_456")
                  .arg("idTag");
/******************************************************/
// 生成字段值非法错误
QString err = QString::fromUtf8(OCPP_ERR_TPL_PROPERTY_VIOLATION)
                  .arg("msg_789")
                  .arg("connectorId")
                  .arg("0");
/******************************************************/
OcppError err(receivedJson);

if (strcmp(err.errorCode(), OCPP_ERR_NOT_IMPLEMENTED) == 0) {
    // 处理未实现错误
} else if (strcmp(err.errorCode(), OCPP_ERR_FORMATION_VIOLATION) == 0) {
    // 处理格式错误
}
/******************************************************/

/******************************************************/

/******************************************************/

#endif

#endif // OCPP_ERROR_CONSTANTS_H
