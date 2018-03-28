#include "agilent.h"
#include <math.h>
#include <QDebug>

Agilent::Agilent(QObject *parent) :
    QObject(parent)
{
    adcRunning = false;
    port = new QSerialPort(this);
    emergencyTimer = new QTimer(this);

    port->setPortName("ttyS6");
    connect (emergencyTimer,SIGNAL(timeout()),this,SLOT(initData()));
    connect(port,SIGNAL(readyRead()),this,SLOT(readData()));
}

void Agilent::convertData(QString strData){
    QTimer::singleShot(100,this,SLOT(writeData()));

    strData = strData.trimmed();
    TerconData data;
    data.unit=tr("В");
    data.id = "{0986e158-6266-4d5e-8498-fa5c3cd84bbe}";

    bool okConvertation;
    data.value = strData.toDouble(&okConvertation);
    if (!okConvertation){
        emit message(tr("Agilent: Ошибка чтения значения \"%1\"").arg(strData),Shared::warning);
        return;
    }

    dataSend(data);
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
    recvBytes.append(port->readAll());
    extractData();
}

bool Agilent::startAck(){
    if(port->portName().isEmpty())
        return false;
    if(!port->open(QIODevice::ReadWrite)){
        emit message(tr("Agilent: Ошибка открытия порта %1").arg(port->portName()),Shared::warning);
        return false;
    }
    if(port->setBaudRate(QSerialPort::Baud9600)&&
            port->setDataBits(QSerialPort::Data7)&&
            port->setParity(QSerialPort::EvenParity)&&
            port->setStopBits(QSerialPort::TwoStop)&&
            port->setFlowControl(QSerialPort::SoftwareControl))
    {
        QTimer::singleShot(2000,this,SLOT(initData()));
        adcRunning = true;
        return true;
    }

    return false;
}

bool Agilent::stopAck(){
    if (port->isOpen()){
        port->write("SYST:LOC\r\n");
        port->close();
    }
    return true;
}

void Agilent::writeData(){
    port->write("READ?\r\n");
    emergencyTimer->start(10000);
}

void Agilent::initData(){
    port->write("*RST; *CLS\r\n");
    port->write("SYST:REM\r\n");
    port->write(":CONFigure:VOLTage:DC MIN, MIN\r\n");
    QTimer::singleShot(2000,this,SLOT(writeData()));

}

void Agilent::finalize(){
    port->close();
}
