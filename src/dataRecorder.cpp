#include "dataRecorder.h"
#include "shared.h"
#include <QDir>
#include <QDate>
#include <QVector>
#include <QPair>
#include <QMessageBox>
#include <QDebug>

DataRecorder::DataRecorder(QObject *parent) : QObject(parent)
{

}

DataRecorder::~DataRecorder()
{

}

DataRecorder * DataRecorder::createDataRecorderFromJSON(const json &parameters)
{
    DataRecorder * dataRecorder = 0;
    if (!parameters.empty()){
        if (QString::compare(parameters["type"].get<std::string>().c_str(), "File") == 0){
            dataRecorder = new FileRecorder;
        }
        if (dataRecorder){
            dataRecorder->setObjectName(parameters["id"].get<std::string>().c_str());
            dataRecorder->setSetting(parameters);
        }
    }
    return dataRecorder;
}





QString FileRecorder::path_ = "";

FileRecorder::FileRecorder()
{
    file = 0;
    if(path_.isEmpty())
        path_ = createPath();
}

FileRecorder::~FileRecorder()
{
    if (file && file->isOpen()){
        file->close();
    }

    delete file;
}

void FileRecorder::setSetting(const json &parameters)
{
    json fileSettings = parameters["settings"];
    if(fileSettings.empty())return;

    file = new QFile(path_ + fileSettings["fileName"].get<std::string>().c_str() + "."
            + fileSettings["fileExtension"].get<std::string>().c_str());

    json columnsInfo = fileSettings["columns"];
    for (unsigned int i = 0; i < columnsInfo.size(); ++i) {
        json columnInfoObject = columnsInfo[i];
        ColumnInfo columnInfo;
        columnInfo.precision = -1;
        columnInfo.isSetData = false;
        columnInfo.idSource = columnInfoObject["source"].get<std::string>().c_str();
        columnInfo.title = columnInfoObject["title"].get<std::string>().c_str();
        if (QString::compare(columnInfoObject["type"].get<std::string>().c_str(), "Value") == 0){
            columnInfo.type = ColumnInfo::Value;
            columnInfo.multiplier = columnInfoObject["multiplier"];
            columnInfo.precision = columnInfoObject["precision"];
        } else if (QString::compare(columnInfoObject["type"].get<std::string>().c_str(), "Time") == 0){
            columnInfo.type = ColumnInfo::Time;
        } else if (QString::compare(columnInfoObject["type"].get<std::string>().c_str(), "Message") == 0){
            columnInfo.type = ColumnInfo::Message;
        } else if (QString::compare(columnInfoObject["type"].get<std::string>().c_str(), "Unit") == 0){
            columnInfo.type = ColumnInfo::Unit;
        } else if (QString::compare(columnInfoObject["type"].get<std::string>().c_str(), "Id") == 0){
            columnInfo.type = ColumnInfo::Id;
        } else {
            columnInfo.type = ColumnInfo::Undef;
        }
        columns.append(columnInfo);
    }

    if (!fileSettings["isStartStopRecord"]){
        if(file->open(QIODevice::ReadWrite)){
            for (int i = 0; i < columns.size(); ++i) {
                file->write(columns[i].title.toUtf8());
                if (i != columns.size() - 1){
                    file->write(fileSettings["columnSeparator"].get<std::string>().c_str());
                } else {
                    file->write("\r\n");
                }
            }
        }
    }

    parameters_ = parameters;
}

