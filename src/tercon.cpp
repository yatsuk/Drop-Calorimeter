#include "tercon.h"
#include <QDebug>

Tercon::Tercon()
{
    port = 0;
}

Tercon::~Tercon()
{
    if (port)
        delete port;
}

bool Tercon::initialization()
{
    port = new QSerialPort;
    connect(port, &QSerialPort::readyRead, this, &Tercon::readData);

    return true;
}

bool Tercon::setSetting(const json &parameters)
{
    Device::setSetting(parameters);
    channelArray = parameters["channels"];
    if (port){
        port->setPortName(parameters["connectionSettings"]["portName"].get<std::string>().c_str());
        return true;
    }
    return false;
}

bool Tercon::start()
{
    return true;
}

bool Tercon::stop()
{
    return true;
}

bool Tercon::connectDevice()
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

bool Tercon::disconnectDevice()
{
    if (port && port->isOpen()){
        port->close();
        return true;
    }

    return false;
}

void Tercon::convertData(QByteArray strData){
    bool convertIsOK=false;

    strData = strData.simplified();
    QString tempStr = strData;
    int indexSeparator = tempStr.indexOf(QRegExp("[tRU]"));
    QString unitAndNumberData = tempStr.left(indexSeparator+1);
    tempStr.remove(0,indexSeparator+1);
    if(indexSeparator==-1){
        emit message(tr("Ошибка чтения данных Теркона\n"
                        "(разделитель не обнаружен): ")+strData+".",Shared::warning);
        return;
    }

    TerconData data;
    data.value = tempStr.toDouble(&convertIsOK);
    if (!convertIsOK){
        emit message(tr("Ошибка чтения данных Теркона\n"
                        "(невозможно преобразовать строку в число): ")+strData+".",Shared::warning);
        return;
    }

    if (unitAndNumberData.at((unitAndNumberData.size() - 1)) == 'U'){
        data.value /= 1000;
    }

    unitAndNumberData.chop(1);
    int channelNumber = unitAndNumberData.toInt(&convertIsOK);
    if (!convertIsOK){
        emit message(tr("Ошибка чтения данных Теркона\n"
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

void Tercon::extractData(){
    QList <QByteArray> splitByteArray(recvBytes.split('\r'));
    for(int i =0;i <splitByteArray.size()-1;++i){
        convertData(splitByteArray.at(i));
    }
    recvBytes.clear();
    recvBytes.append(splitByteArray.last());
}

void Tercon::readData(){
    recvBytes.append(port->readAll());
    extractData();
}


