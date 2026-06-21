#ifndef RESERVENOW_H
#define RESERVENOW_H

#include "cJSON.h"
#include "Authorize.h"
// Qt type definitions (use QtGlobal in Qt project, define manually for standalone)

//enum OcppMsgType : quint8 {
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

enum OcppReserveStatus : quint8 {
    RESERVE_ACCEPTED = 0,
    RESERVE_FAULTED = 1,
    RESERVE_OCCUPIED = 2,
    RESERVE_REJECTED = 3,
    RESERVE_UNAVAILABLE = 4
};

class ReserveNow
{
public:
    ReserveNow();
    explicit ReserveNow(const char *value);
    explicit ReserveNow(cJSON *obj);
    ~ReserveNow();

    ReserveNow(const ReserveNow&) = delete;
    ReserveNow& operator=(const ReserveNow&) = delete;

    ReserveNow(ReserveNow&& other) noexcept;
    ReserveNow& operator=(ReserveNow&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setConnectorId(qint32 connectorId);
    bool setExpiryDate(const char *expiryDate);
    bool setIdTag(const char *idTag);
    bool setReservationId(qint32 reservationId);
    bool setParentIdTag(const char *parentIdTag);
    bool clearParentIdTag();

    // Response setters
    bool setStatus(quint8 status);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    qint32 connectorId() const;
    const char* expiryDate() const;
    const char* idTag() const;
    qint32 reservationId() const;
    const char* parentIdTag() const;
    bool hasParentIdTag() const;

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
    bool setStringField(const char *key, const char *value);
    bool setNumberField(const char *key, qreal value);
    bool clearField(const char *key);
    const char* getStringField(const char *key) const;
    qreal getNumberField(const char *key) const;
    bool hasField(const char *key) const;

    static const char* statusToString(quint8 status);
    static quint8 stringToStatus(const char *str);
};

#endif // RESERVENOW_H
