#ifndef TRIGGERMESSAGE_H
#define TRIGGERMESSAGE_H

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

enum OcppTriggerStatus : quint8
{
    TRIGGER_ACCEPTED = 0,
    TRIGGER_REJECTED = 1,
    TRIGGER_NOT_IMPLEMENTED = 2
};

enum OcppRequestedMessage : quint8
{
    REQ_MSG_BOOT_NOTIFICATION = 0,
    REQ_MSG_DIAGNOSTICS_STATUS_NOTIFICATION = 1,
    REQ_MSG_FIRMWARE_STATUS_NOTIFICATION = 2,
    REQ_MSG_HEARTBEAT = 3,
    REQ_MSG_METER_VALUES = 4,
    REQ_MSG_STATUS_NOTIFICATION = 5
};

class TriggerMessage
{
public:
    TriggerMessage();
    explicit TriggerMessage(const char *value);
    explicit TriggerMessage(cJSON *obj);
    ~TriggerMessage();

    TriggerMessage(const TriggerMessage&) = delete;
    TriggerMessage& operator=(const TriggerMessage&) = delete;

    TriggerMessage(TriggerMessage&& other) noexcept;
    TriggerMessage& operator=(TriggerMessage&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);

    // Request setters
    bool setRequestedMessage(quint8 msgType);
    bool setConnectorId(qint32 connectorId);
    bool clearConnectorId();

    // Response setters
    bool setStatus(quint8 status);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    quint8 requestedMessage() const;
    const char* requestedMessageString() const;
    qint32 connectorId() const;
    bool hasConnectorId() const;

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
    quint8  m_requestedMessage;
    quint8  m_status;

    bool ensurePayload();

    static const char* statusToString(quint8 status);
    static quint8 stringToStatus(const char *str);
    static const char* requestedMessageToString(quint8 msgType);
    static quint8 stringToRequestedMessage(const char *str);
};

#endif // TRIGGERMESSAGE_H
