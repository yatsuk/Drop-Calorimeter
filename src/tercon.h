#ifndef TERCON_H
#define TERCON_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QJsonObject>
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
    int deviceNumber_ = -1;
    QString portName_;
};

class Tercon : public Device
{
    Q_OBJECT
public:
    Tercon();
    ~Tercon();
    void setPortName(const QString & portName);
    void setDeviceNumber(int number);
    bool startAck();
    bool stopAck();
    QString portName();

    QString description;



signals:
    void dataSend(TerconData data);
    //void message(const QString & msg, Shared::MessageLevel msgLevel);
    void operate();
    void finishOperate();
    void sendParameters(const QJsonObject & json);

private:
    int deviceNumber_ = -1;
    QString portName_;
    QThread workerThread_;
};

#endif // TERCON_H
