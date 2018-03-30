#ifndef FURNACE_H
#define FURNACE_H

#include <QObject>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <include/externals/nlohmann/json/json.hpp>
#include "tercon.h"
#include "diagnostic.h"
#include "shared.h"
#include "regulator.h"
#include "dac.h"
#include "adc.h"
#include "ltr43.h"
#include "ltr114.h"
#include "data_manager.h"
#include "parameters.h"
#include "Devices.h"
#include "device.h"
#include "arduino.h"
#include "agilent.h"

using json = nlohmann::json;

class Furnace : public QObject
{
    Q_OBJECT

public:
    explicit Furnace(QObject *parent = 0);
    ~Furnace();
    Regulator * regulatorFurnace();
    Regulator * regulatorThermostat();
    Regulator * regulatorUpHeater();
    Regulator * regulatorDownHeater();
    Diagnostic * diagnostic();
    Covers * getCovers();
    SafetyValve * getSafetyValve();
    SampleLock * getSampleLock();
    qint64 getElapsedTime();
    static Furnace * instance();
    json getSettings();
    
signals:
    void AdcTerconDataSend(TerconData);
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void calibrationHeaterOff();
    
public slots:
    void run();
    void beginDataRecord();
    void endDataRecord();
    void turnOnCalibrationHeater(int duration);
    void turnOffCalibrationHeater();

    void writeLogMessageToFile(const QString & message);
    bool closeAppRequest();

private slots:
    void setDacValueFurnaceChannel(double value);
    void setDacValueThermostatChannel(double value);
    void setDacValueUpHeaterChannel(double value);
    void setDacValueDownHeaterChannel(double value);
    void writeFurnaceRegulatorLog(const QString & logString);
    void writeThermostatRegulatorLog(const QString & logString);
    void writeUpHeaterRegulatorLog(const QString & logString);
    void writeDownHeaterRegulatorLog(const QString & logString);
    void stopDacFurnaceChannel();
    void stopDacThermostatChannel();
    void stopDacUpHeaterChannel();
    void stopDacDownHeaterChannel();
    void terconDataReceive(TerconData data);
    void receiveData(TerconData data);
    bool connectTercon();
    void saveSettings();
    void loadSettings();
    void saveJSONSettings();
    void loadJSONSettings();

private:
    QSettings * settings;
    DataManager * dataManager;
    DeviceManager * deviceManager;

    Arduino * arduino;
    Agilent * agilent;

    Regulator * regulatorOfFurnace;
    Regulator * regulatorOfThermostat;
    Regulator * regulatorUpHeater_;
    Regulator * regulatorDownHeater_;
    Covers * covers;
    SafetyValve * safetyValve;
    SampleLock * sampleLock;
    DropDevice * dropDevice;
    Ltr43 * ltr43;
    Ltr114 * ltr114;
    DAC * dac;
    ADC * adc;
    Diagnostic * m_diagnostic;
    QElapsedTimer * time;
    static Furnace * g_furnace;


    json settings_;
    
};

#endif // FURNACE_H
