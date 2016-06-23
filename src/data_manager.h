#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include "dataRecorder.h"
#include "terconData.h"
#include "filter.h"


class DataManager : public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent = 0);
    ~DataManager();

signals:
    void dataSend(TerconData data);

public slots:
    void addData(TerconData data);
    void addLogMessage(const QString & message);
    bool setSettings(const QJsonObject &settings);
    void startRecordExperimentFile();
    void stopRecordExperimentFile();

private:
    QVector <Filter *> filters;
    QVector <DataRecorder *> dataRecorders;
    int experimentStartStopCount;
};

#endif // DATAMANAGER_H
