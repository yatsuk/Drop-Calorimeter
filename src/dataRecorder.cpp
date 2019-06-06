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
