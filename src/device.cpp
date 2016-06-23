#include "device.h"
#include "tercon.h"
#include <QTime>
#include <QDebug>

Device::~Device()
{

}

Device * Device::createDeviceFromJSON(const QJsonObject &parameters)
{
    Device * device = 0;
    if (!parameters.isEmpty()){
        if (parameters["type"].toString()=="Tercon"){
            device = new Tercon;
            device->setObjectName(parameters["id"].toString());
            device->setSetting(parameters);
        }
    }
    return device;
}
