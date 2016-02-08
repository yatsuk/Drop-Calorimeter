#-------------------------------------------------
#
# Project created by QtCreator 2012-04-20T09:29:42
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport printsupport opengl

TARGET = furnace
TEMPLATE = app
CONFIG += c++14

SOURCES += main.cpp\
    temperatureSegment.cpp \
    segments.cpp \
    regulator.cpp \
    tercon.cpp \
    furnace.cpp \
    terconData.cpp \
    dac.cpp \
    diagnostic.cpp \
    ltr43.cpp \
    adc.cpp \
    dataRecorder.cpp \
    ltr114.cpp \
    Devices.cpp \
    arduino.cpp \
    GUI/adcSettingWidget.cpp \
    GUI/additionalHeatersWidget.cpp \
    GUI/addTempSegmentDialog.cpp \
    GUI/automaticRegulatorWidget.cpp \
    GUI/calibrationHeaterWidget.cpp \
    GUI/chartWidget.cpp \
    GUI/coversAndCalHeaterWidget.cpp \
    GUI/coversWidget.cpp \
    GUI/diagnosticSettingWidget.cpp \
    GUI/diagnosticwidget.cpp \
    GUI/dialogParametersRegulator.cpp \
    GUI/logView.cpp \
    GUI/mainwindow.cpp \
    GUI/manualRegulatorWidget.cpp \
    GUI/progPowerRegulatorWidget.cpp \
    GUI/qcustomplot.cpp \
    GUI/qledindicator.cpp \
    GUI/signalsView.cpp \
    GUI/startupWizard.cpp \
    GUI/widgetRegulatorFurnace.cpp \
    LTR/channel.cpp \
    LTR/crc.cpp \
    LTR/ltr27api.cpp \
    LTR/ltr34api.cpp \
    LTR/ltr43api.cpp \
    LTR/ltr114api.cpp \
    LTR/ltrapi.cpp \
    device.cpp \
    filter.cpp

HEADERS  += shared.h \
    parameters.h \
    temperatureSegment.h \
    segments.h \
    regulator.h \
    tercon.h \
    furnace.h \
    terconData.h \
    dac.h \
    regulatorParameters.h \
    diagnostic.h \
    ltr43.h \
    adc.h \
    dataRecorder.h \
    ltr114.h \
    Devices.h \
    arduino.h \
    GUI/adcSettingWidget.h \
    GUI/additionalHeatersWidget.h \
    GUI/addTempSegmentDialog.h \
    GUI/automaticRegulatorWidget.h \
    GUI/calibrationHeaterWidget.h \
    GUI/chartWidget.h \
    GUI/coversAndCalHeaterWidget.h \
    GUI/coversWidget.h \
    GUI/diagnosticSettingWidget.h \
    GUI/diagnosticwidget.h \
    GUI/dialogParametersRegulator.h \
    GUI/logView.h \
    GUI/mainwindow.h \
    GUI/manualRegulatorWidget.h \
    GUI/progPowerRegulatorWidget.h \
    GUI/qcustomplot.h \
    GUI/qledindicator.h \
    GUI/signalsView.h \
    GUI/startupWizard.h \
    GUI/ui_adcSettingWidget.h \
    GUI/ui_constVelocityWidget.h \
    GUI/ui_dialogParametersRegulator.h \
    GUI/ui_mainwindow.h \
    GUI/ui_safetyValveWidget.h \
    GUI/ui_startupWizard.h \
    GUI/widgetRegulatorFurnace.h \
    LTR/channel.h \
    LTR/crc.h \
    LTR/crclib_cfg.h \
    LTR/ltr27api.h \
    LTR/ltr34api.h \
    LTR/ltr43api.h \
    LTR/ltr114api.h \
    LTR/ltrapi.h \
    LTR/ltrapidefine.h \
    LTR/ltrapitypes.h \
    device.h \
    filter.h


win32 {

LIBS += -lws2_32

}

FORMS += \
    GUI/adcSettingWidget.ui \
    GUI/constVelocityWidget.ui \
    GUI/dialogParametersRegulator.ui \
    GUI/safetyValveWidget.ui \
    GUI/startupWizard.ui \
    GUI/addDevice.ui

RESOURCES += \
    ../res/qt_res.qrc
