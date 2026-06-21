#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "cJSON.h"
#include "Authorize.h"

//enum OcppMsgType : quint8 {
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

class Heartbeat
{
public:
    Heartbeat();
    explicit Heartbeat(const char *value);
    explicit Heartbeat(cJSON *obj);
    ~Heartbeat();

    Heartbeat(const Heartbeat&) = delete;
    Heartbeat& operator=(const Heartbeat&) = delete;

    Heartbeat(Heartbeat&& other) noexcept;
    Heartbeat& operator=(Heartbeat&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Response setters
    bool setCurrentTime(const char *time);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Response getters
    const char* currentTime() const;

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
};

#endif // HEARTBEAT_H
