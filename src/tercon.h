#ifndef TERCON_H
#define TERCON_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QJsonObject>
#include <QJsonArray>
#include "device.h"
#include "terconData.h"
#include "shared.h"

class TerconWorker : public QObject
{
    Q_OBJECT

public:
    TerconWorker();

public slots:
    void doWork();
    void finishWork();
    void setParameters(const QJsonObject &parameters);

private slots:
    void readData();
    void extractData();
    void convertData(QByteArray strData);

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void dataSend(TerconData data);

private:
    QSerialPort * port;
    QByteArray recvBytes;
    QJsonArray channelArray;
};

class Tercon : public Device
{
    Q_OBJECT
public:
    Tercon();
    ~Tercon();

public slots:
    bool start();
    bool stop();
    bool connectDevice();
    bool disconnectDevice();

signals:
    void operate();
    void finishOperate();
    void sendParameters(const QJsonObject & json);

private:
    QThread workerThread_;
};

#endif // TERCON_H
