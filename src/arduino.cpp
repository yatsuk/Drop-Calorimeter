#include "arduino.h"
#include <QTimer>
#include <QDebug>

Arduino::Arduino(QObject *parent) :
    QObject(parent)
{
    port = new QSerialPort(this);
    waitDropEnable = false;
    connect(port,SIGNAL(readyRead()),this,SLOT(readData()));
}

Arduino::~Arduino()
{
    stopAck();
}

void Arduino::readData(){
    QString str = port->readAll();
    qDebug() << str;
    if (str.startsWith("drop")){
        QTimer::singleShot(500,this,SLOT(delayDrop()));
    } else if (str.startsWith("fail: drop timeout")){
        emit message(tr("Пролет ампулы не зафиксирован."),Shared::warning);
        QTimer::singleShot(500,this,SLOT(delayDrop()));
    }

}

void Arduino::delayDrop()
{
    emit message(tr("Ампула сброшена."),Shared::information);
    emit droped();
}

void Arduino::enableLed(bool enable)
{
    if (!port->isOpen())
        return;

    if (enable){
        emit message(tr("Светодиод системы детектирования пролета ампулы включен."),Shared::information);
        port->write("2\r\n");
    } else {
        emit message(tr("Светодиод системы детектирования пролета ампулы выключен."),Shared::information);
        port->write("3\r\n");
    }
}

void Arduino::waitDrop()
{
    if (waitDropEnable){
        if (port->isOpen()){
            port->write("1\r\n");
            waitDropEnable = false;
        }
    }
}

void Arduino::setWaitDropEnable()
{
    waitDropEnable = true;
}

bool Arduino::startAck(){
    port->setPortName("COM3");
    if(!port->open(QIODevice::ReadWrite)){
        qDebug() << "open fail";
    }

    port->setBaudRate(QSerialPort::Baud115200);
    return true;
}

bool Arduino::stopAck(){
    if (port->isOpen()){
        enableLed(false);
        port->close();
    }
    return true;
}

void Arduino::setPortName(const QString &portName){
    port->setPortName(portName);
}

QString Arduino::portName(){
    return port->portName();
}


