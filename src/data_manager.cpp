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

bool DataManager::setSettings(const json &settings)
{
    if(settings.empty()) return false;
    json filtersArray = settings["DataManager"]["filters"];
    for (unsigned int i = 0; i < filtersArray.size(); ++i) {
        json filterObject = filtersArray[i];
        if (filterObject.empty())continue;
        Filter * filter = Filter::createFilterFromJSON(filterObject);
        if (filter){
            filters.append(filter);
            connect (filter,SIGNAL(dataSend(TerconData)),this,SIGNAL(dataSend(TerconData)));
            connect (filter,SIGNAL(dataSend(TerconData)),this,SLOT(addData(TerconData)));
        }
    }

    json storage = settings["DataManager"]["storage"];
    json recordsArray = storage["records"];
    for (unsigned int i = 0; i < recordsArray.size(); ++i) {
        json recordObject = recordsArray[i];
        if (recordObject.empty())continue;
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
