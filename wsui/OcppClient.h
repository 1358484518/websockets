#ifndef OCPPCLIENT_H
#define OCPPCLIENT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QMap>

#include "ocpp/OcppAllMessages.h"

class WebSocketTcpClient;

class OcppClient : public QObject
{
    Q_OBJECT
public:
    explicit OcppClient(QObject *parent = nullptr);
    void setWebSocketClient(WebSocketTcpClient *wsClient);

Q_SIGNALS:
    // 凡事往外面发送websocket的数据都用它。
    void sigWebSocketTextSend( QByteArray data);
private slots:
    void onWebSocketTextReceived(QByteArray data);

private:
    WebSocketTcpClient *m_wsClient = nullptr;
    int m_messageId = 0;

    // 保存待匹配的请求：messageId -> action
//    QMap<QString, QString> m_pendingRequests;

    // 核心解析函数
    void parseOcppMessage(const QByteArray &data);

    // 工具函数
    QString generateMessageId();

private:
    // ===== CP → CS 主动请求：构建发送 Req 10个=====

    // --- 核心事务 ---
    void AuthorizeReq(cJSON *obj);
    void StartTransactionReq(cJSON *obj);
    void StopTransactionReq(cJSON *obj);
    void MeterValuesReq(cJSON *obj);
    void StatusNotificationReq(cJSON *obj);
    void HeartbeatReq(cJSON *obj);

    // --- 启动与透传 ---
    void BootNotificationReq(cJSON *obj);
    void DataTransferReq(cJSON *obj);       // ← 客户端主动发透传

    // --- 状态上报 ---
    void DiagnosticsStatusNotificationReq(cJSON *obj);
    void FirmwareStatusNotificationReq(cJSON *obj);
private:
    // ===== CS → CP 下发请求：处理后回 Conf 19个 DataTransf 是双向=====

    // --- 远程控制 ---
    void RemoteStartTransactionConf(cJSON *obj);
    void RemoteStopTransactionConf(cJSON *obj);
    void UnlockConnectorConf(cJSON *obj);
    void ResetConf(cJSON *obj);
    void ChangeAvailabilityConf(cJSON *obj);
    void TriggerMessageConf(cJSON *obj);

    // --- 配置管理 ---
    void GetConfigurationConf(cJSON *obj);
    void ChangeConfigurationConf(cJSON *obj);
    void GetLocalListVersionConf(cJSON *obj);
    void SendLocalListConf(cJSON *obj);
    void ClearCacheConf(cJSON *obj);

    // --- 充电配置 ---
    void SetChargingProfileConf(cJSON *obj);
    void GetCompositeScheduleConf(cJSON *obj);
    void ClearChargingProfileConf(cJSON *obj);
    void ReserveNowConf(cJSON *obj);
    void CancelReservationConf(cJSON *obj);//取消预约

    // --- 固件与诊断 ---
    void UpdateFirmwareConf(cJSON *obj);
    void GetDiagnosticsConf(cJSON *obj);

    // --- 透传（双向，服务器也能发）---
    void DataTransferConf(cJSON *obj);      // ← 新增：回复服务器的透传请求
private:
    // 收到服务器发来的 CALL（type=2）请求，根据 action 分发到对应 Conf 处理函数
    void handleServerCall(const QString& action, cJSON *payload);
    //回复不支持的请求
    void NotImplementedConf(cJSON *obj);
private:
    // ===== 类型定义 =====
    using HandlerFunc = void (OcppClient::*)(cJSON *obj);

    // ===== Conf 响应映射表：action字符串 → 处理函数 =====
    QMap<QString, HandlerFunc> m_MapActionConf = {
        // --- 远程控制 ---
        { "RemoteStartTransaction",     &OcppClient::RemoteStartTransactionConf     },
        { "RemoteStopTransaction",      &OcppClient::RemoteStopTransactionConf      },
        { "UnlockConnector",            &OcppClient::UnlockConnectorConf            },
        { "Reset",                      &OcppClient::ResetConf                      },
        { "ChangeAvailability",         &OcppClient::ChangeAvailabilityConf         },
        { "TriggerMessage",             &OcppClient::TriggerMessageConf             },

//        // --- 配置管理 ---
        { "GetConfiguration",           &OcppClient::GetConfigurationConf           },
        { "ChangeConfiguration",        &OcppClient::ChangeConfigurationConf        },
        { "GetLocalListVersion",        &OcppClient::GetLocalListVersionConf        },
        { "SendLocalList",              &OcppClient::SendLocalListConf              },
        { "ClearCache",                 &OcppClient::ClearCacheConf                 },

//        // --- 充电配置 ---
        { "SetChargingProfile",         &OcppClient::SetChargingProfileConf         },
        { "GetCompositeSchedule",       &OcppClient::GetCompositeScheduleConf       },
        { "ClearChargingProfile",       &OcppClient::ClearChargingProfileConf       },
        { "ReserveNow",                 &OcppClient::ReserveNowConf                 },
        { "CancelReservation",          &OcppClient::CancelReservationConf          },

//        // --- 固件与诊断 ---
        { "UpdateFirmware",             &OcppClient::UpdateFirmwareConf             },
        { "GetDiagnostics",             &OcppClient::GetDiagnosticsConf             },

//        // --- 透传（双向，服务器也能发）---
        { "DataTransfer",               &OcppClient::DataTransferConf               },
    };
    // ===== Req 请求映射表：action字符串 → 发送/构建函数 =====
    QMap<QString, HandlerFunc> m_MapActionReq = {
        // --- 核心事务 ---
//        { "Authorize",                  &OcppClient::AuthorizeReq                  },
//        { "StartTransaction",           &OcppClient::StartTransactionReq           },
//        { "StopTransaction",            &OcppClient::StopTransactionReq            },
        { "MeterValues",                &OcppClient::MeterValuesReq                },
        { "StatusNotification",         &OcppClient::StatusNotificationReq         },
        { "Heartbeat",                  &OcppClient::HeartbeatReq                  },

//        // --- 启动与透传 ---
        { "BootNotification",           &OcppClient::BootNotificationReq           },
//        { "DataTransfer",               &OcppClient::DataTransferReq               },

//        // --- 状态上报 ---
        { "DiagnosticsStatusNotification", &OcppClient::DiagnosticsStatusNotificationReq },
        { "FirmwareStatusNotification", &OcppClient::FirmwareStatusNotificationReq },
    };
};

#endif // OCPPCLIENT_H
