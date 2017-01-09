#ifndef ARDUINO_H
#define ARDUINO_H

#include <QObject>
#include <QSerialPort>
#include "shared.h"

class Arduino : public QObject
{
    Q_OBJECT
public:
    explicit Arduino(QObject *parent = 0);
    ~Arduino();
    void setPortName(const QString & portName);
    bool startAck();
    bool stopAck();
    QString portName();

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void droped();

public slots:
    void waitDrop();
    void enableLed(bool enable);

private slots:
    void readData();
    void testFotoResistor();
    void dropSensorIsBroken();

private:
    void parseArduinoMessage(const QString & msg);

    bool waitDropEnable;
    bool dropSensorBrokenNotAck = false;
    QSerialPort * port;
    QString arduinoMessage;

};

#endif // ARDUINO_H
