#include "data_manager.h"

DataManager::DataManager(QObject *parent) :
    QObject(parent),
    experimentStartStopCount(1)
{

}

DataManager::~DataManager()
{
    for (int i = 0; i < filters.size(); ++i)
        delete filters[i];
    for (int i = 0; i < dataRecorders.size(); ++i)
        delete dataRecorders[i];
}

void DataManager::addData(TerconData data)
{
    for (int i = 0; i < dataRecorders.size(); ++i)
        dataRecorders[i]->addData(data);
    for (int i = 0; i < filters.size(); ++i)
        filters[i]->addData(data);

}

bool DataManager::setSettings(const QJsonObject &settings)
{
    if(settings.isEmpty()) return false;
    QJsonArray filtersArray = settings["DataManager"].toObject()["filters"].toArray();
    for (int i = 0; i < filtersArray.size(); ++i) {
        QJsonObject filterObject = filtersArray[i].toObject();
        if (filterObject.isEmpty())continue;
        Filter * filter = Filter::createFilterFromJSON(filterObject);
        if (filter){
            filters.append(filter);
            connect (filter,SIGNAL(dataSend(TerconData)),this,SIGNAL(dataSend(TerconData)));
            connect (filter,SIGNAL(dataSend(TerconData)),this,SLOT(addData(TerconData)));
        }
    }

    QJsonObject storage = settings["DataManager"].toObject()["storage"].toObject();
    QJsonArray recordsArray = storage["records"].toArray();
    for (int i = 0; i < recordsArray.size(); ++i) {
        QJsonObject recordObject = recordsArray[i].toObject();
        if (recordObject.isEmpty())continue;
        DataRecorder * dataRecorder = DataRecorder::createDataRecorderFromJSON(recordObject);
        if (dataRecorder){
            dataRecorders.append(dataRecorder);
        }
    }

    return true;
}

void DataManager::addLogMessage(const QString & message)
{
    for (int i = 0; i < dataRecorders.size(); ++i)
        if(dataRecorders[i]->objectName() == "{1230f469-45ac-416b-affd-114407d76e40}"){
            dataRecorders[i]->writeLine(message);
            break;
        }
}

void DataManager::startRecordExperimentFile()
{
    for (int i = 0; i < dataRecorders.size(); ++i)
        dataRecorders[i]->startRecordExperiment(experimentStartStopCount);

    experimentStartStopCount++;
}

void DataManager::stopRecordExperimentFile()
{
    for (int i = 0; i < dataRecorders.size(); ++i)
        dataRecorders[i]->stopRecordExperiment();
}