void FileRecorder::addData(TerconData data)
{
    for (int i = 0; i < columns.size(); ++i) {
        if(columns[i].idSource == data.id || columns[i].idSource=="All"){
            if (columns[i].type == ColumnInfo::Value){
                if (columns[i].precision == -1){
                    columns[i].value = QString::number(data.value * columns[i].multiplier);
                } else {
                    columns[i].value = QString::number(data.value * columns[i].multiplier,'f',columns[i].precision);
                }
                columns[i].isSetData = true;
            }
            else if (columns[i].type == ColumnInfo::Time){
                columns[i].value = QString::number(data.time/1000.0,'f',3);
                columns[i].isSetData = true;
            }
            else if (columns[i].type == ColumnInfo::Message){
                columns[i].value = data.message;
                columns[i].isSetData = true;
            }
            else if (columns[i].type == ColumnInfo::Unit){
                columns[i].value = data.unit;
                columns[i].isSetData = true;
            }
            else if (columns[i].type == ColumnInfo::Id){
                columns[i].value = data.id;
                columns[i].isSetData = true;
            }
        }
    }

    int j;
    for (j = 0; j < columns.size(); ++j) {
        if (!columns[j].isSetData) break;
    }
    if (j == columns.size() && file->isOpen()){
        json fileSettings = parameters_["settings"];
        for (int i = 0; i < columns.size(); ++i) {
            columns[i].isSetData = false;
            file->write(columns[i].value.toUtf8());
            if (i != columns.size() - 1){
                file->write(fileSettings["columnSeparator"].get<std::string>().c_str());
            } else {
                file->write("\r\n");
                file->flush();
            }
        }
    }
}

void FileRecorder::writeLine(const QString & str)
{
    if (file->isOpen()){
        file->write(str.toUtf8());
        file->flush();
    }
}

void FileRecorder::startRecordExperiment(int count)
{
    json fileSettings = parameters_["settings"];
    if (fileSettings["isStartStopRecord"]){
        QDir currentDir(path_);
        QString experimentName = "Experiment_" + QString::number(count);
        currentDir.mkdir(experimentName);

        file->setFileName(path_ +"/"+experimentName+"/"+ fileSettings["fileName"].get<std::string>().c_str()
                + "." + fileSettings["fileExtension"].get<std::string>().c_str());
        if(file->open(QIODevice::ReadWrite)){
            for (int i = 0; i < columns.size(); ++i) {
                file->write(columns[i].title.toUtf8());
                if (i != columns.size() - 1){
                    file->write(fileSettings["columnSeparator"].get<std::string>().c_str());
                } else {
                    file->write("\r\n");
                }
            }
        }
    }
}

void FileRecorder::stopRecordExperiment()
{
    json fileSettings = parameters_["settings"];
    if (fileSettings["isStartStopRecord"]){
        file->close();
    }
}

QString FileRecorder::createPath()
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




