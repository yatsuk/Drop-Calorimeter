#ifndef AGILENT_H
#define AGILENT_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QSerialPort>
#include "terconData.h"
#include "shared.h"
#include "parameters.h"

class Agilent : public QObject
{
    Q_OBJECT
public:
    explicit Agilent(QObject *parent = 0);
    bool startAck();
    bool stopAck();

signals:
    void dataSend(TerconData data);
    void message(const QString & msg, Shared::MessageLevel msgLevel);

private slots:
    void readData();
    void writeData();
    void initData();
    void finalize();
    void extractData();
    void convertData(QByteArray strData);

private:

    double offsetVoltRoomTemperature;
    int deviceNumber;
    QSerialPort * port;
    QByteArray recvBytes;
    bool adcRunning;


    QTimer * emergencyTimer;

};

#endif // AGILENT_H
