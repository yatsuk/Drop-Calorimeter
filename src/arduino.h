#ifndef ARDUINO_H
#define ARDUINO_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include "shared.h"
#include "terconData.h"

class Arduino : public QObject
{
    Q_OBJECT
public:
    explicit Arduino(QObject *parent = nullptr);
    ~Arduino();
    void setPortName(const QString & portName);
    bool startAck();
    bool stopAck();
    QString portName();

signals:
    void dataSend(TerconData data);
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void droped();

public slots:
    void waitDrop();
    void enableLed(bool enable);

private slots:
    void readData();
    void testFotoResistor();
    void dropSensorIsBroken();
    void getColdWaterTemperature();

private:
    void parseArduinoMessage(const QString & msg);

    bool waitDropEnable;
    bool dropSensorBrokenNotAck = false;
    QSerialPort * port;
    QString arduinoMessage;
    QTimer * coldWaterTemperatureTimer;
};

#endif // ARDUINO_H
