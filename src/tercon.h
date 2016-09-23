#ifndef TERCON_H
#define TERCON_H

#include <QObject>
#include <QSerialPort>
#include <QJsonObject>
#include <QJsonArray>
#include "device.h"
#include "terconData.h"
#include "shared.h"


class Tercon : public Device
{
    Q_OBJECT
public:
    Tercon();
    ~Tercon();

public slots:
    bool initialization();
    bool start();
    bool stop();
    bool connectDevice();
    bool disconnectDevice();
    bool setSetting(const QJsonObject &parameters);

private slots:
    void readData();
    void extractData();
    void convertData(QByteArray strData);

private:
    QSerialPort * port;
    QByteArray recvBytes;
    QJsonArray channelArray;
};

#endif // TERCON_H
