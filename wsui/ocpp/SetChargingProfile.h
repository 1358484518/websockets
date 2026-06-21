#ifndef SETCHARGINGPROFILE_H
#define SETCHARGINGPROFILE_H

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

// SetChargingProfile 响应状态
enum OcppSetProfileStatus : quint8
{
    SET_PROFILE_ACCEPTED = 0,
    SET_PROFILE_REJECTED = 1,
    SET_PROFILE_NOT_SUPPORTED = 2
};

// chargingProfilePurpose
#define PROFILE_PURPOSE_CHARGEPOINT_MAX "ChargePointMaxProfile"
#define PROFILE_PURPOSE_TX_DEFAULT "TxDefaultProfile"
#define PROFILE_PURPOSE_TX "TxProfile"

// chargingProfileKind
#define PROFILE_KIND_ABSOLUTE "Absolute"
#define PROFILE_KIND_RECURRING "Recurring"
#define PROFILE_KIND_RELATIVE "Relative"

// recurrencyKind
#define RECUR_DAILY "Daily"
#define RECUR_WEEKLY "Weekly"

// chargingRateUnit
#define RATE_UNIT_A "A"
#define RATE_UNIT_W "W"

class SetChargingProfile
{
public:
    SetChargingProfile();
    explicit SetChargingProfile(const char *value);
    explicit SetChargingProfile(cJSON *obj);
    ~SetChargingProfile();

    SetChargingProfile(const SetChargingProfile&) = delete;
    SetChargingProfile& operator=(const SetChargingProfile&) = delete;

    SetChargingProfile(SetChargingProfile&& other) noexcept;
    SetChargingProfile& operator=(SetChargingProfile&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // ========== Request Setters ==========
    bool setConnectorId(qint32 id);

    // chargingProfile 顶层字段
    bool setChargingProfileId(qint32 id);
    bool setTransactionId(qint32 txId);
    bool clearTransactionId();
    bool setStackLevel(qint32 level);
    bool setChargingProfilePurpose(const char *purpose);
    bool setChargingProfileKind(const char *kind);
    bool setRecurrencyKind(const char *recur);
    bool clearRecurrencyKind();
    bool setValidFrom(const char *time);
    bool clearValidFrom();
    bool setValidTo(const char *time);
    bool clearValidTo();

    // chargingSchedule 字段
    bool setScheduleDuration(qint32 sec);
    bool clearScheduleDuration();
    bool setStartSchedule(const char *isoTime);
    bool clearStartSchedule();
    bool setChargingRateUnit(const char *unit);
    bool setMinChargingRate(qreal rate);
    bool clearMinChargingRate();

    // chargingSchedulePeriod 数组操作
    int addSchedulePeriod(qint32 startPeriod, qreal limit, qint32 numberPhases = 0);
    bool clearSchedulePeriods();

    // ========== Response Setters ==========
    bool setStatus(quint8 status);

    // ========== Getters ==========
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    qint32 connectorId() const;

    qint32 chargingProfileId() const;
    qint32 transactionId() const;
    bool hasTransactionId() const;
    qint32 stackLevel() const;
    const char* chargingProfilePurpose() const;
    const char* chargingProfileKind() const;
    const char* recurrencyKind() const;
    const char* validFrom() const;
    const char* validTo() const;
    bool hasChargingProfile() const;

    qint32 scheduleDuration() const;
    bool hasScheduleDuration() const;
    const char* startSchedule() const;
    const char* chargingRateUnit() const;
    qreal minChargingRate() const;
    bool hasMinChargingRate() const;

    int schedulePeriodCount() const;
    bool getSchedulePeriod(int idx, qint32 &startPeriod, qreal &limit, qint32 &numberPhases) const;

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
    quint8  m_respStatus;

    bool ensurePayload();
    cJSON* ensureChargingProfile();
    cJSON* getChargingProfile() const;
    cJSON* ensureChargingSchedule();
    cJSON* getChargingSchedule() const;
    cJSON* ensureSchedulePeriodArray();
    cJSON* getSchedulePeriodArray() const;

    static const char* statusToString(quint8 s);
    static quint8 stringToStatus(const char *s);
};

#endif // SETCHARGINGPROFILE_H
