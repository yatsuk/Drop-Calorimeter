#ifndef FURNACE_H
#define FURNACE_H

#include <QObject>
#include <QTime>
#include <QFile>
#include <QSettings>
#include "tercon.h"
#include "diagnostic.h"
#include "shared.h"
#include "regulator.h"
#include "dac.h"
#include "adc.h"
#include "ltr43.h"
#include "ltr114.h"
#include "dataRecorder.h"
#include "parameters.h"
#include "Devices.h"
#include "arduino.h"


class Furnace : public QObject
{
    Q_OBJECT

public:
    explicit Furnace(QObject *parent = 0);
    Regulator * regulatorFurnace();
    Regulator * regulatorTermostat();
    Regulator * regulatorUpHeater();
    Regulator * regulatorDownHeater();
    Diagnostic * diagnostic();
    Covers * getCovers();
    SafetyValve * getSafetyValve();
    SampleLock * getSampleLock();
    static Furnace * instance();
    
signals:
    void AdcTerconDataSend(TerconData);
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void calibrationHeaterOff();
    
public slots:
    void run();
    void startTercon();
    void stopTercon();
    void beginDataRecord();
    void endDataRecord();
    void turnOnCalibrationHeater(int duration);
    void turnOffCalibrationHeater();

    void writeLogMessageToFile(const QString & message);
    bool closeAppRequest();

private slots:
    void setDacValueFurnaceChannel(double value);
    void setDacValueThermostateChannel(double value);
    void setDacValueUpHeaterChannel(double value);
    void setDacValueDownHeaterChannel(double value);
    void writeFurnaceRegulatorLog(const QString & logString);
    void writeThermostatRegulatorLog(const QString & logString);
    void writeUpHeaterRegulatorLog(const QString & logString);
    void writeDownHeaterRegulatorLog(const QString & logString);
    void stopDacFurnaceChannel();
    void stopDacThermostateChannel();
    void stopDacUpHeaterChannel();
    void stopDacDownHeaterChannel();
    void terconDataReceive(TerconData data);
    void receiveData(TerconData data);
    bool connectTercon();
    void writeFile(TerconData data);
    void saveSettings();
    void loadSettings();

private:
    double convertU2C (double U);

    QSettings * settings;
    DataRecorder * dataRecorder;

    Arduino * arduino;
    Tercon * tercon;
    Tercon * terconThermostat;
    Tercon * terconCalibrationHeater;
    Regulator * regulatorOfFurnace;
    Regulator * regulatorOfThermostat;
    Regulator * regulatorUpHeater_;
    Regulator * regulatorDownHeater_;
    Covers * covers;
    SafetyValve * safetyValve;
    SampleLock * sampleLock;
    Ltr43 * ltr43;
    Ltr114 * ltr114;
    DAC * dac;
    ADC * adc;
    Diagnostic * m_diagnostic;
    QTime * time;
    QTime * workingTimeCalibrHeater;
    static Furnace * g_furnace;
    
};

#endif // FURNACE_H