/*
void DataRecorder::convertDataFile()
{
    QVector <QPair <QString,QString> > temperatureFurnace;
    QVector <QPair <QString,QString> > temperatureUpHeater;
    QVector <QPair <QString,QString> > temperatureDownHeater;
    QVector <QPair <QString,QString> > temperatureBlock;

    QVector <QPair <QString,QString> > temperatureSample;
    QVector <QPair <QString,QString> > resistance;
    QVector <QPair <QString,QString> > temperatureThermostat;
    QVector <QPair <QString,QString> > diffTemperatureThermostat;
    QVector <QPair <QString,QString> > calibrationHeaterVoltage;
    QVector <QPair <QString,QString> > calibrationHeaterCurrent;


    if(!dataFile || !dataFile->isOpen())
        return;
    dataFile->seek(0);

    const int time = 0;
    const int id = 1;
    const int value = 2;
    const QString temperatureUpHeaterId("{89349bc0-7eab-49db-b86c-047bac3915ef}");
    const QString temperatureFurnaceId("{ff98f69d-11cd-4553-8261-c338fe0e4a29}");
    const QString temperatureDownHeaterId("{f41b67da-dc68-4ee9-8d3c-b74f22368a05}");
    const QString temperatureBlockId("{1bab9ffd-68df-459a-b21b-de569a211232}");
    const QString SampleTemperatureId("{0986e158-6266-4d5e-8498-fa5c3cd84bbe}");
    const QString resistanceId("{41fada5a-879c-43ba-84d9-d565c0e03ead}");
    const QString temperatureThermostatId("{02cd3b3f-314b-4ad3-9da9-8acd0045a010}");
    const QString diffTemperatureThermostatId("{6fb4bc07-318a-4aaa-86c2-6a75d581a6b0}");
    const QString calibrationHeaterVoltageId("{32dc31df-366c-46b7-a9ed-d546a7824f56}");
    const QString calibrationHeaterCurrentId("{a566955d-d770-4b75-bf31-e218850f4efb}");

    QString line;
    do{
        line = dataFile->readLine();
        if (line.isEmpty()) break;

        QStringList valuesLine(line.split("\t"));
        if (valuesLine.size() < 3)break;


            if(valuesLine.at(id)==temperatureUpHeaterId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                temperatureUpHeater.append(pair);
            }
            else if(valuesLine.at(id)==temperatureFurnaceId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                temperatureFurnace.append(pair);
            }
            else if(valuesLine.at(id)==temperatureDownHeaterId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                temperatureDownHeater.append(pair);
            }
            else if(valuesLine.at(id)==temperatureBlockId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                temperatureBlock.append(pair);
            }
            else if(valuesLine.at(id)==SampleTemperatureId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                temperatureSample.append(pair);
            }


            else if(valuesLine.at(id)==resistanceId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                resistance.append(pair);
            }


            else if(valuesLine.at(id)==diffTemperatureThermostatId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                diffTemperatureThermostat.append(pair);
            }
            else if(valuesLine.at(id)==temperatureThermostatId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                temperatureThermostat.append(pair);
            }


            else if(valuesLine.at(id)==calibrationHeaterVoltageId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                calibrationHeaterVoltage.append(pair);
            }
            else if(valuesLine.at(id)==calibrationHeaterCurrentId){
                QPair <QString,QString> pair(valuesLine.at(time).trimmed(),valuesLine.at(value).trimmed());
                calibrationHeaterCurrent.append(pair);
            }


    }while (!line.isNull());

    line.clear();
    QFileInfo fileInfo(dataFile->fileName());
    QFile outFile(fileInfo.dir().absolutePath()+"/initData.txt");
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&outFile);
    out <<"time"<<"\t"<<"temperFurnace"<<"\t"
        <<"time"<<"\t"<<"temperBlockFurnace"<<"\t"
        <<"time"<<"\t"<<"temperUpFurnace"<<"\t"
        <<"time"<<"\t"<<"temperDownFurnace"<<"\t"

        <<"time"<<"\t"<<"temperSample"<<"\t"
        <<"time"<<"\t"<<"resistance"<<"\t"
        <<"time"<<"\t"<<"temperTherm"<<"\t"
        <<"time"<<"\t"<<"diffTemperTherm"<<"\t"
        <<"time"<<"\t"<<"calHeaterV"<<"\t"
        <<"time"<<"\t"<<"calHeaterI"<<"\t"<<"\n";

    bool isAvailableData;
    for(int i =0;;++i){
        isAvailableData = false;
        line.clear();
        if(i<temperatureFurnace.size()){
            line.append(temperatureFurnace.at(i).first+"\t"
                        +temperatureFurnace.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }

        if(i<temperatureBlock.size()){
            line.append(temperatureBlock.at(i).first+"\t"
                        +temperatureBlock.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }

        if(i<temperatureUpHeater.size()){
            line.append(temperatureUpHeater.at(i).first+"\t"
                        +temperatureUpHeater.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }

        if(i<temperatureDownHeater.size()){
            line.append(temperatureDownHeater.at(i).first+"\t"
                        +temperatureDownHeater.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }

        if(i<temperatureSample.size()){
            line.append(temperatureSample.at(i).first+"\t"
                        +temperatureSample.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }
        if(i<resistance.size()){
            line.append(resistance.at(i).first+"\t"
                        +resistance.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }
        if(i<temperatureThermostat.size()){
            line.append(temperatureThermostat.at(i).first+"\t"
                        +temperatureThermostat.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }
        if(i<diffTemperatureThermostat.size()){
            line.append(diffTemperatureThermostat.at(i).first+"\t"
                        +diffTemperatureThermostat.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }
        if(i<calibrationHeaterVoltage.size()){
            line.append(calibrationHeaterVoltage.at(i).first+"\t"
                        +calibrationHeaterVoltage.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }
        if(i<calibrationHeaterCurrent.size()){
            line.append(calibrationHeaterCurrent.at(i).first+"\t"
                        +calibrationHeaterCurrent.at(i).second+"\t");
            isAvailableData= true;
        }else{
            line.append("\t\t");
        }
        out<<line<<"\n";
        if (!isAvailableData)
            break;
    }

    outFile.close();
    QMessageBox::information(0, tr("Калориметр"), tr("Данные успешно конвертированны"));

}*/
