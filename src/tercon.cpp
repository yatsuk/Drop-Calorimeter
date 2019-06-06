#include "tercon.h"
#include <QDebug>

Tercon::Tercon()
{
    port = nullptr;
}

Tercon::~Tercon()
{
    if (port)
        delete port;
}

bool Tercon::initialization()
{
    Device::initialization();
    port = new QSerialPort;
    connect(port, &QSerialPort::readyRead, this, &Tercon::readData);

    return true;
}

bool Tercon::setSetting(const json &parameters)
{
    Device::setSetting(parameters);
    channelArray = parameters["channels"];
    return true;
}

bool Tercon::start()
{
    Device::start();
    return true;
}

bool Tercon::stop()
{
    Device::stop();
    return true;
}

bool Tercon::connectDevice()
{
    Device::connectDevice();
    return openSerialPortSettings(port, parameters_["connectionSettings"]);
}

bool Tercon::disconnectDevice()
{
    Device::disconnectDevice();
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
        sendMessage(tr("Разделитель не обнаружен: (%1).").arg(QString(strData)),Shared::warning);
        return;
    }

    TerconData data;
    data.value = tempStr.toDouble(&convertIsOK);
    if (!convertIsOK){
        sendMessage(tr("Невозможно преобразовать строку в число: (%1).").arg(tempStr),Shared::warning);
        return;
    }

    if (unitAndNumberData.at((unitAndNumberData.size() - 1)) == 'U'){
        data.value /= 1000;
    }

    unitAndNumberData.chop(1);
    int channelNumber = unitAndNumberData.toInt(&convertIsOK);
    if (!convertIsOK){
        sendMessage(tr("Неверный номер канала: (%1).").arg(unitAndNumberData),Shared::warning);
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
    Device::deviceDataSended();
    recvBytes.append(port->readAll());
    extractData();
}


