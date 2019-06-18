#ifndef FILTER_H
#define FILTER_H

#include <QObject>
#include <include/externals/nlohmann/json/json.hpp>
#include <QMap>
#include "shared.h"
#include "terconData.h"

using json = nlohmann::json;

struct OutputData
{
    QString id;
    QString name;
    double value;
    QString unit;
};

class Filter : public QObject
{
    Q_OBJECT
public:
    explicit Filter(QObject *parent = nullptr);
    ~Filter();

signals:
    void dataSend(TerconData data);
    void message(const QString & msg, Shared::MessageLevel msgLevel);

public slots:
    virtual void setParameters(const json &parameters);
    virtual json getSetting(){return parameters_;}
    static  Filter * createFilterFromJSON(const json &parameters);
    void addData(TerconData data);

protected:
    json parameters_;
    QMap <QString, OutputData> outputDataMap_;
    virtual void receive(TerconData){}
};


class MovingAverage: public Filter
{
public:
    void setParameters(const json &parameters);

protected:
    virtual void receive(TerconData data);

private:
    enum Type {Exponential, Undef};
    Type type;
    int averageCount;
    double alpha;
    double lastEmaValue;
    bool firstValue_;
};

class TermocoupleConverter: public Filter
{

public:
    void setParameters(const json &parameters);

protected:
    virtual void receive(TerconData data);

private:
    enum Type {A1, S, K, Undef};
    Type type;
    double coldVoltage;

    double voltageToTemperatureTypeA1 (double voltage);
    double temperatureToVoltageTypeA1 (double temperature);

    double voltageToTemperatureTypeS (double voltage);
    double temperatureToVoltageTypeS (double temperature);

    double voltageToTemperatureTypeK (double voltage);
    double temperatureToVoltageTypeK (double temperature);

    bool isConstColdTemperature;
    bool isSetColdVoltage = false;
};

#endif // FILTER_H
