#include "agilent.h"
#include <QDebug>

Agilent::Agilent()
{
    port = new QSerialPort(this);
    emergencyTimer = new QTimer(this);

    port->setPortName("ttyS6");
    connect (emergencyTimer,SIGNAL(timeout()),this,SLOT(initData()));
    connect(port,SIGNAL(readyRead()),this,SLOT(readData()));
}

Agilent::~Agilent()
{
    if (port)
        delete port;
}

bool Agilent::initialization()
{
    Device::initialization();
    port = new QSerialPort;
    connect(port, &QSerialPort::readyRead, this, &Agilent::readData);

    return true;
}

bool Agilent::setSetting(const json &parameters)
{
    Device::setSetting(parameters);
    channelArray = parameters["channels"];
    return true;
}

bool Agilent::connectDevice()
{
    Device::connectDevice();
    return openSerialPortSettings(port, parameters_["connectionSettings"]);
}

bool Agilent::disconnectDevice()
{
    Device::disconnectDevice();
    if (port && port->isOpen()){
        port->write("SYST:LOC\r\n");
        port->close();
        return true;
    }

    return false;
}

bool Agilent::start()
{
    Device::start();
    QTimer::singleShot(2000,this,SLOT(initData()));
    return true;
}

void Agilent::convertData(QString strData){
    QTimer::singleShot(100,this,SLOT(writeData()));

    strData = strData.trimmed();
    TerconData data;

    bool okConvertation;
    data.value = strData.toDouble(&okConvertation);
    if (!okConvertation){
        sendMessage(tr("Невозможно преобразовать строку в число: (%1).").arg(strData),Shared::warning);
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

void Agilent::extractData(){
    if (recvBytes.isEmpty())
        return;

    QList <QByteArray> splitByteArray(recvBytes.split('\r'));
    for(int i =0;i <splitByteArray.size()-1;++i){
        convertData(splitByteArray.at(i));
    }
    recvBytes.clear();
    recvBytes.append(splitByteArray.last());
}

void Agilent::readData(){
    Device::deviceDataSended();
    recvBytes.append(port->readAll());
    extractData();
}

void Agilent::writeData(){
    port->write("READ?\r\n");
    emergencyTimer->start(20000);
}

void Agilent::initData(){
    port->write("*RST; *CLS\r\n");
    port->write("SYST:REM\r\n");
    port->write(":CONFigure:VOLTage:DC MIN, MIN\r\n");
    QTimer::singleShot(2000,this,SLOT(writeData()));
}

