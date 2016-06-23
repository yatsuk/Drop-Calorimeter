#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QJsonObject>
#include "shared.h"
#include "terconData.h"

class Device : public QObject
{
    Q_OBJECT

public:
    ~Device();

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void dataSend(TerconData data);
    void data (QJsonObject);

public slots:
    virtual void setSetting(const QJsonObject &parameters){parameters_ = parameters;}
    virtual QJsonObject getSetting(){return parameters_;}
    virtual bool connectDevice(){return false;}
    virtual bool disconnectDevice(){return false;}
    virtual bool start(){return false;}
    virtual bool stop(){return false;}
    static  Device * createDeviceFromJSON(const QJsonObject &parameters);

protected:
    QJsonObject parameters_;

};


#endif // DEVICE_H
