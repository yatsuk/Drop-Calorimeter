#ifndef FILTER_H
#define FILTER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include "shared.h"
#include "terconData.h"

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
    virtual void setSetting(const QJsonObject &parameters){parameters_ = parameters;}
    virtual QJsonObject getSetting(){return parameters_;}
    static  Filter * createFilterFromJSON(const QJsonObject &parameters);
    void addData(TerconData data);

protected:
    QJsonObject parameters_;
    virtual double receive(TerconData , bool * ok = 0){return 0;if(ok)*ok=false;}
};


class MovingAverage: public Filter
{
public:
    void setSetting(const QJsonObject &parameters);

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

class BlowoutRemover: public Filter
{
public:
    void setSetting(const QJsonObject &parameters);

protected:
    virtual double receive(TerconData data, bool * ok = 0);

private:
    MovingAverage movingAverage;

};



class TermocoupleConverter: public Filter
{

public:
    void setSetting(const QJsonObject &parameters);

protected:
    virtual double receive(TerconData data, bool * ok = 0);

private:
    enum Type {A1, S, Undef};
    Type type;
    double coldVoltage;

    double voltageToTemperatureTypeA1 (double voltage);
    double temperatureToVoltageTypeA1 (double temperature);

    double voltageToTemperatureTypeS (double voltage);
    double temperatureToVoltageTypeS (double temperature);
};

#endif // FILTER_H
