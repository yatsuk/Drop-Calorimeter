#ifndef LTR114_H
#define LTR114_H

#include <QObject>
#include <QThread>
#include "ltr114api.h"
#include "shared.h"
#include "parameters.h"
#include "terconData.h"

class Ltr114Worker : public QObject
{
    Q_OBJECT
public:
    explicit Ltr114Worker(QObject *parent = 0);
    ~Ltr114Worker();
    
signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void dataSend(TerconData data);
    void endInitialization();
    void endConfiguration();
    void endCalibration();
    void stopAck();
    
public slots:
    bool initialization();
    bool start();
    void stopWorker();
    bool stop();

private slots:
    bool configure();

private:
    long double convertVolt2TemperatureTypeA1(double value);
    long double convertTemperature2VoltTypeA1(double temperature);
    long double convertVolt2TemperatureTypeK(double value);
    long double convertTemperature2VoltTypeK(double temperature);


    AdcParameters parameters_;
    double offsetVoltRoomTemperature;

    bool isConfugurationOk;
    bool isStart;
    bool isWaitTimeout;
    bool isStopAck;
    int sizeBuf;
    DWORD * recvData;
    double * procData;
    TLTR114 * ltr114;

};

class Ltr114 : public QObject
{
    Q_OBJECT
public:
    explicit Ltr114(QObject *parent = 0);
    ~Ltr114();

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void dataSend(TerconData data);

public slots:
    void initialization();
    void start();
    bool stop();

private:
    Ltr114Worker* ltr114Worker;
    QThread * workThread;
};

#endif // LTR114_H
