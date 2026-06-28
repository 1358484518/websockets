#ifndef FIRMWARESTATUSNOTIFICATION_H
#define FIRMWARESTATUSNOTIFICATION_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"
//FW_DOWNLOADED           = 0  // "Downloaded" - 已下载
//FW_DOWNLOAD_FAILED      = 1  // "DownloadFailed" - 下载失败
//FW_INSTALLATION_FAILED  = 2  // "InstallationFailed" - 安装失败
//FW_INSTALLING           = 3  // "Installing" - 安装中
//FW_INSTALLED            = 4  // "Installed" - 已安装
enum OcppFirmwareStatus : quint8 {
    FW_DOWNLOADED = 0,
    FW_DOWNLOAD_FAILED = 1,
    FW_INSTALLATION_FAILED = 2,
    FW_INSTALLING = 3,
    FW_INSTALLED = 4
};

class FirmwareStatusNotification
{
public:
    FirmwareStatusNotification();
    explicit FirmwareStatusNotification(const char *value);
    explicit FirmwareStatusNotification(cJSON *obj);
    ~FirmwareStatusNotification();

    FirmwareStatusNotification(const FirmwareStatusNotification&) = delete;
    FirmwareStatusNotification& operator=(const FirmwareStatusNotification&) = delete;

    FirmwareStatusNotification(FirmwareStatusNotification&& other) noexcept;
    FirmwareStatusNotification& operator=(FirmwareStatusNotification&& other) noexcept;

    bool buildReq();
    bool buildConf();

    bool setMsgSeq(quint64 i);
    bool setMsgSeq(QString i);
    bool setType(quint8 type);

    // Request setters
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

#endif // FIRMWARESTATUSNOTIFICATION_H
