#ifndef OCPP_ALL_MESSAGES_H
#define OCPP_ALL_MESSAGES_H

// ===== OCPP 1.6J 全部消息头文件 =====

// 核心操作
#include "Authorize.h"
#include "BootNotification.h"
#include "Heartbeat.h"
#include "StatusNotification.h"

// 事务相关
#include "StartTransaction.h"
#include "StopTransaction.h"
#include "MeterValues.h"

// 远程控制
#include "RemoteStartTransaction.h"
#include "RemoteStopTransaction.h"
#include "TriggerMessage.h"
#include "UnlockConnector.h"

// 配置与本地列表
#include "ChangeConfiguration.h"
#include "GetConfiguration.h"
#include "GetLocalListVersion.h"
#include "SendLocalList.h"

// 充电配置
#include "SetChargingProfile.h"
#include "ClearChargingProfile.h"
#include "GetCompositeSchedule.h"

// 可用性
#include "ChangeAvailability.h"

// 缓存
#include "ClearCache.h"

// 预约
#include "ReserveNow.h"
#include "CancelReservation.h"

// 固件与诊断
#include "UpdateFirmware.h"
#include "FirmwareStatusNotification.h"
#include "GetDiagnostics.h"
#include "DiagnosticsStatusNotification.h"

// 数据传输
#include "DataTransfer.h"

// 错误相关
#include "OcppError.h"
//#include "ocpp_error_constants.h"

#endif // OCPP_ALL_MESSAGES_H
