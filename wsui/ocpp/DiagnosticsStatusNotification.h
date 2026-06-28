#ifndef DIAGNOSTICSSTATUSNOTIFICATION_H
#define DIAGNOSTICSSTATUSNOTIFICATION_H

#include <QString>
#include "cJSON.h"
#include "Authorize.h"

enum OcppDiagnosticsStatus : quint8 {
    DIAG_IDLE = 0,
    DIAG_UPLOADED = 1,
    DIAG_UPLOAD_FAILED = 2,
    DIAG_UPLOADING = 3
};

class DiagnosticsStatusNotification
{
public:
    DiagnosticsStatusNotification();
    explicit DiagnosticsStatusNotification(const char *value);
    explicit DiagnosticsStatusNotification(cJSON *obj);
    ~DiagnosticsStatusNotification();

    DiagnosticsStatusNotification(const DiagnosticsStatusNotification&) = delete;
    DiagnosticsStatusNotification& operator=(const DiagnosticsStatusNotification&) = delete;

    DiagnosticsStatusNotification(DiagnosticsStatusNotification&& other) noexcept;
    DiagnosticsStatusNotification& operator=(DiagnosticsStatusNotification&& other) noexcept;

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

#endif // DIAGNOSTICSSTATUSNOTIFICATION_H
