#ifndef OCPPERROR_H
#define OCPPERROR_H

#include "cJSON.h"
#include "ocpp_error_constants.h"
// ===== OCPP Error Codes =====
enum class OcppErrorCode {
    NotImplemented,
    NotSupported,
    InternalError,
    ProtocolError,
    SecurityError,
    FormationViolation,
    PropertyConstraintViolation,
    OccurrenceConstraintViolation,
    TypeConstraintViolation,
    GenericError
};

class OcppError
{
public:
    // ===== Constructors =====
    OcppError();
    explicit OcppError(const char *jsonStr);
    OcppError(const char *msgId, const char *code, const char *desc);
    OcppError(const char *msgId, OcppErrorCode code, const char *desc = nullptr);
    ~OcppError();

    // ===== Disable copy =====
    OcppError(const OcppError&) = delete;
    OcppError& operator=(const OcppError&) = delete;

    // ===== Move semantics =====
    OcppError(OcppError&& other) noexcept;
    OcppError& operator=(OcppError&& other) noexcept;

    // ===== Create error manually =====
    void create(const char *msgId, const char *code, const char *desc);
    void create(const char *msgId, OcppErrorCode code, const char *desc = nullptr);

    // ===== Parse from JSON string =====
    bool parse(const char *jsonStr);

    // ===== Validation =====
    bool isValid() const;

    // ===== Getters =====
    const char* messageId() const;
    const char* errorCode() const;
    const char* description() const;
    cJSON* details() const;
    OcppErrorCode errorCodeEnum() const;

    // ===== Add error details =====
    void addDetail(const char *key, const char *value);
    void addDetail(const char *key, int value);

    // ===== Serialize =====
    char* toJson() const;

    // ===== Debug print =====
    void print() const;

    // ===== Clear / reset =====
    void clear();

    // ===== Static factory methods (常用错误快捷创建) =====
    static OcppError makeNotImplemented(const char *msgId, const char *action = nullptr);
    static OcppError makeNotSupported(const char *msgId, const char *action = nullptr);
    static OcppError makeFormationViolation(const char *msgId, const char *detail = nullptr);
    static OcppError makeOccurrenceViolation(const char *msgId, const char *field);
    static OcppError makePropertyViolation(const char *msgId, const char *field, const char *value = nullptr);
    static OcppError makeTypeViolation(const char *msgId, const char *field);
    static OcppError makeInternalError(const char *msgId, const char *detail = nullptr);
    static OcppError makeProtocolError(const char *msgId, const char *detail = nullptr);
    static OcppError makeSecurityError(const char *msgId, const char *detail = nullptr);
    static OcppError makeGenericError(const char *msgId, const char *detail = nullptr);

private:
    cJSON *m_root;

    static const char* codeToString(OcppErrorCode code);
    static const char* defaultDescription(OcppErrorCode code);
};

#endif // OCPPERROR_H
