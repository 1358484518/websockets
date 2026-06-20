import asyncio
import json
import logging
from datetime import datetime, UTC  # 导入UTC时区
from websockets import connect

# ========== 配置 ==========
CSMS_URL = "ws://cpmstest.timxon.com:8887/ws/ocpp/1358484518"
CP_ID = "1358484518"
HEARTBEAT_INTERVAL = 5  # 5秒心跳
METER_VALUES_INTERVAL = 10  # 10秒上报一次电表数据
CONNECTOR_ID = 1  # 单枪充电桩用1
REPORT_METER_WHEN_IDLE = True  # 空闲时也上报电表数据

logging.basicConfig(
    level="INFO",
    format="%(asctime)s | %(levelname)s | %(message)s"
)
logger = logging.getLogger("OCPP1.6J")


class Ocpp16Client:
    def __init__(self, ws_url, cp_id):
        self.ws_url = ws_url
        self.cp_id = cp_id
        self.ws = None
        self.message_id = 0
        self.pending_requests = {}
        
        # 状态管理
        self.current_status = "Available"  # 初始状态：空闲
        self.transaction_id = 0  # 交易ID自增
        self.active_transaction = None  # 当前活跃交易
        
        # 电表数据模拟
        self.total_energy = 0.0  # 累计电能(kWh)
        self.voltage = 220.0  # 电压(V)
        self.current = 0.0  # 电流(A)
        self.power = 0.0  # 有功功率(W)

    async def connect(self):
        """建立WebSocket连接"""
        logger.info(f"🔌 正在连接: {self.ws_url}")
        self.ws = await connect(
            self.ws_url,
            subprotocols=["ocpp1.6"],  # 必须携带，平台靠这个识别OCPP连接
            ping_interval=None,
            ping_timeout=None
        )
        logger.info("✅ WebSocket连接成功")

    async def send_request(self, action, payload):
        """发送OCPP请求并等待响应"""
        self.message_id += 1
        msg_id = str(self.message_id)
        
        request = [2, msg_id, action, payload]
        request_json = json.dumps(request)
        
        logger.debug(f"📤 发送 {action}: {request_json}")
        
        future = asyncio.Future()
        self.pending_requests[msg_id] = future
        
        await self.ws.send(request_json)
        
        try:
            response = await asyncio.wait_for(future, timeout=60)
            return response
        except asyncio.TimeoutError:
            logger.error(f"❌ {action} 请求超时")
            del self.pending_requests[msg_id]
            raise

    async def send_response(self, msg_id, payload):
        """发送OCPP响应"""
        response = [3, msg_id, payload]
        response_json = json.dumps(response)
        await self.ws.send(response_json)
        logger.debug(f"📤 发送响应: {response_json}")

    async def handle_message(self, message):
        """处理收到的OCPP消息"""
        try:
            msg = json.loads(message)
            msg_type = msg[0]
            
            if msg_type == 3:  # 响应消息
                msg_id = msg[1]
                payload = msg[2]
                
                if msg_id in self.pending_requests:
                    future = self.pending_requests.pop(msg_id)
                    future.set_result(payload)
                    logger.debug(f"📥 收到响应: {message}")
                else:
                    logger.warning(f"⚠️ 收到未知消息ID的响应: {msg_id}")
            
            elif msg_type == 2:  # 服务器下发的请求
                msg_id = msg[1]
                action = msg[2]
                payload = msg[3]
                
                logger.info(f"📥 收到服务器指令: {action}")
                logger.debug(f"   指令参数: {payload}")
                
                # 处理不同类型的服务器指令
                if action == "StartTransaction":
                    await self.handle_start_transaction(msg_id, payload)
                elif action == "StopTransaction":
                    await self.handle_stop_transaction(msg_id, payload)
                elif action == "Reset":
                    await self.handle_reset(msg_id, payload)
                elif action == "UnlockConnector":
                    await self.handle_unlock_connector(msg_id, payload)
                elif action == "GetConfiguration":
                    await self.handle_get_configuration(msg_id, payload)
                elif action == "ChangeConfiguration":
                    await self.handle_change_configuration(msg_id, payload)
                else:
                    # 默认响应所有其他指令
                    await self.send_response(msg_id, {"status": "Accepted"})
                    logger.info(f"📤 已响应 {action}")
        
        except Exception as e:
            logger.error(f"❌ 处理消息失败: {e}")

    async def handle_start_transaction(self, msg_id, payload):
        """处理远程启动充电指令"""
        connector_id = payload["connectorId"]
        id_tag = payload["idTag"]
        
        logger.info(f"🚗 收到远程启动充电指令")
        logger.info(f"   连接器: {connector_id}, 卡号: {id_tag}")
        
        # 生成交易ID
        self.transaction_id += 1
        self.active_transaction = {
            "transactionId": self.transaction_id,
            "idTag": id_tag,
            "startTime": datetime.now(UTC).isoformat().replace("+00:00", "Z"),
            "meterStart": round(self.total_energy * 1000)  # 转换为Wh整数
        }
        
        # 重置电表数据
        self.current = 10.0  # 模拟10A充电电流
        self.power = self.voltage * self.current  # 2200W
        
        # 更新状态为充电中
        self.current_status = "Charging"
        await self.send_status_notification()
        
        # 返回启动响应
        response = {
            "transactionId": self.transaction_id,
            "idTagInfo": {"status": "Accepted"}
        }
        await self.send_response(msg_id, response)
        logger.info(f"✅ 充电已启动，交易ID: {self.transaction_id}")

    async def handle_stop_transaction(self, msg_id, payload):
        """处理远程停止充电指令"""
        transaction_id = payload["transactionId"]
        
        logger.info(f"🛑 收到远程停止充电指令")
        logger.info(f"   交易ID: {transaction_id}")
        
        if self.active_transaction and self.active_transaction["transactionId"] == transaction_id:
            # 停止充电
            self.current = 0.0
            self.power = 0.0
            
            # 更新状态为空闲
            self.current_status = "Available"
            await self.send_status_notification()
            
            # 清除活跃交易
            self.active_transaction = None
            
            # 返回停止响应
            response = {
                "idTagInfo": {"status": "Accepted"}
            }
            await self.send_response(msg_id, response)
            logger.info(f"✅ 充电已停止，交易ID: {transaction_id}")
        else:
            response = {
                "idTagInfo": {"status": "Invalid"}
            }
            await self.send_response(msg_id, response)
            logger.warning(f"❌ 无效的交易ID: {transaction_id}")

    async def handle_reset(self, msg_id, payload):
        """处理远程重启指令"""
        reset_type = payload["type"]
        logger.info(f"🔄 收到远程重启指令，类型: {reset_type}")
        
        await self.send_response(msg_id, {"status": "Accepted"})
        logger.info("📤 已响应重启指令，设备将在5秒后重启")
        
        # 模拟重启
        await asyncio.sleep(5)
        raise Exception("Device reset requested")

    async def handle_unlock_connector(self, msg_id, payload):
        """处理远程解锁枪头指令"""
        connector_id = payload["connectorId"]
        logger.info(f"🔓 收到远程解锁枪头指令，连接器: {connector_id}")
        
        await self.send_response(msg_id, {"status": "Unlocked"})
        logger.info("✅ 枪头已解锁")

    async def handle_get_configuration(self, msg_id, payload):
        """处理获取配置指令"""
        logger.info("📋 收到获取配置指令")
        
        response = {
            "configurationKey": [
                {"key": "HeartbeatInterval", "readonly": False, "value": str(HEARTBEAT_INTERVAL)},
                {"key": "MeterValueSampleInterval", "readonly": False, "value": str(METER_VALUES_INTERVAL)},
                {"key": "ChargePointModel", "readonly": True, "value": "AC_16J_TEST"},
                {"key": "ChargePointVendor", "readonly": True, "value": "TIMXON"}
            ]
        }
        await self.send_response(msg_id, response)
        logger.info("📤 已返回配置信息")

    async def handle_change_configuration(self, msg_id, payload):
        """处理修改配置指令"""
        key = payload["key"]
        value = payload["value"]
        
        logger.info(f"⚙️ 收到修改配置指令: {key} = {value}")
        
        # 支持修改心跳间隔和电表上报间隔
        if key == "HeartbeatInterval":
            global HEARTBEAT_INTERVAL
            HEARTBEAT_INTERVAL = int(value)
            logger.info(f"✅ 心跳间隔已修改为: {HEARTBEAT_INTERVAL}秒")
        elif key == "MeterValueSampleInterval":
            global METER_VALUES_INTERVAL
            METER_VALUES_INTERVAL = int(value)
            logger.info(f"✅ 电表上报间隔已修改为: {METER_VALUES_INTERVAL}秒")
        else:
            logger.warning(f"⚠️ 不支持修改的配置项: {key}")
        
        await self.send_response(msg_id, {"status": "Accepted"})

    async def send_status_notification(self):
        """发送状态上报"""
        payload = {
            "connectorId": CONNECTOR_ID,
            "status": self.current_status,
            "errorCode": "NoError",
            "timestamp": datetime.now(UTC).isoformat().replace("+00:00", "Z")
        }
        
        logger.info(f"📤 上报状态: {self.current_status}")
        await self.send_request("StatusNotification", payload)

    async def send_meter_values(self):
        """发送电表数据上报（支持空闲上报）"""
        # 模拟累计电能增加
        if self.current_status == "Charging" and self.active_transaction:
            # 充电中：10A电流，2200W功率
            self.total_energy += (self.power / 1000) * (METER_VALUES_INTERVAL / 3600)
        else:
            # 空闲中：模拟0.1A待机电流，22W功耗
            self.total_energy += (22 / 1000) * (METER_VALUES_INTERVAL / 3600)
        
        # 模拟电压波动
        self.voltage = 220.0 + (0.5 - asyncio.get_event_loop().time() % 1) * 2
        
        # OCPP1.6标准MeterValues格式
        payload = {
            "connectorId": CONNECTOR_ID,
            # 空闲时不携带transactionId
            "transactionId": self.active_transaction["transactionId"] if self.active_transaction else None,
            "meterValue": [
                {
                    "timestamp": datetime.now(UTC).isoformat().replace("+00:00", "Z"),
                    "sampledValue": [
                        {"value": f"{self.voltage:.1f}", "measurand": "Voltage", "unit": "V"},
                        {"value": f"{self.current:.1f}", "measurand": "Current.Import", "unit": "A"},
                        {"value": f"{self.power:.0f}", "measurand": "Power.Active.Import", "unit": "W"},
                        {"value": f"{round(self.total_energy * 1000)}", "measurand": "Energy.Active.Import.Register", "unit": "Wh"}
                    ]
                }
            ]
        }
        
        logger.info(f"📊 上报电表数据 | 电压:{self.voltage:.1f}V | 电流:{self.current:.1f}A | 功率:{self.power:.0f}W | 累计电能:{self.total_energy:.3f}kWh")
        await self.send_request("MeterValues", payload)

    async def boot_notification(self):
        """发送BootNotification"""
        payload = {
            "chargePointVendor": "TIMXON",
            "chargePointModel": "AC_16J_TEST",
            "chargePointSerialNumber": self.cp_id,
            "firmwareVersion": "1.0.0"
        }
        
        response = await self.send_request("BootNotification", payload)
        
        if response["status"] == "Accepted":
            logger.info("✅ Boot注册成功！设备已上线")
        elif response["status"] == "Pending":
            logger.warning("⏳ Boot注册待审核，请联系平台管理员通过")
        elif response["status"] == "Rejected":
            logger.error("❌ Boot注册被拒绝")
            raise Exception("Boot rejected")
        
        return response

    async def heartbeat_loop(self):
        """5秒一次心跳循环"""
        while True:
            try:
                response = await self.send_request("Heartbeat", {})
                logger.info(f"💓 心跳成功 | 服务器时间: {response['currentTime']}")
            except Exception as e:
                logger.error(f"❌ 心跳失败: {e}")
            await asyncio.sleep(HEARTBEAT_INTERVAL)

    async def meter_values_loop(self):
        """电表数据上报循环"""
        while True:
            try:
                await self.send_meter_values()
            except Exception as e:
                logger.error(f"❌ 电表数据上报失败: {e}")
            await asyncio.sleep(METER_VALUES_INTERVAL)

    async def message_loop(self):
        """消息接收循环"""
        try:
            async for message in self.ws:
                await self.handle_message(message)
        except Exception as e:
            logger.error(f"❌ 消息循环异常: {e}")
            raise

    async def run(self):
        """运行客户端"""
        while True:
            try:
                await self.connect()
                
                # 启动所有后台任务
                message_task = asyncio.create_task(self.message_loop())
                heartbeat_task = asyncio.create_task(self.heartbeat_loop())
                meter_values_task = asyncio.create_task(self.meter_values_loop())
                
                # 发送Boot注册
                await self.boot_notification()
                
                # 上报初始状态
                await self.send_status_notification()
                
                # 等待所有任务完成
                await asyncio.gather(message_task, heartbeat_task, meter_values_task)
                
            except Exception as e:
                logger.error(f"❌ 连接断开，5秒后重连: {e}")
                await asyncio.sleep(5)


if __name__ == "__main__":
    client = Ocpp16Client(CSMS_URL, CP_ID)
    asyncio.run(client.run())
    