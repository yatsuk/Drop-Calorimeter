#include "lt300.h"
#include <QDebug>

LT300::LT300()
{
    port = nullptr;
    askTimer = new QTimer(this);
    connect(askTimer, &QTimer::timeout, this, &LT300::askTimerTimeout);
}

LT300::~LT300()
{
    if (port)
        delete port;
}

bool LT300::initialization()
{
    port = new QSerialPort;
    connect(port, &QSerialPort::readyRead, this, &LT300::readData);

    return true;
}

bool LT300::setSetting(const json &parameters)
{
    Device::setSetting(parameters);
    channelArray = parameters["channels"];
    if (port){
        port->setPortName(parameters["connectionSettings"]["portName"].get<std::string>().c_str());
        return true;
    }
    return false;
}

bool LT300::start()
{
    return true;
}

bool LT300::stop()
{
    return true;
}

bool LT300::connectDevice()
{
    if(!port || port->portName().isEmpty())
        return false;

    port->setBaudRate(QSerialPort::Baud4800);

    if(!port->open(QIODevice::ReadWrite)){
        emit message(tr("Ошибка открытия порта %1").arg(port->portName()),Shared::warning);
        return false;
    }

    port->setRequestToSend(false);
    askTimer->start(500);
    return true;
}

bool LT300::disconnectDevice()
{
    if (port && port->isOpen()){
        port->close();
        return true;
    }

    return false;
}

void LT300::askTimerTimeout()
{
    port->write("d\r");
}

void LT300::convertData(QByteArray strData){
    bool convertIsOK=false;
    TerconData data;

    strData = strData.simplified();
    int indexSeparator = strData.indexOf(" ");
    if(indexSeparator==-1){
        emit message(tr("Ошибка чтения данных LT300\n"
                        "(разделитель не обнаружен): ")+strData+".",Shared::warning);
        return;
    }

    strData.left(indexSeparator).toDouble(&convertIsOK);
    if (!convertIsOK){
        emit message(tr("Ошибка чтения данных LT300\n"
                        "(невозможно преобразовать строку в число): ")+strData+".",Shared::warning);
        return;
    }
    data.value = strData.right(indexSeparator - 2).toDouble(&convertIsOK);
    if (!convertIsOK){
        emit message(tr("Ошибка чтения данных LT300\n"
                        "(невозможно преобразовать строку в число): ")+strData+".",Shared::warning);
        return;
    }


    int channelNumber = 1;
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

void LT300::extractData(){
    QList <QByteArray> splitByteArray(recvBytes.split('\r'));
    for(int i =0;i <splitByteArray.size()-1;++i){
        convertData(splitByteArray.at(i));
    }
    recvBytes.clear();
    recvBytes.append(splitByteArray.last());
}

void LT300::readData(){
    recvBytes.append(port->readAll());
    extractData();
}


