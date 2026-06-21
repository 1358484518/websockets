#ifndef BOOTNOTIFICATION_H
#define BOOTNOTIFICATION_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"

enum OcppBootStatus : quint8 {
    BOOT_ACCEPTED = 0,
    BOOT_PENDING  = 1,
    BOOT_REJECTED = 2
};

class BootNotification
{
public:
    BootNotification();
    explicit BootNotification(const char *value);
    explicit BootNotification(cJSON *obj);
    ~BootNotification();

    BootNotification(const BootNotification&) = delete;
    BootNotification& operator=(const BootNotification&) = delete;

    BootNotification(BootNotification&& other) noexcept;
    BootNotification& operator=(BootNotification&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setType(quint8 type);

    // Request setters
    bool setVendor(const char *vendor);
    bool setModel(const char *model);
    bool setSerialNumber(const char *sn);
    bool setBoxSerialNumber(const char *sn);
    bool setFirmwareVersion(const char *version);
    bool setIccid(const char *iccid);
    bool setImsi(const char *imsi);
    bool setMeterType(const char *type);
    bool setMeterSerialNumber(const char *sn);

    // Response setters
    bool setStatus(quint8 s);
    bool setCurrentTime(const char *time);
    bool setInterval(qint32 interval);

    // Getters
    quint64 msgSeq() const;
    quint8 type() const;

    const char* vendor() const;
    const char* model() const;
    const char* serialNumber() const;
    const char* boxSerialNumber() const;
    const char* firmwareVersion() const;
    const char* iccid() const;
    const char* imsi() const;
    const char* meterType() const;
    const char* meterSerialNumber() const;

    quint8 status() const;
    const char* statusString() const;
    const char* currentTime() const;
    qint32 interval() const;

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
    qint32  m_interval;

    const char* statusToString(quint8 s) const;
    quint8 stringToStatus(const char *str) const;

    bool ensurePayload();
    bool setStringField(const char *key, const char *value);
    bool setNumberField(const char *key, double value);
    const char* getStringField(const char *key) const;
};

#endif // BOOTNOTIFICATION_H
