#ifndef ADC_H
#define ADC_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include <QVector>
#include "parameters.h"
#include "shared.h"
#include "ltr27api.h"
#include "terconData.h"


class ADC : public QObject
{
    Q_OBJECT
public:
    explicit ADC(QObject *parent = 0);
    ~ADC();
    void setTime(QTime * time);

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void dataSend(TerconData data);

public slots:
    void startADC();
    void stopADC();
    void initializationLTR27();
    //void setParameters(AdcParameters parameters);
    //AdcParameters parameters();

private slots:
    void recvData();

private:
    AdcParameters parameters_;
    RegulatorParameters regparam;
    QTime * m_Time;

    double moveAverage(const QVector <double> &valueArray);
    long double convertVolt2TemperatureTypeA1(double value);
    long double convertTemperature2VoltTypeA1(double temperature);
    long double convertVolt2TemperatureTypeK(double value);
    long double convertTemperature2VoltTypeK(double temperature);

    const static double maxDeltaCurrentPrevValue = 0.1;//mVolt
    QVector <double> sourcesDataCh1;
    QVector <double> averageDataCh1;
    QVector <double> sourcesDataCh2;
    QVector <double> averageDataCh2;
    double roomTemperature;
    double offsetVoltRoomTemperature;
    const static double offsetZero = 0.004;
    const static double scale = 0.99976;
    bool firstValue;

    TLTR27 * ltr27;
    QTimer * recvTimer;
    const static int SamplesCount = 16;
    DWORD buf[SamplesCount];
    double dataArray[SamplesCount];
};

#endif // ADC_H
