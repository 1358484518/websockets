#ifndef GETDIAGNOSTICS_H
#define GETDIAGNOSTICS_H

#include "cJSON.h"
#include "Authorize.h"
//enum OcppMsgType : quint8 {
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

class GetDiagnostics
{
public:
    GetDiagnostics();
    explicit GetDiagnostics(const char *value);
    explicit GetDiagnostics(cJSON *obj);
    ~GetDiagnostics();

    GetDiagnostics(const GetDiagnostics&) = delete;
    GetDiagnostics& operator=(const GetDiagnostics&) = delete;

    GetDiagnostics(GetDiagnostics&& other) noexcept;
    GetDiagnostics& operator=(GetDiagnostics&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setLocation(const char *location);
    bool setRetries(qint32 retries);
    bool clearRetries();
    bool setRetryInterval(qint32 interval);
    bool clearRetryInterval();
    bool setStartTime(const char *time);
    bool setStopTime(const char *time);

    // Response setters
    bool setFileName(const char *fileName);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    const char* location() const;
    qint32 retries() const;
    bool hasRetries() const;
    qint32 retryInterval() const;
    bool hasRetryInterval() const;
    const char* startTime() const;
    const char* stopTime() const;

    // Response getters
    const char* fileName() const;

    bool isCall() const;
    bool isCallResult() const;
    bool isValid() const;

    bool parse(const char *value);
    bool parse(cJSON *obj);

    char* toJson() const;
    void print() const;
    void clear();

    cJSON *root() const { return m_root; }

private:
    cJSON  *m_root;
    quint8  m_type;

    bool ensurePayload();
    bool setStringField(const char *key, const char *value);
    bool setNumberField(const char *key, qreal value);
    bool clearField(const char *key);
    const char* getStringField(const char *key) const;
    qreal getNumberField(const char *key) const;
    bool hasField(const char *key) const;
};

#endif // GETDIAGNOSTICS_H
