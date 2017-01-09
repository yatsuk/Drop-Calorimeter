#include "agilent.h"
#include <math.h>
#include <QDebug>

Agilent::Agilent(QObject *parent) :
    QObject(parent)
{
    adcRunning = false;
    port = new QSerialPort(this);
    emergencyTimer = new QTimer(this);

    port->setPortName("COM9");
    connect (emergencyTimer,SIGNAL(timeout()),this,SLOT(initData()));
    connect(port,SIGNAL(readyRead()),this,SLOT(readData()));
}

void Agilent::convertData(QByteArray strData){
    QTimer::singleShot(100,this,SLOT(writeData()));

    TerconData data;
    data.unit=tr("В");
    data.id = "{0986e158-6266-4d5e-8498-fa5c3cd84bbe}";

    bool okConvertation;
    double value = strData.toDouble(&okConvertation)*1000;
    if (!okConvertation)return;

    data.value = value;

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
    if(!port->open(QIODevice::ReadWrite | QIODevice::Unbuffered))
        return false;
    if(port->setBaudRate(QSerialPort::Baud9600)&&
            port->setDataBits(QSerialPort::Data7)&&
            port->setParity(QSerialPort::EvenParity)&&
            port->setStopBits(QSerialPort::OneStop)&&
            port->setFlowControl(QSerialPort::NoFlowControl)){
        QTimer::singleShot(2000,this,SLOT(initData()));
        adcRunning = true;
        return true;
    }

    return false;
}

bool Agilent::stopAck(){
    if (port->isOpen())
        port->close();
    //port->write("SYST:LOC\r\n");
    //QTimer::singleShot(1000,this,SLOT(finalize()));
    return true;
}

void Agilent::writeData(){
    //qDebug()<<"write data";
    port->write("READ?\r\n");
    emergencyTimer->start(3000);
}

void Agilent::initData(){
    port->write("*RST; *CLS\r\n");
    port->write("SYST:REM\r\n");
    QTimer::singleShot(2000,this,SLOT(writeData()));

}

void Agilent::finalize(){
    port->close();
}
