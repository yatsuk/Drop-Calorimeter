#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <QObject>
#include <QTimer>
#include "shared.h"

enum StatusThermocouple{normalThermocouple,breakage};
enum Pressure {normal,upper,lower,undefined};

class Diagnostic : public QObject
{
    Q_OBJECT
public:
    explicit Diagnostic(QObject *parent = 0);
    
signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void controlThermocouple(bool);
    void alarmUpperPressure();
    void alarmNormalPressure();
    void alarmLowerPressure();
    void smoothOffRegulator();
    
public slots:
    void diagnosticThermocouple(double value);
    void upperPressure();
    void lowerPressure();
    void normalPressure();
    void enableEmitAlarmSignals(bool enable);
    void stopEmitAlarmSignals();
    void startEmitAlarmSignals();

private slots:
    void offRegulatorTimerTimeout();

private:
    QTimer * offRegulatorTimer;
    bool enableSignals;
    double oldValue;
    bool firstValue;
    StatusThermocouple statusThermocouple;
    Pressure m_Pressure;
};

#endif // DIAGNOSTIC_H
