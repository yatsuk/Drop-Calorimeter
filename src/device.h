#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <include/externals/nlohmann/json/json.hpp>
#include <QThread>
#include <QVector>
#include <QPair>
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
    explicit Device(QObject *parent = 0);
    ~Device();

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void dataSend(TerconData data);
    void data (json);

public slots:
    virtual bool setSetting(const json &parameters){parameters_ = parameters;return true;}
    virtual json getSetting(){return parameters_;}
    virtual bool initialization(){return false;}
    virtual bool connectDevice(){return false;}
    virtual bool disconnectDevice(){return false;}
    virtual bool start(){return false;}
    virtual bool stop(){return false;}

protected:
    json parameters_;
};


#endif // DEVICE_H
