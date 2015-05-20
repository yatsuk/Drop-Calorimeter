#include "tercon.h"
#include <QDebug>
#include <QStringList>

Tercon::Tercon(QObject *parent) :
    QObject(parent)
{
    deviceNumber = -1;
    port = new QSerialPort(this);
    connect(port,SIGNAL(readyRead()),this,SLOT(readData()));
}

Tercon::~Tercon()
{
    stopAck();
}

void Tercon::setDeviceNumber(int number){
    deviceNumber = number;
}

void Tercon::convertData(QByteArray strData){
    TerconData data;
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

    data.value = tempStr.toDouble(&convertIsOK);
    if (!convertIsOK){
        emit message(tr("Ошибка чтения данных Теркона\n"
                        "(невозможно преобразовать строку в число): ")+strData+".",Shared::warning);
        return;
    }

    data.unit  = unitAndNumberData.at(unitAndNumberData.size()-1);
    unitAndNumberData.chop(1);

    data.channel = unitAndNumberData.toShort(&convertIsOK);
    if (!convertIsOK){
        emit message(tr("Ошибка чтения данных Теркона\n"
                        "(неверный номер канала): ")+strData+".",Shared::warning);
        return;
    }
    data.deviceNumber = deviceNumber;

    emit dataSend(data);
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

bool Tercon::startAck(){
    if(port->portName().isEmpty())
        return false;

    port->setBaudRate(QSerialPort::Baud9600);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    if(!port->open(QIODevice::ReadOnly)){
        qDebug() << "tercon open fail";
        return false;
    }

    return true;
}

bool Tercon::stopAck(){
    if (port->isOpen())
        port->close();
    return true;
}

void Tercon::setPortName(const QString &portName){
    port->setPortName(portName);
}

QString Tercon::portName(){
    return port->portName();
}
