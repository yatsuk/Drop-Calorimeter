#include "mit_8_20.h"
#include <QDebug>

Mit_8_20::Mit_8_20()
{
    port = nullptr;
}

Mit_8_20::~Mit_8_20()
{
    if (port)
        delete port;
}

bool Mit_8_20::initialization()
{
    Device::initialization();
    port = new QSerialPort;
    connect(port, &QSerialPort::readyRead, this, &Mit_8_20::readData);

    return true;
}

bool Mit_8_20::setSetting(const json &parameters)
{
    Device::setSetting(parameters);
    channelArray = parameters["channels"];
    return true;
}

bool Mit_8_20::start()
{
    Device::start();
    return true;
}

bool Mit_8_20::stop()
{
    Device::stop();
    return true;
}

bool Mit_8_20::connectDevice()
{
    Device::connectDevice();
    return openSerialPortSettings(port, parameters_["connectionSettings"]);
}

bool Mit_8_20::disconnectDevice()
{
    Device::disconnectDevice();
    if (port && port->isOpen()){
        port->close();
        return true;
    }

    return false;
}

void Mit_8_20::convertData(QByteArray strData){
    bool convertIsOK=false;

    strData = strData.simplified();
    QString valueStr = strData.mid(2);
    valueStr.chop(1);

    TerconData data;
    data.value = valueStr.toDouble(&convertIsOK);
    if (!convertIsOK){
        sendMessage(tr("Невозможно преобразовать строку в число: (%1).").arg(valueStr),Shared::warning);
        return;
    }

    int channelNumber = strData.left(1).toInt(&convertIsOK);
    if (!convertIsOK){
        sendMessage(tr("Неверный номер канала: (%1).").arg(QString(strData.left(1))),Shared::warning);
        return;
    }

    for (unsigned int i = 0; i < channelArray.size(); ++i){
        json channel = channelArray[i];
        if (channel["channelNumber"] == channelNumber){
            data.id = channel["id"].get<std::string>().c_str();
            data.unit = channel["unit"].get<std::string>().c_str();
            emit dataSend(data);
            break;
        }
    }
}

void Mit_8_20::extractData(){
    QList <QByteArray> splitByteArray(recvBytes.split(' '));
    for(int i =0;i <splitByteArray.size()-1;++i){
        convertData(splitByteArray.at(i));
    }
    recvBytes.clear();
    recvBytes.append(splitByteArray.last());
}

void Mit_8_20::readData(){
    Device::deviceDataSended();
    recvBytes.append(port->readAll());
    extractData();
}


