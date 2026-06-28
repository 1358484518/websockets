#ifndef STATUSNOTIFICATION_H
#define STATUSNOTIFICATION_H

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

enum OcppChargePointStatus : quint8
{
    STATUS_AVAILABLE = 0,
    STATUS_PREPARING = 1,
    STATUS_CHARGING = 2,
    STATUS_SUSPENDED_EVSE = 3,
    STATUS_SUSPENDED_EV = 4,
    STATUS_FINISHING = 5,
    STATUS_RESERVED = 6,
    STATUS_UNAVAILABLE = 7,
    STATUS_FAULTED = 8
};

enum OcppChargePointErrorCode : quint8
{
    ERROR_NO_ERROR = 0,
    ERROR_CONNECTOR_LOCK_FAILURE = 1,
    ERROR_EV_COMMUNICATION_ERROR = 2,
    ERROR_GROUND_FAILURE = 3,
    ERROR_HIGH_TEMPERATURE = 4,
    ERROR_INTERNAL_ERROR = 5,
    ERROR_LOCAL_LIST_CONFLICT = 6,
    ERROR_OVER_CURRENT_FAILURE = 7,
    ERROR_POWER_METER_FAILURE = 8,
    ERROR_POWER_SWITCH_FAILURE = 9,
    ERROR_READER_FAILURE = 10,
    ERROR_RESET_FAILURE = 11,
    ERROR_UNDER_VOLTAGE = 12,
    ERROR_OVER_VOLTAGE = 13,
    ERROR_WEAK_SIGNAL = 14
};

class StatusNotification
{
public:
    StatusNotification();
    explicit StatusNotification(const char *value);
    explicit StatusNotification(cJSON *obj);
    ~StatusNotification();

    StatusNotification(const StatusNotification&) = delete;
    StatusNotification& operator=(const StatusNotification&) = delete;

    StatusNotification(StatusNotification&& other) noexcept;
    StatusNotification& operator=(StatusNotification&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);

    // Request setters
    bool setConnectorId(qint32 connectorId);
    bool setStatus(quint8 status);
    bool setErrorCode(quint8 errorCode);
    bool setInfo(const char *info);
    bool clearInfo();
    bool setTimestamp(const char *timestamp);
    bool clearTimestamp();
    bool setVendorId(const char *vendorId);
    bool clearVendorId();
    bool setVendorErrorCode(const char *vendorErrorCode);
    bool clearVendorErrorCode();

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    qint32 connectorId() const;
    quint8 status() const;
    const char* statusString() const;
    quint8 errorCode() const;
    const char* errorCodeString() const;
    const char* info() const;
    bool hasInfo() const;
    const char* timestamp() const;
    bool hasTimestamp() const;
    const char* vendorId() const;
    bool hasVendorId() const;
    const char* vendorErrorCode() const;
    bool hasVendorErrorCode() const;

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
    quint8  m_status;
    quint8  m_errorCode;

    bool ensurePayload();
    bool setStringField(const char *key, const char *value);
    bool clearField(const char *key);
    const char* getStringField(const char *key) const;
    bool hasField(const char *key) const;

    static const char* statusToString(quint8 status);
    static quint8 stringToStatus(const char *str);
    static const char* errorCodeToString(quint8 errorCode);
    static quint8 stringToErrorCode(const char *str);
};

#endif // STATUSNOTIFICATION_H
