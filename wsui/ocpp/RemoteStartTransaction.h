#ifndef REMOTESTARTTRANSACTION_H
#define REMOTESTARTTRANSACTION_H

#include "cJSON.h"
#include "Authorize.h"

//enum OcppMsgType : quint8 {
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

enum OcppRemoteStartStatus : quint8 {
    REMOTE_START_ACCEPTED = 0,
    REMOTE_START_REJECTED = 1
};

class RemoteStartTransaction
{
public:
    RemoteStartTransaction();
    explicit RemoteStartTransaction(const char *value);
    explicit RemoteStartTransaction(cJSON *obj);
    ~RemoteStartTransaction();

    RemoteStartTransaction(const RemoteStartTransaction&) = delete;
    RemoteStartTransaction& operator=(const RemoteStartTransaction&) = delete;

    RemoteStartTransaction(RemoteStartTransaction&& other) noexcept;
    RemoteStartTransaction& operator=(RemoteStartTransaction&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);

    // Request setters
    bool setConnectorId(qint32 connectorId);
    bool clearConnectorId();
    bool setIdTag(const char *idTag);

    // ChargingProfile setters (request)
    bool setChargingProfileId(qint32 id);
    bool setChargingProfileTransactionId(qint32 id);
    bool clearChargingProfileTransactionId();
    bool setStackLevel(qint32 level);
    bool setChargingProfilePurpose(const char *purpose);
    bool setChargingProfileKind(const char *kind);
    bool setRecurrencyKind(const char *kind);
    bool clearRecurrencyKind();
    bool setValidFrom(const char *time);
    bool clearValidFrom();
    bool setValidTo(const char *time);
    bool clearValidTo();

    // ChargingSchedule setters (inside chargingProfile)
    bool setChargingScheduleDuration(qint32 duration);
    bool clearChargingScheduleDuration();
    bool setStartSchedule(const char *time);
    bool clearStartSchedule();
    bool setChargingRateUnit(const char *unit);
    bool setMinChargingRate(qreal rate);
    bool clearMinChargingRate();

    // Schedule period operations
    int addSchedulePeriod(qint32 startPeriod, qreal limit, qint32 numberPhases = 0);
    bool clearSchedulePeriods();

    // Response setters
    bool setStatus(quint8 status);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    qint32 connectorId() const;
    bool hasConnectorId() const;
    const char* idTag() const;

    // ChargingProfile getters
    qint32 chargingProfileId() const;
    qint32 chargingProfileTransactionId() const;
    bool hasChargingProfileTransactionId() const;
    qint32 stackLevel() const;
    const char* chargingProfilePurpose() const;
    const char* chargingProfileKind() const;
    const char* recurrencyKind() const;
    const char* validFrom() const;
    const char* validTo() const;
    bool hasChargingProfile() const;

    // ChargingSchedule getters
    qint32 chargingScheduleDuration() const;
    bool hasChargingScheduleDuration() const;
    const char* startSchedule() const;
    const char* chargingRateUnit() const;
    qreal minChargingRate() const;
    bool hasMinChargingRate() const;
    int schedulePeriodCount() const;
    bool getSchedulePeriod(int index, qint32 &startPeriod, qreal &limit, qint32 &numberPhases) const;

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
    cJSON* getChargingProfile() const;
    cJSON* ensureChargingProfile();
    cJSON* getChargingSchedule() const;
    cJSON* ensureChargingSchedule();
    cJSON* getSchedulePeriodArray() const;
    cJSON* ensureSchedulePeriodArray();

    static const char* statusToString(quint8 status);
    static quint8 stringToStatus(const char *str);
};

#endif // REMOTESTARTTRANSACTION_H
