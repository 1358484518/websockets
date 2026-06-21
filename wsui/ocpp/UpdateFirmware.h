#ifndef UPDATEFIRMWARE_H
#define UPDATEFIRMWARE_H

#include "cJSON.h"
#include "Authorize.h"

//#ifdef QT_CORE_LIB
//#include <QtGlobal>
//#else
//typedef unsigned char quint8;
//typedef unsigned long long quint64;
//typedef int qint32;
//typedef double qreal;
//#endif

//enum OcppMsgType : quint8
//{
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

class UpdateFirmware
{
public:
    UpdateFirmware();
    explicit UpdateFirmware(const char *value);
    explicit UpdateFirmware(cJSON *obj);
    ~UpdateFirmware();

    UpdateFirmware(const UpdateFirmware&) = delete;
    UpdateFirmware& operator=(const UpdateFirmware&) = delete;

    UpdateFirmware(UpdateFirmware&& other) noexcept;
    UpdateFirmware& operator=(UpdateFirmware&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setLocation(const char *location);
    bool setRetries(qint32 retries);
    void clearRetries();
    bool setRetryInterval(qint32 interval);
    void clearRetryInterval();
    bool setRetrieveDate(const char *date);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    const char* location() const;
    bool hasRetries() const;
    qint32 retries() const;
    bool hasRetryInterval() const;
    qint32 retryInterval() const;
    const char* retrieveDate() const;

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
    void clearField(const char *key);
    const char* getStringField(const char *key) const;
    qreal getNumberField(const char *key) const;
    bool hasField(const char *key) const;
};

#endif // UPDATEFIRMWARE_H
