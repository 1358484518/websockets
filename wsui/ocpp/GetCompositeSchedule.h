#ifndef GETCOMPOSITESCHEDULE_H
#define GETCOMPOSITESCHEDULE_H

#include "cJSON.h"
#include "Authorize.h"

//enum OcppMsgType : quint8 {
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

enum OcppCompositeStatus : quint8 {
    COMPOSITE_ACCEPTED = 0,
    COMPOSITE_REJECTED = 1
};

enum OcppChargingRateUnit : quint8 {
    RATE_UNIT_W = 0,
    RATE_UNIT_A = 1
};

class GetCompositeSchedule
{
public:
    GetCompositeSchedule();
    explicit GetCompositeSchedule(const char *value);
    explicit GetCompositeSchedule(cJSON *obj);
    ~GetCompositeSchedule();

    GetCompositeSchedule(const GetCompositeSchedule&) = delete;
    GetCompositeSchedule& operator=(const GetCompositeSchedule&) = delete;

    GetCompositeSchedule(GetCompositeSchedule&& other) noexcept;
    GetCompositeSchedule& operator=(GetCompositeSchedule&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);

    // Request setters
    bool setConnectorId(qint32 id);
    bool setDuration(qint32 seconds);
    bool setChargingRateUnit(quint8 unit);

    // Response setters
    bool setStatus(quint8 s);
    bool setScheduleStart(const char *time);
    bool setConfConnectorId(qint32 id);

    // Charging schedule setters (response only)
    bool setChargingScheduleDuration(qint32 seconds);
    bool setChargingScheduleStart(const char *time);
    bool setChargingScheduleUnit(quint8 unit);
    bool setMinChargingRate(qreal rate);
    bool addSchedulePeriod(qint32 startPeriod, qreal limit, qint32 numberPhases = 0);
    bool clearSchedulePeriods();

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    qint32 connectorId() const;
    qint32 duration() const;
    quint8 chargingRateUnit() const;
    const char* chargingRateUnitString() const;

    quint8 status() const;
    const char* statusString() const;
    const char* scheduleStart() const;
    qint32 confConnectorId() const;

    qint32 scheduleDuration() const;
    const char* scheduleStartTime() const;
    quint8 scheduleUnit() const;
    const char* scheduleUnitString() const;
    qreal minChargingRate() const;
    int schedulePeriodCount() const;
    bool getSchedulePeriod(int index, qint32 &startPeriod, qreal &limit, qint32 &numberPhases) const;

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
    qint32  m_connectorId;
    qint32  m_duration;
    quint8  m_rateUnit;

    const char* statusToString(quint8 s) const;
    quint8 stringToStatus(const char *str) const;

    const char* unitToString(quint8 u) const;
    quint8 stringToUnit(const char *str) const;

    bool ensurePayload();
    bool ensureChargingSchedule();
};

#endif // GETCOMPOSITESCHEDULE_H
