#ifndef GETCONFIGURATION_H
#define GETCONFIGURATION_H

#include "cJSON.h"
#include "Authorize.h"

//enum OcppMsgType : quint8 {
//    OCPP_CALL = 2,
//    OCPP_CALLRESULT = 3,
//    OCPP_CALLERROR = 4
//};

class GetConfiguration
{
public:
    GetConfiguration();
    explicit GetConfiguration(const char *value);
    explicit GetConfiguration(cJSON *obj);
    ~GetConfiguration();

    GetConfiguration(const GetConfiguration&) = delete;
    GetConfiguration& operator=(const GetConfiguration&) = delete;

    GetConfiguration(GetConfiguration&& other) noexcept;
    GetConfiguration& operator=(GetConfiguration&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool addKey(const char *key);
    bool clearKeys();

    // Response setters
    bool addConfigKey(const char *key, bool readonly, const char *value = nullptr);
    bool clearConfigKeys();
    bool addUnknownKey(const char *key);
    bool clearUnknownKeys();

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    int keyCount() const;
    const char* getKey(int index) const;

    // Response getters
    int configKeyCount() const;
    bool getConfigKey(int index, const char *&key, bool &readonly, const char *&value) const;
    int unknownKeyCount() const;
    const char* getUnknownKey(int index) const;

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

#endif // GETCONFIGURATION_H
