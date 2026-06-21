#ifndef GETLOCALLISTVERSION_H
#define GETLOCALLISTVERSION_H

#include "cJSON.h"
#include "Authorize.h"
//enum OcppMsgType : quint8 {
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

class GetLocalListVersion
{
public:
    GetLocalListVersion();
    explicit GetLocalListVersion(const char *value);
    explicit GetLocalListVersion(cJSON *obj);
    ~GetLocalListVersion();

    GetLocalListVersion(const GetLocalListVersion&) = delete;
    GetLocalListVersion& operator=(const GetLocalListVersion&) = delete;

    GetLocalListVersion(GetLocalListVersion&& other) noexcept;
    GetLocalListVersion& operator=(GetLocalListVersion&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Response setters
    bool setListVersion(qint32 version);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Response getters
    qint32 listVersion() const;

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

#endif // GETLOCALLISTVERSION_H

