#ifndef CHANGEAVAILABILITY_H
#define CHANGEAVAILABILITY_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"
//这包含 ChangeAvailability.conf PDU 按充电点返回到中央系统的字段定义
// Request type: Operative / Inoperative
enum OcppAvailabilityType : quint8 {
    AVAIL_OPERATIVE = 0, // 可用
    AVAIL_INOPERATIVE = 1// 不可用
};

// Response status: Accepted / Rejected / Scheduled
enum OcppChangeStatus : quint8 {
    CHANGE_ACCEPTED = 0,// 已接受
    CHANGE_REJECTED = 1,// 已拒绝
    CHANGE_SCHEDULED = 2// 已调度
};

class ChangeAvailability
{
public:
    ChangeAvailability();
    explicit ChangeAvailability(const char *value);
    explicit ChangeAvailability(cJSON *obj);
    ~ChangeAvailability();

    ChangeAvailability(const ChangeAvailability&) = delete;
    ChangeAvailability& operator=(const ChangeAvailability&) = delete;

    ChangeAvailability(ChangeAvailability&& other) noexcept;
    ChangeAvailability& operator=(ChangeAvailability&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);

    // Request setters
    bool setConnectorId(qint32 id);
    bool setAvailabilityType(quint8 type);

    // Response setters
    bool setStatus(quint8 s);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    qint32 connectorId() const;
    quint8 availabilityType() const;
    const char* availabilityTypeString() const;

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
    quint8  m_type;           // message type (CALL/CALLRESULT)
    quint8  m_availType;      // request: Operative/Inoperative
    quint8  m_status;         // response: Accepted/Rejected/Scheduled
    qint32  m_connectorId;

    const char* availTypeToString(quint8 t) const;
    quint8 stringToAvailType(const char *str) const;
    const char* statusToString(quint8 s) const;
    quint8 stringToStatus(const char *str) const;

    bool ensurePayload();
    bool setNumberField(const char *key, double value);
    bool setStringField(const char *key, const char *value);
    double getNumberField(const char *key) const;
    const char* getStringField(const char *key) const;
};

#endif // CHANGEAVAILABILITY_H
