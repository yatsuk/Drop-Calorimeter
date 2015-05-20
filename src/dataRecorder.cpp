#include "dataRecorder.h"
#include "shared.h"
#include <QDir>
#include <QDate>
#include <QDebug>

DataRecorder::DataRecorder(QObject *parent) :
    QObject(parent)
{
    logFile = 0;
    dataFile = 0;
    regulatorFurnaceFile = 0;
    regulatorThermostatFile = 0;
    regulatorUpHeaterFile = 0;
    regulatorDownHeaterFile = 0;
    mainSignalsFile = 0;
    thermostatSignalsFile = 0;
    calibrationHeaterFile = 0;


    path_=createPath();
    createFiles(path_);
    recordEnabled = false;
}

DataRecorder::~DataRecorder()
{
    if (logFile)
        logFile->close();
    if (dataFile)
        dataFile->close();
    if (regulatorFurnaceFile)
        regulatorFurnaceFile->close();
    if (regulatorThermostatFile)
        regulatorThermostatFile->close();
    if (regulatorUpHeaterFile)
        regulatorUpHeaterFile->close();
    if (regulatorDownHeaterFile)
        regulatorDownHeaterFile->close();

    delete logFile;
    delete dataFile;
    delete regulatorFurnaceFile;
    delete regulatorThermostatFile;
    delete regulatorUpHeaterFile;
    delete regulatorDownHeaterFile;
    delete mainSignalsFile;
    delete thermostatSignalsFile;
    delete calibrationHeaterFile;
}

void DataRecorder::beginRecord()
{
    recordEnabled = true;
    static int fileCount = 1;

    mainSignalsFile = new QFile(path_+"mainSignals_"+QString::number(fileCount)+".txt");
    if(mainSignalsFile->open(QIODevice::WriteOnly))
        mainSignalsFile->write("Time(sec)\tResistance(Omh)\tSampleTemperature(mV)\r\n");

    thermostatSignalsFile = new QFile(path_+"thermostatSignals_"+QString::number(fileCount)+".txt");
    if(thermostatSignalsFile->open(QIODevice::WriteOnly))
        thermostatSignalsFile->write("Time(sec)\tDiffTemperature(mV)\tThermostatTemperature(gr C)\r\n");

    calibrationHeaterFile = new QFile(path_+"calibrHeaterSignals_"+QString::number(fileCount)+".txt");
    if(calibrationHeaterFile->open(QIODevice::WriteOnly))
        calibrationHeaterFile->write("Time(sec)\tCalibrHeaterI(mV)\tCalibrHeaterV(mV)\r\n");

    fileCount++;
}

void DataRecorder::endRecord()
{
    recordEnabled = false;
    mainSignalsFile->close();
    thermostatSignalsFile->close();
    calibrationHeaterFile->close();
}

void DataRecorder::writeFile(const QString &data, Shared::FileType fileType)
{
    if ((fileType==Shared::logFile)&&logFile->isOpen()){
        logFile->write(data.toLocal8Bit());
        logFile->flush();
    }
    else if ((fileType==Shared::dataFile)&&dataFile->isOpen()){
        dataFile->write(data.toLatin1());
        dataFile->flush();
    }
    else if ((fileType==Shared::regulatorFurnaceFile)&&regulatorFurnaceFile->isOpen()){
        regulatorFurnaceFile->write(data.toLatin1());
        regulatorFurnaceFile->flush();
    }
    else if ((fileType==Shared::regulatorThermostatFile)&&regulatorThermostatFile->isOpen()){
        regulatorThermostatFile->write(data.toLatin1());
        regulatorThermostatFile->flush();
    }
    else if ((fileType==Shared::regulatorUpHeaterFile)&&regulatorUpHeaterFile->isOpen()){
        regulatorUpHeaterFile->write(data.toLatin1());
        regulatorUpHeaterFile->flush();
    }
    else if ((fileType==Shared::regulatorDownHeaterFile)&&regulatorDownHeaterFile->isOpen()){
        regulatorDownHeaterFile->write(data.toLatin1());
        regulatorDownHeaterFile->flush();
    }
    if (recordEnabled){
        if ((fileType==Shared::mainSignalsFile)&&mainSignalsFile->isOpen()){
            mainSignalsFile->write(data.toLatin1());
            mainSignalsFile->flush();
        }
        else if ((fileType==Shared::thermostatSignalsFile)&&thermostatSignalsFile->isOpen()){
            thermostatSignalsFile->write(data.toLatin1());
            thermostatSignalsFile->flush();
        }
        else if ((fileType==Shared::calibrationHeaterFile)&&calibrationHeaterFile->isOpen()){
            calibrationHeaterFile->write(data.toLatin1());
            calibrationHeaterFile->flush();
        }
    }
}

QString DataRecorder::createPath()
{
    QString date = QDate::currentDate().toString("dd.MM.yyyy");
    date.replace(".","_");
    QString appendName;
    QDir currentDir(QDir::current());
    if(!currentDir.exists("data"))
        currentDir.mkdir("data");
    currentDir.cd("data");

    bool isExist = currentDir.exists(date);
    if (isExist){
        for(int i =1;;++i){
            appendName="_"+QString::number(i);
            if (!currentDir.exists(date+appendName))
                break;
        }
    }

    currentDir.mkdir(date+appendName);
    return QString(currentDir.absolutePath()+"/"+date+appendName+"/");
}

void DataRecorder::createFiles(const QString & path)
{
    logFile = new QFile(path+"log.txt");
    logFile->open(QIODevice::WriteOnly);

    dataFile = new QFile(path+"data.txt");
    dataFile->open(QIODevice::WriteOnly);

    regulatorFurnaceFile = new QFile(path+"regulatorFurnace.txt");
    if(regulatorFurnaceFile->open(QIODevice::WriteOnly))
        regulatorFurnaceFile->write("Value\tavValue\tError\tPower\tP\tI\tD\r\n");

    regulatorThermostatFile = new QFile(path+"regulatorThermostat.txt");
    if(regulatorThermostatFile->open(QIODevice::WriteOnly))
        regulatorThermostatFile->write("Value\tavValue\tError\tPower\tP\tI\tD\r\n");

    regulatorUpHeaterFile = new QFile(path+"regulatorUpHeater.txt");
    if(regulatorUpHeaterFile->open(QIODevice::WriteOnly))
        regulatorUpHeaterFile->write("Value\tavValue\tError\tPower\tP\tI\tD\r\n");

    regulatorDownHeaterFile = new QFile(path+"regulatorDownHeater.txt");
    if(regulatorDownHeaterFile->open(QIODevice::WriteOnly))
        regulatorDownHeaterFile->write("Value\tavValue\tError\tPower\tP\tI\tD\r\n");


}
