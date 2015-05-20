#ifndef DATARECORDER_H
#define DATARECORDER_H

#include <QObject>
#include <QFile>
#include "shared.h"

class DataRecorder : public QObject
{
    Q_OBJECT
public:
    explicit DataRecorder(QObject *parent = 0);
    ~DataRecorder();
    
signals:
    
public slots:
    void writeFile(const QString & data,Shared::FileType fileType);
    void beginRecord();
    void endRecord();

private:
    QString createPath();
    void createFiles(const QString &path);
    QString path_;
    bool recordEnabled;
    QFile * logFile;
    QFile * dataFile;
    QFile * regulatorFurnaceFile;
    QFile * regulatorThermostatFile;
    QFile * regulatorUpHeaterFile;
    QFile * regulatorDownHeaterFile;
    QFile * mainSignalsFile;
    QFile * thermostatSignalsFile;
    QFile * calibrationHeaterFile;
};

#endif // DATARECORDER_H
