#-------------------------------------------------
#
# Project created by QtCreator 2012-04-20T09:29:42
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport printsupport opengl

TARGET = furnace
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    widgetRegulatorFurnace.cpp \
    logView.cpp \
    automaticRegulatorWidget.cpp \
    manualRegulatorWidget.cpp \
    progPowerRegulatorWidget.cpp \
    addTempSegmentDialog.cpp \
    temperatureSegment.cpp \
    segments.cpp \
    regulator.cpp \
    tercon.cpp \
    furnace.cpp \
    terconData.cpp \
    ltrapi.cpp \
    ltr34api.cpp \
    dac.cpp \
    channel.cpp \
    diagnosticwidget.cpp \
    diagnostic.cpp \
    ltr43.cpp \
    ltr43api.cpp \
    diagnosticSettingWidget.cpp \
    ltr27api.cpp \
    adc.cpp \
    signalsView.cpp \
    calibrationHeaterWidget.cpp\
    dataRecorder.cpp \
    dialogParametersRegulator.cpp \
    startupWizard.cpp \
    adcSettingWidget.cpp \
    ltr114api.cpp \
    crc.cpp \
    ltr114.cpp \
    additionalHeatersWidget.cpp \
    coversWidget.cpp \
    qledindicator.cpp \
    coversAndCalHeaterWidget.cpp \
    Devices.cpp \
    arduino.cpp \
    qcustomplot.cpp \
    chartWidget.cpp

HEADERS  += mainwindow.h \
    widgetRegulatorFurnace.h \
    logView.h \
    shared.h \
    parameters.h \
    automaticRegulatorWidget.h \
    manualRegulatorWidget.h \
    progPowerRegulatorWidget.h \
    addTempSegmentDialog.h \
    temperatureSegment.h \
    segments.h \
    regulator.h \
    tercon.h \
    furnace.h \
    terconData.h \
    ltrapitypes.h \
    ltrapidefine.h \
    ltrapi.h \
    ltr34api.h \
    dac.h \
    channel.h \
    regulatorParameters.h \
    diagnosticwidget.h \
    diagnostic.h \
    ltr43.h \
    ltr43api.h \
    diagnosticSettingWidget.h \
    ltr27api.h \
    adc.h \
    signalsView.h \
    calibrationHeaterWidget.h\
    dataRecorder.h \
    dialogParametersRegulator.h \
    startupWizard.h \
    ltr114api.h \
    crc.h \
    crclib_cfg.h \
    ltr114.h \
    adcSettingWidget.h \
    additionalHeatersWidget.h \
    qledindicator.h \
    coversWidget.h \
    coversAndCalHeaterWidget.h \
    Devices.h \
    arduino.h \
    qcustomplot.h \
    chartWidget.h


win32 {

LIBS += -lws2_32

}

FORMS += \
    dialogParametersRegulator.ui \
    startupWizard.ui \
    constVelocityWidget.ui \
    adcSettingWidget.ui \
    safetyValveWidget.ui
