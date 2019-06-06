#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <include/externals/nlohmann/json/json.hpp>
#include <QThread>
#include <QVector>
#include <QPair>
#include <QTimer>
#include <QtSerialPort>
#include "shared.h"
#include "terconData.h"

using json = nlohmann::json;

class Device;

class DeviceManager: public QObject
{
    Q_OBJECT

public:
    explicit DeviceManager(QObject *parent = nullptr);
    Device * createDeviceFromJSON(const json &parameters);
    ~DeviceManager();

public slots:
    bool destroyDevices();

private:
    QList <Device *> devices;

};

class Device : public QObject
{
    Q_OBJECT

public:
    explicit Device(QObject *parent = nullptr);
    ~Device();

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void dataSend(TerconData data);
    void data (json);

public slots:
    virtual bool setSetting(const json &parameters);
    virtual json getSetting(){return parameters_;}
    virtual bool initialization(){return true;}
    virtual bool connectDevice(){return true;}
    virtual bool disconnectDevice(){return true;}
    virtual bool start(){resetTimeoutTimer(); return true;}
    virtual bool stop(){stopTimeoutTimer(); return true;}
    void deviceDataSended();
    void sendMessage(const QString & msg, Shared::MessageLevel msgLevel);

private slots:
    void deviceTimerTimeout();

protected:
    bool openSerialPortSettings(QSerialPort * port, const json &portSettings);
    void resetTimeoutTimer();
    void stopTimeoutTimer();

    json parameters_;
    QTimer * deviceTimer_;
    bool deviceNotWorking_ = false;
    int deviceTimerTimeout_;
};


#endif // DEVICE_H
