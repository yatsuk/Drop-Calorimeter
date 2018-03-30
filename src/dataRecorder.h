#ifndef DATARECORDER_H
#define DATARECORDER_H

#include <QObject>
#include <include/externals/nlohmann/json/json.hpp>
#include <QVector>
#include <QFile>
#include <QFileInfo>
#include "terconData.h"
#include "shared.h"

using json = nlohmann::json;

class ColumnInfo;

class DataRecorder : public QObject
{
    Q_OBJECT
public:
    explicit DataRecorder(QObject *parent = 0);
    ~DataRecorder();

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);

public slots:
    virtual void setSetting(const json &parameters){parameters_ = parameters;}
    virtual json getSetting(){return parameters_;}
    virtual void startRecordExperiment(int){}
    virtual void stopRecordExperiment(){}
    static  DataRecorder * createDataRecorderFromJSON(const json &parameters);
    virtual void addData(TerconData){}
    virtual void writeLine(const QString &){}

protected:
    json parameters_;
};


class FileRecorder : public DataRecorder
{
public:
    FileRecorder();
    ~FileRecorder();
    void setSetting(const json &parameters);
    void addData(TerconData data);
    void writeLine(const QString & str);
    void startRecordExperiment(int count);
    void stopRecordExperiment();

private:
    QFile * file;
    QString createPath();
    static QString path_ ;
    QVector <ColumnInfo> columns;
};

class ColumnInfo
{
public:
    enum ValueType{Value, Time, Message, Unit, Id, Undef};
    QString idSource;
    QString title;
    ValueType type;
    bool isSetData;
    double multiplier;
    int precision;
    QString value;
};


#endif // DATARECORDER_H
