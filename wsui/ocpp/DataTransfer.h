#ifndef DATATRANSFER_H
#define DATATRANSFER_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"

enum OcppDataTransferStatus : quint8 {
    DATA_TRANSFER_ACCEPTED = 0,
    DATA_TRANSFER_REJECTED = 1,
    DATA_TRANSFER_UNKNOWN_MESSAGE_ID = 2,
    DATA_TRANSFER_UNKNOWN_VENDOR_ID = 3
};

class DataTransfer
{
public:
    DataTransfer();
    explicit DataTransfer(const char *value);
    explicit DataTransfer(cJSON *obj);
    ~DataTransfer();

    DataTransfer(const DataTransfer&) = delete;
    DataTransfer& operator=(const DataTransfer&) = delete;

    DataTransfer(DataTransfer&& other) noexcept;
    DataTransfer& operator=(DataTransfer&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setVendorId(const char *vendorId);
    bool setMessageId(const char *msgId);
    bool setData(const char *data);

    // Response setters
    bool setStatus(quint8 s);
    bool setResponseData(const char *data);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    const char* vendorId() const;
    const char* messageId() const;
    const char* data() const;

    quint8 status() const;
    const char* statusString() const;
    const char* responseData() const;

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
    bool hasField(const char *key) const;
};

#endif // DATATRANSFER_H


