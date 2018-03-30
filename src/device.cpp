#include "device.h"
#include "tercon.h"
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
    Device * device = 0;
    QThread * deviceThread = 0;

    if (!parameters.empty()){
        if (parameters["type"].get<std::string>().c_str()=="Tercon"){
            deviceThread = new QThread;
            device = new Tercon;

            device->moveToThread(deviceThread);
            deviceThread->start(QThread::HighPriority);

            if(!device->initialization()){
                qDebug() << "initialization fail";
            }
            device->setObjectName(parameters["id"].get<std::string>().c_str());
            if(!device->setSetting(parameters)){
                qDebug() << "set settings fail";
            }
        }
    }
    devices.push_back(qMakePair(deviceThread, device));
    return device;
}

bool DeviceManager::destroyDevices()
{    
    while (!devices.isEmpty()) {
        Device * device = devices.back().second;
        QThread * deviceThread = devices.back().first;
        device->stop();
        device->disconnectDevice();
        deviceThread->quit();
        if (!deviceThread->wait(60*1000)){
            qDebug() << "thread force terminate";
            deviceThread->terminate();
            deviceThread->wait();
        }

        delete device;
        delete deviceThread;

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


