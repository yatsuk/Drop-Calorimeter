#ifndef TERCONDATA_H
#define TERCONDATA_H

#include <QChar>
#include <QString>

class TerconData
{
public:
    TerconData();
    TerconData(int devNumber,int devChannel,double devValue):
        deviceNumber(devNumber),
        channel(devChannel),
        value(devValue)
    {}

    int deviceNumber;
    short channel;
    double value;
    QChar unit;  
    int time;

    static const int virtualDeviceNumber = 100;
};

#endif // TERCONDATA_H
