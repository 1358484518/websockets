#ifndef REMOTESTOPTRANSACTION_H
#define REMOTESTOPTRANSACTION_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"

//enum OcppMsgType : quint8 {
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

enum OcppRemoteStopStatus : quint8 {
    REMOTE_STOP_ACCEPTED = 0,
    REMOTE_STOP_REJECTED = 1
};

class RemoteStopTransaction
{
public:
    RemoteStopTransaction();
    explicit RemoteStopTransaction(const char *value);
    explicit RemoteStopTransaction(cJSON *obj);
    ~RemoteStopTransaction();

    RemoteStopTransaction(const RemoteStopTransaction&) = delete;
    RemoteStopTransaction& operator=(const RemoteStopTransaction&) = delete;

    RemoteStopTransaction(RemoteStopTransaction&& other) noexcept;
    RemoteStopTransaction& operator=(RemoteStopTransaction&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setTransactionId(qint32 transactionId);

    // Response setters
    bool setStatus(quint8 status);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    qint32 transactionId() const;

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

#endif // REMOTESTOPTRANSACTION_H
