#ifndef AGILENT_H
#define AGILENT_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QSerialPort>
#include "device.h"
#include "terconData.h"
#include "shared.h"
#include "parameters.h"

class Agilent : public Device
{
    Q_OBJECT
public:
    Agilent();
    ~Agilent();
    bool startAck();
    bool stopAck();

private slots:
    bool initialization();
    bool setSetting(const json &parameters);
    bool connectDevice();
    bool disconnectDevice();
    bool start();

    void readData();
    void writeData();
    void initData();
    void extractData();
    void convertData(QString strData);

private:

    QSerialPort * port;
    QByteArray recvBytes;
    json channelArray;

    QTimer * emergencyTimer;

};

#endif // AGILENT_H
