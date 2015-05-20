#ifndef REGULATORPARAMETERS_H
#define REGULATORPARAMETERS_H

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

#endif // REGULATORPARAMETERS_H
