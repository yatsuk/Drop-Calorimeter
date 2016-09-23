#ifndef TERCONDATA_H
#define TERCONDATA_H

#include <QChar>
#include <QString>
#include <QMetaType>

struct TerconData
{
    double value;
    QString unit;
    QString id;
    QString message;
    int time;

    static const int virtualDeviceNumber = 100;
};

Q_DECLARE_METATYPE(TerconData)

#endif // TERCONDATA_H
