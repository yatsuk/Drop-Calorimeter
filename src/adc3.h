#ifndef ADC_H
#define ADC_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include <QVector>
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

private slots:
    void recvData();

private:
    QTime * m_Time;

    double moveAverage(const QVector <double> &valueArray);
    long double convertVolt2TemperatureTypeA1(double value);
    long double convertTemperature2VoltTypeA1(double temperature);

    const static int averageCount = 20;
    QVector <double> sourcesDataCh1;
    QVector <double> averageDataCh1;
    QVector <double> sourcesDataCh2;
    QVector <double> averageDataCh2;
    double roomTemperature;
    double offsetVolt;
    const static double offsetZero = 0.004;
    const static double scale = 0.99976;

    TLTR27 * ltr27;
    QTimer * recvTimer;
    const static int SamplesCount = 50;
    DWORD buf[SamplesCount];
    double dataArray[SamplesCount];
};

#endif // ADC_H
