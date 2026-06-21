#ifndef SENDLOCALLIST_H
#define SENDLOCALLIST_H

#include "cJSON.h"
#include "Authorize.h"

// 更新类型
enum OcppUpdateType : quint8 {
    UPDATE_DIFFERENTIAL = 0,
    UPDATE_FULL = 1
};

// 本地列表返回状态
enum OcppSendListStatus : quint8 {
    SEND_LIST_ACCEPTED = 0,
    SEND_LIST_FAILED = 1,
    SEND_LIST_NOT_SUPPORTED = 2,
    SEND_LIST_VERSION_MISMATCH = 3
};

//// 授权卡状态
//enum OcppAuthStatus : quint8 {
//    AUTH_ACCEPTED = 0,
//    AUTH_BLOCKED = 1,
//    AUTH_EXPIRED = 2,
//    AUTH_INVALID = 3,
//    AUTH_CONCURRENT_TX = 4
//};

class SendLocalList
{
public:
    SendLocalList();
    explicit SendLocalList(const char *value);
    explicit SendLocalList(cJSON *obj);
    ~SendLocalList();

    SendLocalList(const SendLocalList&) = delete;
    SendLocalList& operator=(const SendLocalList&) = delete;

    SendLocalList(SendLocalList&& other) noexcept;
    SendLocalList& operator=(SendLocalList&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setListVersion(qint32 ver);
    bool setUpdateType(quint8 type);
    int addAuthorization(const char *idTag, quint8 authStatus,
                         const char *expiryDate = nullptr,
                         const char *parentIdTag = nullptr);
    bool clearAuthorizations();

    // Response setters
    bool setStatus(quint8 status);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    // Request getters
    qint32 listVersion() const;
    quint8 updateType() const;
    const char* updateTypeString() const;
    int authorizationCount() const;
    bool getAuthorization(int idx,
        const char *&idTag, quint8 &authStatus,
        const char *&expiryDate, const char *&parentIdTag) const;

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
    quint8  m_respStatus;

    bool ensurePayload();
    cJSON* getAuthArray() const;
    cJSON* ensureAuthArray();

    static const char* updateTypeToString(quint8 t);
    static quint8 stringToUpdateType(const char *s);
    static const char* sendListStatusToString(quint8 s);
    static quint8 stringToSendListStatus(const char *s);
    static const char* authStatusToString(quint8 s);
    static quint8 stringToAuthStatus(const char *s);
};

#endif // SENDLOCALLIST_H
