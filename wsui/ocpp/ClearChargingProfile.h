#ifndef CLEARCHARGINGPROFILE_H
#define CLEARCHARGINGPROFILE_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"

enum OcppChargingProfilePurpose : quint8 {
    CHARGE_POINT_MAX_PROFILE = 0,
    TX_DEFAULT_PROFILE = 1,
    TX_PROFILE = 2
};

enum OcppClearProfileStatus : quint8 {
    CLEAR_PROFILE_ACCEPTED = 0,
    CLEAR_PROFILE_UNKNOWN = 1
};
//ClearChargingProfile 是智能充电功能的一部分，
//用于中央系统动态管理充电桩的充电功率限制策略。所有请求字段都是可选的，
//可以组合使用来精确筛选要清除的配置文件；如果都不传，则清除所有配置文件。
class ClearChargingProfile
{
public:
    ClearChargingProfile();
    explicit ClearChargingProfile(const char *value);
    explicit ClearChargingProfile(cJSON *obj);
    ~ClearChargingProfile();

    ClearChargingProfile(const ClearChargingProfile&) = delete;
    ClearChargingProfile& operator=(const ClearChargingProfile&) = delete;

    ClearChargingProfile(ClearChargingProfile&& other) noexcept;
    ClearChargingProfile& operator=(ClearChargingProfile&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters (all optional)
    bool setId(qint32 id);
    bool setConnectorId(qint32 id);
    bool setChargingProfilePurpose(quint8 purpose);

    // Response setters
    bool setStatus(quint8 s);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    qint32 id() const;
    qint32 connectorId() const;
    quint8 chargingProfilePurpose() const;
    const char* chargingProfilePurposeString() const;

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
    qint32  m_id;
    qint32  m_connectorId;
    quint8  m_purpose;

    const char* purposeToString(quint8 p) const;
    quint8 stringToPurpose(const char *str) const;
    const char* statusToString(quint8 s) const;
    quint8 stringToStatus(const char *str) const;

    bool ensurePayload();
    bool setNumberField(const char *key, double value);
    bool setStringField(const char *key, const char *value);
    double getNumberField(const char *key) const;
    const char* getStringField(const char *key) const;
    bool hasField(const char *key) const;
};

#endif // CLEARCHARGINGPROFILE_H
