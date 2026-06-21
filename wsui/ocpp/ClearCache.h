#ifndef CLEARCACHE_H
#define CLEARCACHE_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"

enum OcppClearStatus : quint8 {
    CLEAR_ACCEPTED = 0,
    CLEAR_REJECTED = 1
};

class ClearCache
{
public:
    ClearCache();
    explicit ClearCache(const char *value);
    explicit ClearCache(cJSON *obj);
    ~ClearCache();

    ClearCache(const ClearCache&) = delete;
    ClearCache& operator=(const ClearCache&) = delete;

    ClearCache(ClearCache&& other) noexcept;
    ClearCache& operator=(ClearCache&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Response setters
    bool setStatus(quint8 s);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

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
};

#endif // CLEARCACHE_H
