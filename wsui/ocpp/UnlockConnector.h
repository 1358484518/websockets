#ifndef UNLOCKCONNECTOR_H
#define UNLOCKCONNECTOR_H

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

enum OcppUnlockStatus : quint8
{
    UNLOCK_UNLOCKED = 0,
    UNLOCK_UNLOCK_FAILED = 1,
    UNLOCK_NOT_SUPPORTED = 2
};

class UnlockConnector
{
public:
    UnlockConnector();
    explicit UnlockConnector(const char *value);
    explicit UnlockConnector(cJSON *obj);
    ~UnlockConnector();

    UnlockConnector(const UnlockConnector&) = delete;
    UnlockConnector& operator=(const UnlockConnector&) = delete;

    UnlockConnector(UnlockConnector&& other) noexcept;
    UnlockConnector& operator=(UnlockConnector&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);

    // Request setters
    bool setConnectorId(qint32 connectorId);

    // Response setters
    bool setStatus(quint8 status);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    qint32 connectorId() const;

    // Response getters
    quint8 status() const;
    const char* statusString() const;

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

    bool ensurePayload();

    static const char* statusToString(quint8 status);
    static quint8 stringToStatus(const char *str);
};

#endif // UNLOCKCONNECTOR_H
