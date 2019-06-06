#include "device.h"
#include "tercon.h"
#include "mit_8_20.h"
#include "lt300.h"
#include "agilent.h"
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
        } else if(QString::compare(parameters["type"].get<std::string>().c_str(), "Mit_8_20") == 0){
            device = new Mit_8_20;
        } else if(QString::compare(parameters["type"].get<std::string>().c_str(), "Lt300") == 0){
            device = new LT300;
        } else if(QString::compare(parameters["type"].get<std::string>().c_str(), "Agilent") == 0){
            device = new Agilent;
        }
        if (device != nullptr){
            if(!device->initialization()){
                device->sendMessage(tr("Сбой инициализации."), Shared::MessageLevel::critical);
            }
            device->setObjectName(parameters["id"].get<std::string>().c_str());
            if(!device->setSetting(parameters)){
                device->sendMessage(tr("Сбой установки настроек."), Shared::MessageLevel::critical);
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
    deviceTimer_ = new QTimer(this);
    deviceTimer_->setSingleShot(true);
    connect(deviceTimer_, &QTimer::timeout, this, &Device::deviceTimerTimeout);
}

Device::~Device()
{

}

bool Device::setSetting(const json &parameters)
{
    parameters_ = parameters;
    deviceTimerTimeout_ = parameters_["deviceTimeout"].get<int>();
    return true;
}

bool Device::openSerialPortSettings(QSerialPort * port, const json &portSettings)
{
    if(port == nullptr) return false;
    port->setPortName(portSettings["portName"].get<std::string>().c_str());
    port->setBaudRate(portSettings["baudRate"].get<int>());

    if(QString::compare(portSettings["dataBits"].get<std::string>().c_str(), "Data5") == 0){
        port->setDataBits(QSerialPort::Data5);
    } else if (QString::compare(portSettings["dataBits"].get<std::string>().c_str(), "Data6") == 0){
        port->setDataBits(QSerialPort::Data6);
    } else if (QString::compare(portSettings["dataBits"].get<std::string>().c_str(), "Data7") == 0){
        port->setDataBits(QSerialPort::Data7);
    } else if (QString::compare(portSettings["dataBits"].get<std::string>().c_str(), "Data8") == 0){
        port->setDataBits(QSerialPort::Data8);
    }

    if(QString::compare(portSettings["parity"].get<std::string>().c_str(), "NoParity") == 0){
        port->setParity(QSerialPort::NoParity);
    } else if (QString::compare(portSettings["parity"].get<std::string>().c_str(), "EvenParity") == 0){
        port->setParity(QSerialPort::EvenParity);
    } else if (QString::compare(portSettings["parity"].get<std::string>().c_str(), "OddParity") == 0){
        port->setParity(QSerialPort::OddParity);
    } else if (QString::compare(portSettings["parity"].get<std::string>().c_str(), "SpaceParity") == 0){
        port->setParity(QSerialPort::SpaceParity);
    } else if (QString::compare(portSettings["parity"].get<std::string>().c_str(), "MarkParity") == 0){
        port->setParity(QSerialPort::MarkParity);
    }

    if(QString::compare(portSettings["stopBits"].get<std::string>().c_str(), "OneStop") == 0){
        port->setStopBits(QSerialPort::OneStop);
    } else if (QString::compare(portSettings["stopBits"].get<std::string>().c_str(), "OneAndHalfStop") == 0){
        port->setStopBits(QSerialPort::OneAndHalfStop);
    } else if (QString::compare(portSettings["stopBits"].get<std::string>().c_str(), "TwoStop") == 0){
        port->setStopBits(QSerialPort::TwoStop);
    }

    if(QString::compare(portSettings["flowControl"].get<std::string>().c_str(), "NoFlowControl") == 0){
        port->setFlowControl(QSerialPort::NoFlowControl);
    } else if (QString::compare(portSettings["flowControl"].get<std::string>().c_str(), "HardwareControl") == 0){
        port->setFlowControl(QSerialPort::HardwareControl);
    } else if (QString::compare(portSettings["flowControl"].get<std::string>().c_str(), "SoftwareControl") == 0){
        port->setFlowControl(QSerialPort::SoftwareControl);
    }

    QIODevice::OpenModeFlag openMode = QIODevice::ReadWrite;
    if(QString::compare(portSettings["openMode"].get<std::string>().c_str(), "ReadOnly") == 0){
        openMode = QIODevice::ReadOnly;
    } else if (QString::compare(portSettings["openMode"].get<std::string>().c_str(), "WriteOnly") == 0){
        openMode = QIODevice::WriteOnly;
    } else if (QString::compare(portSettings["openMode"].get<std::string>().c_str(), "ReadWrite") == 0){
        openMode = QIODevice::ReadWrite;
    }

    if(!port->open(openMode)){
        sendMessage(tr("Ошибка открытия com-порта %1").arg(port->portName()),Shared::critical);
        return false;
    }

    if(portSettings["rts"].get<bool>()){
        port->setRequestToSend(true);
    } else {
        port->setRequestToSend(false);
    }

    return true;
}

void Device::sendMessage(const QString & msg, Shared::MessageLevel msgLevel)
{
    QString completeMessage;
    if(msgLevel == Shared::MessageLevel::warning){
        completeMessage.append(tr("Предупрежение! "));
    } else if (msgLevel == Shared::MessageLevel::critical){
        completeMessage.append(tr("Ошибка! "));
    }
    completeMessage.append(tr("Устройство \"%1\": ").arg(parameters_["name"].get<std::string>().c_str()));
    completeMessage.append(msg);
    emit (message(completeMessage, msgLevel));
}

void Device::deviceTimerTimeout()
{
    sendMessage(tr("В течение %1 мс устройство не передаёт данные.").arg(deviceTimerTimeout_),Shared::warning);
    deviceNotWorking_ = true;
}

void Device::deviceDataSended()
{
    if(deviceNotWorking_){
        sendMessage(tr("Устройство возобновило передачу данных."),Shared::information);
    }
    deviceNotWorking_ = false;
    resetTimeoutTimer();
}

void Device::resetTimeoutTimer()
{
    deviceTimer_->start(deviceTimerTimeout_);
}

void Device::stopTimeoutTimer()
{
    deviceTimer_->stop();
}


