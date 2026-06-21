#ifndef CANCELRESERVATION_H
#define CANCELRESERVATION_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"

enum OcppCancelStatus : quint8 {
    CANCEL_ACCEPTED = 0,
    CANCEL_REJECTED = 1
};

class CancelReservation
{
public:
    CancelReservation();
    explicit CancelReservation(const char *value);
    explicit CancelReservation(cJSON *obj);
    ~CancelReservation();

    CancelReservation(const CancelReservation&) = delete;
    CancelReservation& operator=(const CancelReservation&) = delete;

    CancelReservation(CancelReservation&& other) noexcept;
    CancelReservation& operator=(CancelReservation&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setReservationId(qint32 id);

    // Response setters
    bool setStatus(quint8 s);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    qint32 reservationId() const;

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
    qint32  m_reservationId;

    const char* statusToString(quint8 s) const;
    quint8 stringToStatus(const char *str) const;

    bool ensurePayload();
    bool setNumberField(const char *key, double value);
    bool setStringField(const char *key, const char *value);
    double getNumberField(const char *key) const;
    const char* getStringField(const char *key) const;
};

#endif // CANCELRESERVATION_H
