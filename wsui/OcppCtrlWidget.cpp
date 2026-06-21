#include "OcppCtrlWidget.h"
#include "QPushButton"
#include "QLineEdit"
#include "QScrollArea"
#include "QVBoxLayout"

OcppCtrlWidget::OcppCtrlWidget(QWidget *parent) : QWidget(parent)
{
    struct OcppActionInfo {
        const char *name;
        const char *info;
        const char *action;
    };

    struct OcppActionInfo info[] = {
        // ===== 核心功能 =====
        {"Authorize",              "{\"idTag\":\"ABC1234567890\"}", "req"},
        {"BootNotification",       "{\"chargePointVendor\":\"ShenzhenX\",\"chargePointModel\":\"AC-7kW\",\"chargePointSerialNumber\":\"CP001\",\"firmwareVersion\":\"v1.0.0\"}", "req"},
        {"ChangeAvailability",     "{\"status\":\"Accepted\"}", "conf"},
        {"ChangeConfiguration",    "{\"status\":\"Accepted\"}", "conf"},
        {"ClearCache",             "{\"status\":\"Accepted\"}", "conf"},
        {"DataTransfer",           "{\"vendorId\":\"custom.vendor\",\"messageId\":\"test\",\"data\":\"hello\"}", "req"},
        {"GetConfiguration",       "{\"configurationKey\":[{\"key\":\"HeartbeatInterval\",\"readonly\":false,\"value\":\"300\"},{\"key\":\"ConnectionTimeOut\",\"readonly\":false,\"value\":\"60\"}],\"unknownKey\":[]}", "conf"},
        {"Heartbeat",              "{}", "req"},

        // ===== 核心功能（续）=====
        {"MeterValues",            "{\"connectorId\":1,\"transactionId\":1001,\"meterValue\":[{\"timestamp\":\"2024-01-01T12:00:00Z\",\"sampledValue\":[{\"value\":\"12345.67\",\"measurand\":\"Energy.Active.Import.Register\",\"unit\":\"Wh\",\"context\":\"Sample.Periodic\"},{\"value\":\"220.5\",\"measurand\":\"Voltage\",\"unit\":\"V\"},{\"value\":\"15.3\",\"measurand\":\"Current.Import\",\"unit\":\"A\"},{\"value\":\"3300\",\"measurand\":\"Power.Active.Import\",\"unit\":\"W\"}]}]}", "req"},
        {"RemoteStartTransaction", "{\"status\":\"Accepted\"}", "conf"},
        {"RemoteStopTransaction",  "{\"status\":\"Accepted\"}", "conf"},
        {"Reset",                  "{\"status\":\"Accepted\"}", "conf"},
        {"StartTransaction",       "{\"connectorId\":1,\"idTag\":\"ABC1234567890\",\"timestamp\":\"2024-01-01T12:00:00Z\",\"meterStart\":12000,\"reservationId\":0}", "req"},
        {"StatusNotification",     "{\"connectorId\":1,\"status\":\"Available\",\"errorCode\":\"NoError\",\"info\":\"\",\"timestamp\":\"2024-01-01T12:00:00Z\",\"vendorId\":\"\",\"vendorErrorCode\":\"\"}", "req"},
        {"StopTransaction",        "{\"idTag\":\"ABC1234567890\",\"meterStop\":12500,\"timestamp\":\"2024-01-01T13:00:00Z\",\"transactionId\":1001,\"reason\":\"EVDisconnected\"}", "req"},
        {"UnlockConnector",        "{\"status\":\"Unlocked\"}", "conf"},

        // ===== 固件管理 =====
        {"GetDiagnostics",              "{\"fileName\":\"diag_20240101.log\"}", "conf"},
        {"DiagnosticsStatusNotification", "{\"status\":\"Uploaded\"}", "req"},
        {"FirmwareStatusNotification",  "{\"status\":\"Installed\"}", "req"},
        {"UpdateFirmware",              "{}", "conf"},

        // ===== 本地身份验证列表管理 =====
        {"GetLocalListVersion",     "{\"listVersion\":1}", "conf"},
        {"SendLocalList",           "{\"status\":\"Accepted\"}", "conf"},

        // ===== 预约 =====
        {"CancelReservation",       "{\"status\":\"Accepted\"}", "conf"},
        {"ReserveNow",              "{\"status\":\"Accepted\"}", "conf"},

        // ===== 智能充电 =====
        {"ClearChargingProfile",    "{\"status\":\"Accepted\"}", "conf"},
        {"GetCompositeSchedule",    "{\"status\":\"Accepted\",\"connectorId\":1,\"scheduleStart\":\"2024-01-01T00:00:00Z\",\"chargingSchedule\":{\"duration\":86400,\"startSchedule\":\"2024-01-01T00:00:00Z\",\"chargingRateUnit\":\"W\",\"chargingSchedulePeriod\":[{\"startPeriod\":0,\"limit\":11000}]}}", "conf"},
        {"SetChargingProfile",      "{\"status\":\"Accepted\"}", "conf"},

        // ===== 消息触发 =====
        {"TriggerMessage",          "{\"status\":\"Accepted\"}", "conf"},
    };

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);           // ✅ 关键：内部 widget 自适应宽度
    scrollArea->setFrameShape(QFrame::NoFrame);     // 去掉边框，更美观

    // 2. 创建滚动区域内部的容器 widget
    QWidget *container = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(container);
    contentLayout->setContentsMargins(4, 4, 4, 4);
    contentLayout->setSpacing(4);

    // 3. 动态创建 LabeledEdit
    listLabeledEdit.clear();
    for (const auto &i : info) {                    // C++11 范围 for，比 foreach 更现代
        LabeledEdit *w = new LabeledEdit(i.name, i.action);
        w->setText(i.info);
        listLabeledEdit.append(w);
        contentLayout->addWidget(w);
    }

    contentLayout->addStretch();                    // ✅ 底部加弹簧，控件靠上排列

    // 4. 把容器 widget 设置给 scrollArea
    scrollArea->setWidget(container);               // ✅ 正确用法

    // 5. 加到主布局
    mainLayout->addWidget(scrollArea);
}
