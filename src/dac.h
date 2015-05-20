#ifndef DAC_H
#define DAC_H

#include <QObject>
#include "ltrapi.h"
#include "shared.h"
#include "ltr34api.h"

class DAC : public QObject
{
    Q_OBJECT
public:
    explicit DAC(QObject *parent = 0);
    ~DAC();
    
signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    
public slots:
    void startDAC();
    void stopDAC(int channelNumber);
    void setValueDAC(double value,int channelNumber);
    void initializationLTR();
    void initializationLTR34();

private:
    TLTR * ltr;
    TLTR34 * ltr34;
    double * code;
    DWORD * arrayToSend;
    int ltrError;
    int ltr34Error;
    const static uint sizeArray = 4;
    bool isStarted;
};

#endif // DAC_H
