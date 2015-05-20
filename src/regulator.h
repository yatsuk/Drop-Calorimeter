#ifndef REGULATOR_H
#define REGULATOR_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include <QTextStream>
#include "segments.h"
#include "parameters.h"
#include "shared.h"
#include "terconData.h"

class Regulator : public QObject
{
    Q_OBJECT
public:
    explicit Regulator(QObject *parent = 0);
    ~Regulator();
    void setTemperatureProgramm(Segments * tProgramm);
    RegulatorParameters parameters();
    Segments * getTemperatureProgramm();
    
signals:
    void startRegulator();
    void stopRegulator();
    void emergencyStopRegulator();
    void outPower(double);
    void currentTemperatureSegment(int);
    void manualMode();
    void regulatorLogData(const QString & logString);
    void setPointTemperature(TerconData data);
    void updateParameters();
    
public slots:
    void setMode(Shared::RegulatorMode regulatorMode);
    void setParameters(RegulatorParameters parameters);
    void regulatorStart();
    void regulatorStop();
    void regulatorEmergencyStop();
    void setValueADC(double value);
    void setValueManual(double value);
    void setTargetValue(double value);
    void setTemperatureProgMode(double temperature);
    void setDurationProgMode(double duration);
    void setVelocity(double velocity);
    void updateTemperatureProgramm();
    void smoothOff();
    void goToSegment(int numberSegment);

    void calculatePower(double value);

private slots:
    void smoothTimerTimeout();
    void timerTestSlot();
    void progTimerTimeout();

private:
    Shared::RegulatorMode mode;
    QTime startAutoRegulatorTime;
    Segments * temperatureProgramm;
    bool regulatorOn;
    bool firstValue;
    int currentSegment;
    double currentValueADC;

    double computePowerProg(double temperature);
    double averageValue(const QVector <double> &valueArray);
    bool isEndSegment(double currentTemperature,int segmentNumber);
    void clearRegulator();

    RegulatorParameters parameters_;
    double initialTemperature_;
    double temperatureProg_;
    bool progPowerHeat_;
    double duration_;
    double velocity_;
    double incPower_;
    double setPoint;
    double derivative;
    double integral;
    int count;
    bool firstPoint;
    QVector <double> waveform;
    QVector <double> control;
    QVector <double> I;
    QVector <double> D;
    QVector <double> averageArray;
    QVector <double> averagePowerArray;
    QVector <double> powerArray;
    double prevError;
    QTextStream outFileStream;    
    double power;
    double prevPower;

    QTimer * smoothOffTimer;
    QTimer * progPowerTimer;
    QTimer * testTimer;

    Segment * constVelocitySegment_;
};

#endif // REGULATOR_H
