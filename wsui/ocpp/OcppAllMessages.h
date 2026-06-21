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

// Accepted	✅ 授权通过	刷卡有效，可以开始充电
// Blocked	🚫 卡被封禁 / 拉黑	这张卡被后台禁用了，比如挂失、欠费
// Expired	⏰ 卡已过期	卡的有效期到了，比如月卡到期
// Invalid	❌ 卡号无效	后台根本不认识这张卡，卡号不存在
// ConcurrentTx	⚠️ 重复交易	这张卡已经在另一个充电桩 / 另一把枪上充电了
enum OcppIdTagStatus : quint8
{
    IDTAG_ACCEPTED = 0,
    IDTAG_BLOCKED = 1,
    IDTAG_EXPIRED = 2,
    IDTAG_INVALID = 3,
    IDTAG_CONCURRENT_TX = 4
};
/*********************OcppAction********************************/

//九、OCPP 1.6 JSON Schema 核心总结
//对于 OCPP 开发，你只需要重点掌握以下 7 个最常用的字段，就能覆盖 99% 的场景：
//type：指定数据类型
//properties：定义对象的属性
//required：指定必填字段
//additionalProperties: false：禁止额外字段（OCPP 强制）
//enum：限制字段只能取指定值
//items：定义数组元素的结构
//format: date-time：时间戳格式

//{
//    "$schema": "http://json-schema.org/draft-04/schema#",
//    "id": "urn:OCPP:1.6:2019:12:RemoteStartTransactionResponse",
//    "title": "RemoteStartTransactionResponse",
//    "type": "object",
//    "properties": {
//        "status": {
//            "type": "string",
//            "additionalProperties": false,
//            "enum": [
//                "Accepted",
//                "Rejected"
//            ]
//        }
//    },
//    "additionalProperties": false,
//    "required": [
//        "status"
//    ]
//}

#endif // OCPP_ALL_MESSAGES_H
