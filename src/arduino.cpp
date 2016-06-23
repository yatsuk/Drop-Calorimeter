#include "arduino.h"
#include <QTimer>
#include <QDebug>

Arduino::Arduino(QObject *parent) :
    QObject(parent)
{
    port = new QSerialPort(this);
    connect(port,SIGNAL(readyRead()),this,SLOT(readData()));
}

Arduino::~Arduino()
{

}

void Arduino::readData(){
    arduinoMessage.append(port->readAll());
    while(true){
        int endStrPos = arduinoMessage.indexOf('\n');
        if (endStrPos== -1) break;

        parseArduinoMessage(arduinoMessage.left(endStrPos).trimmed());
        arduinoMessage.remove(0, endStrPos + 1);
    }
}

void Arduino::parseArduinoMessage(const QString & msg)
{
    if (msg == "drop"){
        emit message(tr("Ампула сброшена."),Shared::information);
        QTimer::singleShot(500,this,SIGNAL(droped()));
    } else if (msg == "fail: drop timeout"){
        emit message(tr("Пролет ампулы не зафиксирован."),Shared::warning);
        emit droped();
    } else if (msg == "led_pin, HIGH"){
        emit message(tr("Светодиод системы детектирования пролета ампулы включен."),Shared::information);
    } else if (msg == "led_pin, LOW"){
        emit message(tr("Светодиод системы детектирования пролета ампулы выключен."),Shared::information);
    } else if (msg == "ADC start"){
        emit message(tr("Ожидание сброса ампулы."),Shared::information);
    } else {
        emit message(tr("Arduino message: %1").arg(msg),Shared::information);
    }
}

void Arduino::enableLed(bool enable)
{
    if (!port->isOpen())
        return;

    if (enable){
        port->write("2\r\n");
    } else {
        port->write("3\r\n");
    }
}

void Arduino::waitDrop()
{  
    if (port->isOpen())
        port->write("1\r\n");
}

bool Arduino::startAck(){
    port->setPortName("COM3");
    if(!port->open(QIODevice::ReadWrite)){
        emit message(tr("Arduino: Ошибка открытия порта COM3"),Shared::warning);
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


