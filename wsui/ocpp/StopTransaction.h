#ifndef STOPTRANSACTION_H
#define STOPTRANSACTION_H

#include "cJSON.h"
//#include "Authorize.h"
#include "OcppAllMessages.h"

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


enum OcppStopReason : quint8
{
    STOP_REASON_EMERGENCY_STOP = 0,
    STOP_REASON_EV_DISCONNECTED = 1,
    STOP_REASON_HARD_RESET = 2,
    STOP_REASON_LOCAL = 3,
    STOP_REASON_OTHER = 4,
    STOP_REASON_POWER_LOSS = 5,
    STOP_REASON_REBOOT = 6,
    STOP_REASON_REMOTE = 7,
    STOP_REASON_SOFT_RESET = 8,
    STOP_REASON_UNLOCK_COMMAND = 9,
    STOP_REASON_DEAUTHORIZED = 10
};

class StopTransaction
{
public:
    StopTransaction();
    explicit StopTransaction(const char *value);
    explicit StopTransaction(cJSON *obj);
    ~StopTransaction();

    StopTransaction(const StopTransaction&) = delete;
    StopTransaction& operator=(const StopTransaction&) = delete;

    StopTransaction(StopTransaction&& other) noexcept;
    StopTransaction& operator=(StopTransaction&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setIdTag(const char *idTag);
    bool clearIdTag();
    bool setMeterStop(qint32 meterStop);
    bool setTimestamp(const char *timestamp);
    bool setTransactionId(qint32 transactionId);
    bool setReason(quint8 reason);
    bool clearReason();

    // TransactionData operations (request)
    int addTransactionData(const char *timestamp);
    int addSampledValue(int dataIndex, const char *value,
                        const char *context = nullptr, const char *format = nullptr,
                        const char *measurand = nullptr, const char *phase = nullptr,
                        const char *location = nullptr, const char *unit = nullptr);
    void clearTransactionData();

    // Response setters
    bool setIdTagStatus(quint8 status);
    bool setIdTagExpiryDate(const char *expiryDate);
    bool clearIdTagExpiryDate();
    bool setIdTagParentIdTag(const char *parentIdTag);
    bool clearIdTagParentIdTag();

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    const char* idTag() const;
    bool hasIdTag() const;
    qint32 meterStop() const;
    const char* timestamp() const;
    qint32 transactionId() const;
    quint8 reason() const;
    const char* reasonString() const;
    bool hasReason() const;

    // TransactionData getters
    int transactionDataCount() const;
    const char* transactionDataTimestamp(int index) const;
    int sampledValueCount(int dataIndex) const;
    bool getSampledValue(int dataIndex, int sampleIndex,
                         const char *&value, const char *&context,
                         const char *&format, const char *&measurand,
                         const char *&phase, const char *&location,
                         const char *&unit) const;

    // Response getters
    quint8 idTagStatus() const;
    const char* idTagStatusString() const;
    const char* idTagExpiryDate() const;
    bool hasIdTagExpiryDate() const;
    const char* idTagParentIdTag() const;
    bool hasIdTagParentIdTag() const;
    bool hasIdTagInfo() const;

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
    quint8  m_reason;
    bool    m_hasReason;

    bool ensurePayload();
    cJSON* getIdTagInfo() const;
    cJSON* ensureIdTagInfo();
    cJSON* getTransactionDataArray() const;
    cJSON* ensureTransactionDataArray();

    static const char* statusToString(quint8 status);
    static quint8 stringToStatus(const char *str);
    static const char* reasonToString(quint8 reason);
    static quint8 stringToReason(const char *str);
};

#endif // STOPTRANSACTION_H
