#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QJsonObject>
#include "shared.h"

class Device : public QObject
{
    Q_OBJECT

public:
    ~Device();

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);

public slots:
    virtual void setSetting(const QJsonObject &parameters){parameters_ = parameters;}
    virtual QJsonObject getSetting(){return parameters_;}
    virtual bool connectDevice(){return false;}
    virtual bool disconnectDevice(){return false;}

protected:
    void createDevice();

private:
    QJsonObject parameters_;

};

#endif // DEVICE_H
