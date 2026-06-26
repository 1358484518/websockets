QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    OcppClient.cpp \
    OcppCtrlWidget.cpp \
    cJSON.c \
    main.cpp \
    ocpp/Authorize.cpp \
    ocpp/BootNotification.cpp \
    ocpp/CancelReservation.cpp \
    ocpp/ChangeAvailability.cpp \
    ocpp/ChangeConfiguration.cpp \
    ocpp/ClearCache.cpp \
    ocpp/ClearChargingProfile.cpp \
    ocpp/DataTransfer.cpp \
    ocpp/DiagnosticsStatusNotification.cpp \
    ocpp/FirmwareStatusNotification.cpp \
    ocpp/GetCompositeSchedule.cpp \
    ocpp/GetConfiguration.cpp \
    ocpp/GetDiagnostics.cpp \
    ocpp/GetLocalListVersion.cpp \
    ocpp/Heartbeat.cpp \
    ocpp/MeterValues.cpp \
    ocpp/OcppError.cpp \
    ocpp/RemoteStartTransaction.cpp \
    ocpp/RemoteStopTransaction.cpp \
    ocpp/ReserveNow.cpp \
    ocpp/Reset.cpp \
    ocpp/SendLocalList.cpp \
    ocpp/StartTransaction.cpp \
    ocpp/StatusNotification.cpp \
    ocpp/StopTransaction.cpp \
    ocpp/TriggerMessage.cpp \
    ocpp/UnlockConnector.cpp \
    ocpp/UpdateFirmware.cpp \
    ocpp/setchargingprofile.cpp \
    websockettcpclient.cpp \
    widget.cpp

HEADERS += \
    OcppClient.h \
    OcppCtrlWidget.h \
    cJSON.h \
    ocpp/Authorize.h \
    ocpp/BootNotification.h \
    ocpp/CancelReservation.h \
    ocpp/ChangeAvailability.h \
    ocpp/ChangeConfiguration.h \
    ocpp/ClearCache.h \
    ocpp/ClearChargingProfile.h \
    ocpp/DataTransfer.h \
    ocpp/DiagnosticsStatusNotification.h \
    ocpp/FirmwareStatusNotification.h \
    ocpp/GetCompositeSchedule.h \
    ocpp/GetConfiguration.h \
    ocpp/GetDiagnostics.h \
    ocpp/GetLocalListVersion.h \
    ocpp/Heartbeat.h \
    ocpp/MeterValues.h \
    ocpp/OcppAllMessages.h \
    ocpp/OcppError.h \
    ocpp/RemoteStartTransaction.h \
    ocpp/RemoteStopTransaction.h \
    ocpp/ReserveNow.h \
    ocpp/Reset.h \
    ocpp/SendLocalList.h \
    ocpp/SetChargingProfile.h \
    ocpp/StartTransaction.h \
    ocpp/StatusNotification.h \
    ocpp/StopTransaction.h \
    ocpp/TriggerMessage.h \
    ocpp/UnlockConnector.h \
    ocpp/UpdateFirmware.h \
    ocpp/ocpp_error_constants.h \
    websockettcpclient.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
