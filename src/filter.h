#ifndef FILTER_H
#define FILTER_H

#include <QObject>
#include <include/externals/nlohmann/json/json.hpp>
#include <QMap>
#include "shared.h"
#include "terconData.h"

using json = nlohmann::json;

class Filter : public QObject
{
    Q_OBJECT
public:
    explicit Filter(QObject *parent = 0);
    ~Filter();

signals:
    void dataSend(TerconData data);
    void message(const QString & msg, Shared::MessageLevel msgLevel);

public slots:
    virtual void setSetting(const json &parameters){parameters_ = parameters;}
    virtual void emitData(TerconData data);
    virtual json getSetting(){return parameters_;}
    static  Filter * createFilterFromJSON(const json &parameters);
    void addData(TerconData data);

protected:
    json parameters_;
    virtual double receive(TerconData , bool * ok = 0){return 0;if(ok)*ok=false;}
};


class MovingAverage: public Filter
{
public:
    void setSetting(const json &parameters);

protected:
    virtual double receive(TerconData data, bool * ok = 0);

private:
    double emaValue(TerconData newValue);
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
    void setSetting(const json &parameters);

protected:
    virtual double receive(TerconData data, bool * ok = nullptr);
    virtual void emitData(TerconData data);

private:
    enum Type {A1, S, Undef};
    Type type;
    double coldVoltage;

    double voltageToTemperatureTypeA1 (double voltage);
    double temperatureToVoltageTypeA1 (double temperature);

    double voltageToTemperatureTypeS (double voltage);
    double temperatureToVoltageTypeS (double temperature);

    bool isConstColdTemperature;
    bool isSetColdVoltage = false;
};

class ResistanceThermometerConverter: public Filter
{

public:
    void setSetting(const json &parameters);

protected:
    virtual double receive(TerconData data, bool * ok = nullptr);

private:
    enum Type {Pt100, Undef};
    Type type;

    double resistanceToTemperaturePt100 (double resistance);
};

#endif // FILTER_H
