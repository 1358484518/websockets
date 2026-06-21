#include "OcppProtocol.h"
#include "QDebug"

OcppProtocol::OcppProtocol(QObject *parent) : QObject(parent)
{
//    m_ocppAction = new OcppAction();
}
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
