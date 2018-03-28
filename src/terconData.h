#ifndef TERCONDATA_H
#define TERCONDATA_H

#include <QString>
#include <QMetaType>

struct TerconData
{
    double value;
    QString unit;
    QString id;
    QString message;
    int time;
};

Q_DECLARE_METATYPE(TerconData)

#endif // TERCONDATA_H
