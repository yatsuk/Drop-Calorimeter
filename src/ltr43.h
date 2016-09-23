#ifndef LTR43_H
#define LTR43_H

#include <QObject>
#include <QTimer>
#include <QTimerEvent>
#include <QElapsedTimer>
#include "LTR\ltr43api.h"
#include "shared.h"

class Ltr43 : public QObject
{
    Q_OBJECT
public:
    explicit Ltr43(QObject *parent = 0);
    DWORD getLastStatusPort();
    ~Ltr43();
    
signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void upperPressure();
    void lowerPressure();
    void normalPressure();
    void calibrationHeaterOff(qint64 elapsedTime);
    void readPortsSignal(DWORD);

public slots:
    void initializationLTR43();
    void turnOnCalibrationHeater();
    void turnOnCalibrationHeaterTimer(int duration);
    void turnOffCalibrationHeater();
    void writePort (int port, int pin, bool value);
    DWORD readPorts ();


private:
    TLTR43 * ltr43;
    QTimer * readPortsTimer;
    QTimer * calibrHeaterTimer;
    QElapsedTimer * workTimeCalibrHeater;
    DWORD  lastStatusPort;

};

#endif // LTR43_H
