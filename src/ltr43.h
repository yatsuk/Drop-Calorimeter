#ifndef LTR43_H
#define LTR43_H

#include <QObject>
#include <QTimer>
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
    void calibrationHeaterOff();
    void readPortsSignal(DWORD);

public slots:
    void initializationLTR43();
    void turnOnCalibrationHeater();
    void turnOnCalibrationHeaterTimer(int duration);
    void turnOffCalibrationHeater();
    void writePort (int port, int pin, bool value);
    DWORD readPorts ();


private slots:
    void testSlotTimer();

private:
    TLTR43 * ltr43;
    QTimer * testTimer;
    QTimer * readPortsTimer;
    DWORD  lastStatusPort;

};

#endif // LTR43_H
