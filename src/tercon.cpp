#include "tercon.h"
#include <QDebug>
#include <QStringList>

Tercon::Tercon()
{
    TerconWorker * worker =  new TerconWorker;
    worker->moveToThread(&workerThread_);
    connect(&workerThread_, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &Tercon::sendParameters, worker, &TerconWorker::setParameters);
    connect(this, &Tercon::operate, worker, &TerconWorker::doWork);
    connect(this, &Tercon::finishOperate, worker, &TerconWorker::finishWork);
    connect(worker, SIGNAL(message(QString,Shared::MessageLevel)),
            this, SIGNAL(message(QString,Shared::MessageLevel)),Qt::BlockingQueuedConnection);
    connect(worker, SIGNAL(dataSend(TerconData)),
            this, SIGNAL(dataSend(TerconData)),Qt::BlockingQueuedConnection);
    workerThread_.start();
}

Tercon::~Tercon()
{
    stop();
    workerThread_.quit();
    if(!workerThread_.wait(5000)){
        workerThread_.terminate();
        qDebug() << "terminate tercon";
    }
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
    emit sendParameters(parameters_);
    emit operate();

    return true;
}

bool Tercon::disconnectDevice()
{
    emit finishOperate();
    return true;
}





TerconWorker::TerconWorker()
{
    port = new QSerialPort(this);
    connect(port, &QSerialPort::readyRead, this, &TerconWorker::readData);
}

void TerconWorker::doWork()
{
    if(port->portName().isEmpty())

    port->setBaudRate(QSerialPort::Baud9600);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    if(!port->open(QIODevice::ReadOnly)){
        emit message(tr("Ошибка открытия порта %1").arg(port->portName()),Shared::warning);
    }
}

void TerconWorker::finishWork()
{
    if (port->isOpen()){
        port->close();
    }
}

void TerconWorker::setParameters(const QJsonObject & parameters)
{

    channelArray = parameters["channels"].toArray();
    port->setPortName(parameters["connectionSettings"].toObject()["portName"].toString());
}

void TerconWorker::convertData(QByteArray strData){

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

    for (int i = 0; i < channelArray.size(); ++i){
        QJsonObject channel = channelArray[i].toObject();
        if (channel["channelNumber"].toInt() == channelNumber){
            data.id = channel["id"].toString();
            data.unit = channel["unit"].toString();
            emit dataSend(data);
            break;
        }
    }
}

void TerconWorker::extractData(){
    QList <QByteArray> splitByteArray(recvBytes.split('\r'));
    for(int i =0;i <splitByteArray.size()-1;++i){
        convertData(splitByteArray.at(i));
    }
    recvBytes.clear();
    recvBytes.append(splitByteArray.last());
}

void TerconWorker::readData(){
    recvBytes.append(port->readAll());
    extractData();
}
