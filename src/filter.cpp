#include "filter.h"
#include <math.h>
#include <QDebug>


Filter::Filter(QObject *parent) : QObject(parent)
{

}

Filter::~Filter()
{

}

Filter * Filter::createFilterFromJSON(const json &parameters)
{
    Filter * filter = nullptr;
    if (!parameters.empty()){
        if (parameters["type"].get<std::string>()=="Thermocouple"){
            filter = new TermocoupleConverter;
        } else if (parameters["type"].get<std::string>()=="MA"){
            filter = new MovingAverage;
        }
        if (filter){
            filter->setObjectName(parameters["id"].get<std::string>().c_str());
            filter->setSetting(parameters);
        }
    }
    return filter;
}

void Filter::addData(TerconData data)
{
    json settings = parameters_["settings"];
    if (QString::compare(settings["sourceId"].get<std::string>().c_str(), data.id) == 0){
        TerconData newData;
        newData.id = settings["newId"].get<std::string>().c_str();
        newData.time = data.time;
        bool ok;
        newData.value = receive(data, &ok);
        if(!ok)
            return;

        newData.unit = settings["unit"].get<std::string>().c_str();
        emit dataSend(newData);
    }
}







double MovingAverage::receive(TerconData data, bool * ok)
{
    if (type==Exponential){
        if(ok)*ok=true;
        return emaValue(data);
    }
    if(ok)*ok=false;
    return 0;
}

void MovingAverage::setSetting(const json &parameters)
{
    json averageSettings = parameters["settings"];
    type = Undef;
    averageCount = 1;
    if (averageSettings["type"].get<std::string>()=="exponential"){
        type = Exponential;
        averageCount = averageSettings["averageCount"];
        alpha = 2.0 / (averageCount + 1.0);
    }

    parameters_ = parameters;
    firstValue_ = true;
}

double MovingAverage::emaValue(TerconData newValue)
{
    if (firstValue_){
        lastEmaValue = newValue.value;
        firstValue_ = false;
        return newValue.value;
    }
    double ema = alpha*newValue.value + (1.0 - alpha)*lastEmaValue;
    lastEmaValue = ema;
    return ema;
}




double TermocoupleConverter::receive(TerconData data, bool * ok)
{
    if (type==S){
        if(ok)*ok=true;
        return voltageToTemperatureTypeS(data.value + coldVoltage);
    } else if (type==A1){
        if(ok)*ok=true;
        return voltageToTemperatureTypeA1(data.value + coldVoltage);
    }
    if(ok)*ok=false;
    return 0;
}

void TermocoupleConverter::setSetting(const json &parameters)
{
    json thermocoupleSettings = parameters["settings"];
    type = Undef;
    coldVoltage = 0;
    if (thermocoupleSettings["type"].get<std::string>()=="S"){
        type = S;
        coldVoltage = temperatureToVoltageTypeS(thermocoupleSettings["coldTemperature"]);
    } else if (thermocoupleSettings["type"].get<std::string>()=="A1"){
        type = A1;
        coldVoltage = temperatureToVoltageTypeA1(thermocoupleSettings["coldTemperature"]);
    }

    parameters_ = parameters;
}

double TermocoupleConverter::voltageToTemperatureTypeA1 (double voltage)
{
    voltage=voltage*1e+3;
    long double temperature=0.9643027
            + 79.495086*voltage
            - 4.9990310*pow(voltage,2)
            + 0.6341776*pow(voltage,3)
            - 4.7440967e-2*pow(voltage,4)
            + 2.1811337e-3*pow(voltage,5)
            - 5.8324228e-5*pow(voltage,6)
            + 8.2433725e-7*pow(voltage,7)
            - 4.5928480e-9*pow(voltage,8);
    return temperature;
}

double TermocoupleConverter::temperatureToVoltageTypeA1 (double temperature)
{
    long double volt =7.1564735E-04
            + 1.1951905E-02*temperature
            + 1.6672625E-05*pow(temperature,2)
            - 2.8287807E-08*pow(temperature,3)
            + 2.8397839E-11*pow(temperature,4)
            - 1.8505007E-14*pow(temperature,5)
            + 7.3632123E-18*pow(temperature,6)
            - 1.6148878E-21*pow(temperature,7)
            + 1.4901679E-25*pow(temperature,8);
    return volt*1e-3;
}

double TermocoupleConverter::voltageToTemperatureTypeS (double voltage)
{
    voltage=voltage*1e+3;
    long double temperature=0;
    if (voltage < 1.874){
        temperature+=1.8494946E+02*voltage;
        temperature-=8.00504062E+01*pow(voltage,2);
        temperature+=1.0223743E+02*pow(voltage,3);
        temperature-=1.52248592E+02*pow(voltage,4);
        temperature+=1.88821343E+02*pow(voltage,5);
        temperature-=1.59085941E+02*pow(voltage,6);
        temperature+=8.2302788E+01*pow(voltage,7);
        temperature-=2.34181944E+01*pow(voltage,8);
        temperature+=2.7978626E+00*pow(voltage,9);
    }
    else if ((voltage >= 1.874) && (voltage < 10.332)){
        temperature+=1.291507177E+01;
        temperature+=1.466298863E+02*voltage;
        temperature-=1.534713402E+01*pow(voltage,2);
        temperature+=3.145945973E+00*pow(voltage,3);
        temperature-=4.163257839E-01*pow(voltage,4);
        temperature+=3.187963771E-02*pow(voltage,5);
        temperature-=1.2916375E-03*pow(voltage,6);
        temperature+=2.183475087E-05*pow(voltage,7);
        temperature-=1.447379511E-07*pow(voltage,8);
        temperature+=8.211272125E-09*pow(voltage,9);

    } else if( voltage >= 10.332){
        temperature-=8.087801117E+01;
        temperature+=1.621573104E+02*voltage;
        temperature-=8.536869453E+00*pow(voltage,2);
        temperature+=4.719686976E-01*pow(voltage,3);
        temperature-=1.441693666E-02*pow(voltage,4);
        temperature+=2.08161889E-04*pow(voltage,5);
    }
    return temperature;
}

double TermocoupleConverter::temperatureToVoltageTypeS (double temperature)
{
    long double voltage=0;
    if (temperature < 1064.18){
        voltage+=5.40313308631E-03*temperature;
        voltage+=1.25934289740E-05*pow(temperature,2);
        voltage-=2.32477968689E-08*pow(temperature,3);
        voltage+=3.22028823036E-11*pow(temperature,4);
        voltage-=3.31465196389E-14*pow(temperature,5);
        voltage+=2.55744251786E-17*pow(temperature,6);
        voltage-=1.25068871393E-20*pow(temperature,7);
        voltage+=2.71443176145E-24*pow(temperature,8);
    }
    else if ((temperature >= 1064.18) && (temperature < 1664.5)){
        voltage+=1.32900444085;
        voltage+=3.34509311344E-03*temperature;
        voltage+=6.54805192818E-06*pow(temperature,2);
        voltage-=1.64856259209E-09*pow(temperature,3);
        voltage+=1.29989605174E-14*pow(temperature,4);
    } else if( temperature >= 1664.5){
        voltage+=1.46628232636E+02;
        voltage-=2.58430516752E-01*temperature;
        voltage+=1.63693574641E-04*pow(temperature,2);
        voltage-=3.30439046987E-08*pow(temperature,3);
        voltage-=9.43223690612E-18*pow(temperature,4);
    }
    return voltage*1e-3;
}
