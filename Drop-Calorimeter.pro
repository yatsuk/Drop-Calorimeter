#-------------------------------------------------
#
# Project created by QtCreator 2012-04-20T09:29:42
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport printsupport opengl

TARGET = Drop-Calorimeter
TEMPLATE = app
CONFIG += c++14
SOURCES += src/main.cpp\
    src/lt300.cpp \
    src/temperatureSegment.cpp \
    src/segments.cpp \
    src/regulator.cpp \
    src/tercon.cpp \
    src/furnace.cpp \
    src/dac.cpp \
    src/diagnostic.cpp \
    src/ltr43.cpp \
    src/adc.cpp \
    src/dataRecorder.cpp \
    src/ltr114.cpp \
    src/Devices.cpp \
    src/arduino.cpp \
    src/GUI/adcSettingWidget.cpp \
    src/GUI/additionalHeatersWidget.cpp \
    src/GUI/addTempSegmentDialog.cpp \
    src/GUI/automaticRegulatorWidget.cpp \
    src/GUI/calibrationHeaterWidget.cpp \
    src/GUI/chartWidget.cpp \
    src/GUI/calorimetrBlockWidget.cpp \
    src/GUI/coversWidget.cpp \
    src/GUI/diagnosticSettingWidget.cpp \
    src/GUI/diagnosticwidget.cpp \
    src/GUI/dialogParametersRegulator.cpp \
    src/GUI/logView.cpp \
    src/GUI/mainwindow.cpp \
    src/GUI/manualRegulatorWidget.cpp \
    src/GUI/signalsView.cpp \
    src/GUI/widgetRegulatorFurnace.cpp \
    src/device.cpp \
    src/filter.cpp \
    src/data_manager.cpp \
    src/agilent.cpp \
    include/externals/qcustomplot/qcustomplot.cpp \
    include/externals/qledindicator/qledindicator.cpp \
    src/mit_8_20.cpp

HEADERS  += src/shared.h \
    src/lt300.h \
    src/parameters.h \
    src/temperatureSegment.h \
    src/segments.h \
    src/regulator.h \
    src/tercon.h \
    src/furnace.h \
    src/terconData.h \
    src/dac.h \
    src/regulatorParameters.h \
    src/diagnostic.h \
    src/ltr43.h \
    src/adc.h \
    src/dataRecorder.h \
    src/ltr114.h \
    src/Devices.h \
    src/arduino.h \
    src/GUI/adcSettingWidget.h \
    src/GUI/additionalHeatersWidget.h \
    src/GUI/addTempSegmentDialog.h \
    src/GUI/automaticRegulatorWidget.h \
    src/GUI/calibrationHeaterWidget.h \
    src/GUI/chartWidget.h \
    src/GUI/calorimetrBlockWidget.h \
    src/GUI/coversWidget.h \
    src/GUI/diagnosticSettingWidget.h \
    src/GUI/diagnosticwidget.h \
    src/GUI/dialogParametersRegulator.h \
    src/GUI/logView.h \
    src/GUI/mainwindow.h \
    src/GUI/manualRegulatorWidget.h \
    src/GUI/signalsView.h \
    src/GUI/widgetRegulatorFurnace.h \
    src/device.h \
    src/filter.h \
    src/data_manager.h \
    src/agilent.h \
    include/externals/nlohmann/json/json.hpp \
    include/externals/qledindicator/qledindicator.h \
    include/externals/qcustomplot/qcustomplot.h \
    src/mit_8_20.h


win32 {

LIBS += -lws2_32

}

linux {
    LIBS += -lltrapi -lltr27api -lltr34api -lltr43api -lltr114api
    QMAKE_LFLAGS += -no-pie
}

FORMS += \
    src/GUI/adcSettingWidget.ui \
    src/GUI/constVelocityWidget.ui \
    src/GUI/dialogParametersRegulator.ui \
    src/GUI/safetyValveWidget.ui

RESOURCES += \
    res/qt_res.qrc
