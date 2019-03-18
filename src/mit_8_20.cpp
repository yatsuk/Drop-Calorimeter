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
    port = new QSerialPort;
    connect(port, &QSerialPort::readyRead, this, &Mit_8_20::readData);

    return true;
}

bool Mit_8_20::setSetting(const json &parameters)
{
    Device::setSetting(parameters);
    channelArray = parameters["channels"];
    if (port){
        port->setPortName(parameters["connectionSettings"]["portName"].get<std::string>().c_str());
        return true;
    }
    return false;
}

bool Mit_8_20::start()
{
    return true;
}

bool Mit_8_20::stop()
{
    return true;
}

bool Mit_8_20::connectDevice()
{
    if(!port || port->portName().isEmpty())
        return false;

    port->setBaudRate(QSerialPort::Baud9600);

    if(!port->open(QIODevice::ReadOnly)){
        emit message(tr("Ошибка открытия порта %1").arg(port->portName()),Shared::warning);
        return false;
    }

    port->setRequestToSend(false);
    return true;
}

bool Mit_8_20::disconnectDevice()
{
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
        emit message(tr("Ошибка чтения данных МИТ8.20\n"
                        "(невозможно преобразовать строку в число): ")+strData+".",Shared::warning);
        return;
    }

    int channelNumber = strData.left(1).toInt(&convertIsOK);
    if (!convertIsOK){
        emit message(tr("Ошибка чтения данных МИТ8.20\n"
                        "(неверный номер канала): ")+strData+".",Shared::warning);
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
    recvBytes.append(port->readAll());
    extractData();
}


