#ifndef AUTHORIZE_H
#define AUTHORIZE_H

#include <QString>
#include "cJSON.h"

// OCPP Message Type
enum OcppMsgType : quint8 {
    OCPP_CALL = 2,
    OCPP_CALLRESULT = 3,
    OCPP_CALLERROR = 4
};

// OCPP Authorization Status
enum OcppAuthStatus : quint8 {
    AUTH_ACCEPTED = 0,
    AUTH_BLOCKED = 1,
    AUTH_EXPIRED = 2,
    AUTH_INVALID = 3,
    AUTH_CONCURRENT_TX = 4
};

class Authorize
{
public:
    // ===== Constructors =====
    Authorize();
    explicit Authorize(const char *value);
    explicit Authorize(cJSON *obj);   // takes ownership
    ~Authorize();

    // ===== Disable copy =====
    Authorize(const Authorize&) = delete;
    Authorize& operator=(const Authorize&) = delete;

    // ===== Move semantics =====
    Authorize(Authorize&& other) noexcept;
    Authorize& operator=(Authorize&& other) noexcept;

    // ===== Build message =====
    bool buildReq();     // returns false on allocation failure
    bool buildConf();    // returns false on allocation failure

    // ===== Setters =====
    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);
    bool setState(quint8 s);

    bool setIdTag(const char *idTag);
    bool setExpiryDate(const char *date);
    bool setParentIdTag(const char *parentId);

    // ===== Getters =====
    quint64 msgSeq() const;
    quint8 type() const;
    quint8 state() const;

    const char* idTag() const;
    const char* status() const;
    const char* expiryDate() const;
    const char* parentIdTag() const;

    bool isCall() const;
    bool isCallResult() const;
    bool isValid() const;

    // ===== Parse =====
    bool parse(const char *value);
    bool parse(cJSON *obj);   // takes ownership on success

    // ===== Serialize =====
    char* toJson() const;

    // ===== Debug =====
    void print() const;

    // ===== Raw access =====
    cJSON *root() const { return m_root; }

    // ===== Clear / reset =====
    void clear();

private:
    cJSON  *m_root;
    quint8  m_type;
    quint8  m_status;

    const char* statusToString(quint8 s) const;
    quint8 stringToStatus(const char *str) const;

    bool ensurePayload();
    bool ensureIdTagInfo();
    bool setStringField(cJSON *obj, const char *key, const char *value);
};

#endif // AUTHORIZE_H
