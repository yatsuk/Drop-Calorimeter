#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QJsonObject>
#include <QThread>
#include <QVector>
#include <QPair>
#include "shared.h"
#include "terconData.h"

class Device;

class DeviceManager: public QObject
{
    Q_OBJECT

public:
    explicit DeviceManager(QObject *parent = 0);
    Device * createDeviceFromJSON(const QJsonObject &parameters);
    ~DeviceManager();

public slots:
    bool destroyDevices();

private:
    QList <QPair <QThread *, Device *> > devices;

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
    void data (QJsonObject);

public slots:
    virtual bool setSetting(const QJsonObject &parameters){parameters_ = parameters;return true;}
    virtual QJsonObject getSetting(){return parameters_;}
    virtual bool initialization(){return false;}
    virtual bool connectDevice(){return false;}
    virtual bool disconnectDevice(){return false;}
    virtual bool start(){return false;}
    virtual bool stop(){return false;}

protected:
    QJsonObject parameters_;
};


#endif // DEVICE_H
