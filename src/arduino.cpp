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
    if (msg.startsWith("drop")){
        emit message(tr("Ампула сброшена."),Shared::information);
        emit message(msg,Shared::information);
        emit droped();
    } else if (msg.startsWith("fail: drop timeout")){
        emit message(tr("Пролет ампулы не зафиксирован."),Shared::warning);
        emit message(msg,Shared::information);
    }else if (msg.startsWith("Test fotoresistor")){
        QStringList splittedMsg = msg.split("=");
        bool ok;
        double resistorValue = splittedMsg[1].trimmed().toDouble(&ok);
        dropSensorBrokenNotAck = false;
        if (resistorValue > 1000 || !ok){
            emit message(tr("Неисправность детектора пролета ампулы. "
                            "Ошибка получения сопритивления фоторезистора или поломка светодиода. "
                            "Сопротивление фоторезистора = %1").arg(resistorValue),Shared::warning);
        }
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
        port->write("2");
        dropSensorBrokenNotAck = false;
        QTimer::singleShot(5000, this, SLOT(testFotoResistor()));
    } else {
        port->write("3");
    }
}

void Arduino::testFotoResistor()
{
    port->write("4");
    dropSensorBrokenNotAck = true;
    QTimer::singleShot(5000, this, SLOT(dropSensorIsBroken()));
}

void Arduino::dropSensorIsBroken()
{
    if (dropSensorBrokenNotAck)
        emit message(tr("Неисправность детектора пролета ампулы."
                        "Детектор пролета не ответил на функцию тестирования."),Shared::warning);
}

void Arduino::waitDrop()
{  
    if (port->isOpen())
        port->write("1");
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


