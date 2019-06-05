#include "device.h"
#include "tercon.h"
#include "mit_8_20.h"
#include "lt300.h"
#include <QTime>
#include <QDebug>

DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType <TerconData>();
}

DeviceManager::~DeviceManager()
{

}

Device * DeviceManager::createDeviceFromJSON(const json &parameters)
{
    Device * device = nullptr;

    if (!parameters.empty()){
        if (QString::compare(parameters["type"].get<std::string>().c_str(), "Tercon") == 0){
            device = new Tercon;

            if(!device->initialization()){
                qDebug() << "initialization fail";
            }
            device->setObjectName(parameters["id"].get<std::string>().c_str());
            if(!device->setSetting(parameters)){
                qDebug() << "set settings fail";
            }
        } else if(QString::compare(parameters["type"].get<std::string>().c_str(), "Mit_8_20") == 0){
            device = new Mit_8_20;

            if(!device->initialization()){
                qDebug() << "initialization fail";
            }
            device->setObjectName(parameters["id"].get<std::string>().c_str());
            if(!device->setSetting(parameters)){
                qDebug() << "set settings fail";
            }
        } else if(QString::compare(parameters["type"].get<std::string>().c_str(), "Lt300") == 0){
            device = new LT300;

            if(!device->initialization()){
                qDebug() << "initialization fail";
            }
            device->setObjectName(parameters["id"].get<std::string>().c_str());
            if(!device->setSetting(parameters)){
                qDebug() << "set settings fail";
            }
        }
    }
    devices.push_back(device);
    return device;
}

bool DeviceManager::destroyDevices()
{    
    while (!devices.isEmpty()) {
        Device * device = devices.back();
        device->stop();
        device->disconnectDevice();
        delete device;
        devices.pop_back();
    }

    return true;
}






Device::Device(QObject *parent) :
    QObject(parent)
{

}

Device::~Device()
{

}


