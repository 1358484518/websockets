#ifndef RESET_H
#define RESET_H
#include <QString>
#include "Authorize.h"
#include "cJSON.h"

enum OcppResetType : quint8 {
    RESET_TYPE_HARD = 0,
    RESET_TYPE_SOFT = 1
};
enum OcppResetStatus : quint8 {
    RESET_STATUS_ACCEPTED = 0,
    RESET_STATUS_REJECTED = 1
};
class Reset
{
public:
    Reset();
    explicit Reset(const char *value);
    explicit Reset(cJSON *obj);
    ~Reset();
    Reset(const Reset&) = delete;
    Reset& operator=(const Reset&) = delete;
    Reset(Reset&& other) noexcept;
    Reset& operator=(Reset&& other) noexcept;
    bool buildReq();
    bool buildConf();
    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);
    // Request setters
    bool setResetType(quint8 resetType);
    // Response setters
    bool setStatus(quint8 status);
    // Getters
    quint64 msgSeq() const;
    quint8 type() const;
    // Request getters
    quint8 resetType() const;
    const char* resetTypeString() const;
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
    quint8  m_resetType;
    quint8  m_status;
    bool ensurePayload();
    static const char* resetTypeToString(quint8 resetType);
    static quint8 stringToResetType(const char *str);
    static const char* statusToString(quint8 status);
    static quint8 stringToStatus(const char *str);
};
#endif // RESET_H
