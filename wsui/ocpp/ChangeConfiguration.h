#ifndef CHANGECONFIGURATION_H
#define CHANGECONFIGURATION_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"

enum OcppConfigStatus : quint8 {
    CONFIG_ACCEPTED = 0,
    CONFIG_REJECTED = 1,
    CONFIG_REBOOT_REQUIRED = 2,
    CONFIG_NOT_SUPPORTED = 3
};

class ChangeConfiguration
{
public:
    ChangeConfiguration();
    explicit ChangeConfiguration(const char *value);
    explicit ChangeConfiguration(cJSON *obj);
    ~ChangeConfiguration();

    ChangeConfiguration(const ChangeConfiguration&) = delete;
    ChangeConfiguration& operator=(const ChangeConfiguration&) = delete;

    ChangeConfiguration(ChangeConfiguration&& other) noexcept;
    ChangeConfiguration& operator=(ChangeConfiguration&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);

    // Request setters
    bool setKey(const char *key);
    bool setValue(const char *value);

    // Response setters
    bool setStatus(quint8 s);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    const char* key() const;
    const char* value() const;

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

    const char* statusToString(quint8 s) const;
    quint8 stringToStatus(const char *str) const;

    bool ensurePayload();
    bool setStringField(const char *key, const char *value);
    const char* getStringField(const char *key) const;
};

#endif // CHANGECONFIGURATION_H
