#ifndef METERVALUES_H
#define METERVALUES_H

#include "cJSON.h"
#include "Authorize.h"

//enum OcppMsgType : quint8 {
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

class MeterValues
{
public:
    MeterValues();
    explicit MeterValues(const char *value);
    explicit MeterValues(cJSON *obj);
    ~MeterValues();

    MeterValues(const MeterValues&) = delete;
    MeterValues& operator=(const MeterValues&) = delete;

    MeterValues(MeterValues&& other) noexcept;
    MeterValues& operator=(MeterValues&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);

    // Request setters
    bool setConnectorId(qint32 connectorId);
    bool setTransactionId(qint32 transactionId);
    bool clearTransactionId();

    // Meter value operations
    int addMeterValue(const char *timestamp);
    bool addSampledValue(int meterIndex, const char *value,
                         const char *context = nullptr,
                         const char *format = nullptr,
                         const char *measurand = nullptr,
                         const char *phase = nullptr,
                         const char *location = nullptr,
                         const char *unit = nullptr);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    qint32 connectorId() const;
    qint32 transactionId() const;
    bool hasTransactionId() const;
    int meterValueCount() const;
    const char* meterValueTimestamp(int index) const;
    int sampledValueCount(int meterIndex) const;
    bool getSampledValue(int meterIndex, int sampleIndex,
                         const char *&value,
                         const char *&context,
                         const char *&format,
                         const char *&measurand,
                         const char *&phase,
                         const char *&location,
                         const char *&unit) const;

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
    cJSON* getMeterValueArray() const;
    cJSON* getSampledValueArray(cJSON *meterValue) const;
};

#endif // METERVALUES_H
