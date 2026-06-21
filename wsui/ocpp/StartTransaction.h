#ifndef STARTTRANSACTION_H
#define STARTTRANSACTION_H

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

enum OcppIdTagStatus : quint8
{
    IDTAG_ACCEPTED = 0,
    IDTAG_BLOCKED = 1,
    IDTAG_EXPIRED = 2,
    IDTAG_INVALID = 3,
    IDTAG_CONCURRENT_TX = 4
};

class StartTransaction
{
public:
    StartTransaction();
    explicit StartTransaction(const char *value);
    explicit StartTransaction(cJSON *obj);
    ~StartTransaction();

    StartTransaction(const StartTransaction&) = delete;
    StartTransaction& operator=(const StartTransaction&) = delete;

    StartTransaction(StartTransaction&& other) noexcept;
    StartTransaction& operator=(StartTransaction&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setConnectorId(qint32 connectorId);
    bool setIdTag(const char *idTag);
    bool setMeterStart(qint32 meterStart);
    bool setTimestamp(const char *timestamp);
    bool setReservationId(qint32 reservationId);
    bool clearReservationId();

    // Response setters
    bool setTransactionId(qint32 transactionId);
    bool setIdTagStatus(quint8 status);
    bool setIdTagExpiryDate(const char *expiryDate);
    bool clearIdTagExpiryDate();
    bool setIdTagParentIdTag(const char *parentIdTag);
    bool clearIdTagParentIdTag();

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    qint32 connectorId() const;
    const char* idTag() const;
    qint32 meterStart() const;
    const char* timestamp() const;
    qint32 reservationId() const;
    bool hasReservationId() const;

    // Response getters
    qint32 transactionId() const;
    quint8 idTagStatus() const;
    const char* idTagStatusString() const;
    const char* idTagExpiryDate() const;
    bool hasIdTagExpiryDate() const;
    const char* idTagParentIdTag() const;
    bool hasIdTagParentIdTag() const;

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
    quint8  m_idTagStatus;

    bool ensurePayload();
    cJSON* getIdTagInfo() const;
    cJSON* ensureIdTagInfo();

    static const char* statusToString(quint8 status);
    static quint8 stringToStatus(const char *str);
};

#endif // STARTTRANSACTION_H
