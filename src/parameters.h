#ifndef REGULATORPARAMETERS_H
#define REGULATORPARAMETERS_H

#include <QString>

class RegulatorParameters
{

public:

    double minPower;
    double maxPower;
    double gI;
    double gP;
    double gD;
    double offset;
    double procentPerSec;
    double maxIntegralValue;
    double maxProportionalValue;
    int averageCount;
    int averagePowerCount;
    
};

class AdcParameters
{

public:

    double roomTemperature;
    bool filter;
    int averageCount;
    QString thermocoupleType;
    QString portName;

};

#endif // REGULATORPARAMETERS_H
