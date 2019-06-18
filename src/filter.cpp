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
        } else if (parameters["type"].get<std::string>()=="FakeData"){
            filter = new FakeData;
        }

        if (filter){
            filter->setObjectName(parameters["id"].get<std::string>().c_str());
            filter->setParameters(parameters);
        }
    }
    return filter;
}

void Filter::setParameters(const json &parameters)
{
    parameters_ = parameters;
    for(auto & outputDataJson : parameters_["settings"]["outputData"]){
        OutputData outputData;
        outputData.id = outputDataJson["id"].get<std::string>().c_str();
        outputData.name = outputDataJson["name"].get<std::string>().c_str();
        outputData.unit = outputDataJson["unit"].get<std::string>().c_str();
        outputDataMap_[outputData.name] = outputData;
    }
    OutputData outputData;

}

void Filter::addData(TerconData data)
{
    for(auto & sourceId : parameters_["settings"]["sourceId"])
        if (QString::compare(sourceId.get<std::string>().c_str(), data.id) == 0)
            receive(data);
}





void MovingAverage::receive(TerconData data)
{
    TerconData newData;
    newData.time = data.time;

    if (type==Exponential){
        if (firstValue_){
            lastEmaValue = data.value;
            firstValue_ = false;
            if(outputDataMap_.contains("emaValue")){
                newData.id = outputDataMap_["emaValue"].id;
                newData.unit = outputDataMap_["emaValue"].unit;
                newData.value = data.value;
                emit dataSend(newData);
            }
        }
        double ema = alpha*data.value + (1.0 - alpha)*lastEmaValue;
        lastEmaValue = ema;
        newData.value = ema;
        if(outputDataMap_.contains("emaValue")){
            newData.id = outputDataMap_["emaValue"].id;
            newData.unit = outputDataMap_["emaValue"].unit;
            emit dataSend(newData);
        }
    }

}

void MovingAverage::setParameters(const json &parameters)
{
    Filter::setParameters(parameters);
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




void TermocoupleConverter::receive(TerconData data)
{
    if(!isConstColdTemperature){
        if(QString::compare(parameters_["settings"]["coldTemperature"].get<std::string>().c_str(), data.id) == 0){
            if (type==S){
                coldVoltage = temperatureToVoltageTypeS(data.value);
                isSetColdVoltage = true;
            } else if (type==A1){
                coldVoltage = temperatureToVoltageTypeA1(data.value);
                isSetColdVoltage = true;
            } else if (type==K){
                coldVoltage = temperatureToVoltageTypeK(data.value);
                isSetColdVoltage = true;
            }
            return;
        }
    }

    if(!isSetColdVoltage && !isConstColdTemperature)return;


    TerconData newData;
    newData.time = data.time;

    if(outputDataMap_.contains("thermocoupleValueMVolt")){
        newData.value = data.value + coldVoltage;
        newData.id = outputDataMap_["thermocoupleValueMVolt"].id;
        newData.unit = outputDataMap_["thermocoupleValueMVolt"].unit;
        emit dataSend(newData);
    }

    if (type==S){
        if(outputDataMap_.contains("thermocoupleValue")){
            newData.value = voltageToTemperatureTypeS(data.value + coldVoltage);
            newData.id = outputDataMap_["thermocoupleValue"].id;
            newData.unit = outputDataMap_["thermocoupleValue"].unit;
            emit dataSend(newData);
        }
    } else if (type==A1){
        if(outputDataMap_.contains("thermocoupleValue")){
            newData.value = voltageToTemperatureTypeA1(data.value + coldVoltage);
            newData.id = outputDataMap_["thermocoupleValue"].id;
            newData.unit = outputDataMap_["thermocoupleValue"].unit;
            emit dataSend(newData);
        }
    } else if (type==K){
        if(outputDataMap_.contains("thermocoupleValue")){
            newData.value = voltageToTemperatureTypeK(data.value + coldVoltage);
            newData.id = outputDataMap_["thermocoupleValue"].id;
            newData.unit = outputDataMap_["thermocoupleValue"].unit;
            emit dataSend(newData);
        }
    }
}

void TermocoupleConverter::setParameters(const json &parameters)
{
    Filter::setParameters(parameters);
    json thermocoupleSettings = parameters["settings"];
    type = Undef;
    coldVoltage = 0;
    isConstColdTemperature = thermocoupleSettings["coldTemperature"].is_number();
    if (thermocoupleSettings["type"].get<std::string>()=="S"){
        type = S;
        if (isConstColdTemperature){
            coldVoltage = temperatureToVoltageTypeS(thermocoupleSettings["coldTemperature"]);
        }
    } else if (thermocoupleSettings["type"].get<std::string>()=="A1"){
        type = A1;
        if (isConstColdTemperature){
            coldVoltage = temperatureToVoltageTypeA1(thermocoupleSettings["coldTemperature"]);
        }
    } else if (thermocoupleSettings["type"].get<std::string>()=="K"){
        type = K;
        if (isConstColdTemperature){
            coldVoltage = temperatureToVoltageTypeK(thermocoupleSettings["coldTemperature"]);
        }
    }

    parameters_ = parameters;
}

double TermocoupleConverter::voltageToTemperatureTypeA1 (double voltage)
{
    voltage=voltage*1e+3;
    double temperature=0.9643027
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
    double volt =7.1564735E-04
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
    double temperature=0;
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
    double voltage=0;
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

double TermocoupleConverter::voltageToTemperatureTypeK (double voltage)
{
    voltage=voltage*1e+3;
    double temperature=25.08355*voltage
            +7.860106E-2*pow(voltage,2)
            -2.503131E-1*pow(voltage,3)
            +8.315270E-2*pow(voltage,4)
            -1.228034E-2*pow(voltage,5)
            +9.804036E-4*pow(voltage,6)
            -4.413030E-5*pow(voltage,7)
            +1.057734E-6*pow(voltage,8)
            -1.052755E-8*pow(voltage,9);
    return temperature;
}

double TermocoupleConverter::temperatureToVoltageTypeK (double temperature)
{
    double volt =-1.7600413686E-2
            + 3.8921204975E-2*temperature
            + 1.8558770032E-5*pow(temperature,2)
            - 9.9457592874E-8*pow(temperature,3)
            + 3.1840945719E-10*pow(temperature,4)
            - 5.6072844889E-13*pow(temperature,5)
            + 5.6075059059E-16*pow(temperature,6)
            - 3.2020720003E-19*pow(temperature,7)
            + 9.7151147152E-23*pow(temperature,8)
            - 1.2104721275E-26*pow(temperature,9)
            + 1.185976E-1*exp(-1.183432E-4 * pow((temperature - 126.9686),2));
    return volt*1e-3;
}


void FakeData::setParameters(const json &parameters)
{
    Filter::setParameters(parameters);
    value_ = parameters_["settings"]["value"].get<double>();
}

void FakeData::receive(TerconData data)
{
    TerconData newData;
    newData.time = data.time;

    if(outputDataMap_.contains("fakeValue")){
        newData.value = value_;
        newData.id = outputDataMap_["fakeValue"].id;
        newData.unit = outputDataMap_["fakeValue"].unit;
        emit dataSend(newData);
    }
}

