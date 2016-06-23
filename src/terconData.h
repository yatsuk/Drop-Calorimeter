#ifndef TERCONDATA_H
#define TERCONDATA_H

#include <QChar>
#include <QString>

class TerconData
{
public:
    TerconData();

    double value;
    QString unit;
    QString id;
    QString message;
    int time;

    static const int virtualDeviceNumber = 100;
};

#endif // TERCONDATA_H
