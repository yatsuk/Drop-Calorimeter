#ifndef SIGNALSVIEW_H
#define SIGNALSVIEW_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QString>
#include <QVector>
#include <QVBoxLayout>
#include <QJsonObject>
#include <GUI\qledindicator.h>
#include "terconData.h"

class FurnaceSignalsView;
class HeaterSignalsView;
class SignalsView : public QWidget
{
    Q_OBJECT
public:
    explicit SignalsView(QWidget *parent = 0);
    void addSignal(const QString & signalName);

    
signals:
    
public slots:   
    void addValue (TerconData terconData);
    void updateState(const QJsonObject & json);

private:
    QVBoxLayout * layout;
    QVector <QLabel *> labels;
    FurnaceSignalsView * furnaceSignalsView;

};

class FurnaceSignalsView : public QGroupBox
{
    Q_OBJECT
public:
    explicit FurnaceSignalsView(QWidget *parent = 0);
public slots:
    void updateState(const QJsonObject & json);
    void setTemperature(TerconData terconData);

private:   
    HeaterSignalsView * mainHeater;
    HeaterSignalsView * upHeater;
    HeaterSignalsView * downHeater;
    QLabel * sampleTemperatureLabel;
    QLabel * furnaceInertBlockTemperatureLabel;
    QLabel * sampleTemperatureValueLabel;
    QLabel * furnaceInertBlockTemperatureValueLabel;
};

class HeaterSignalsView : public QWidget
{
    Q_OBJECT
public:
    explicit HeaterSignalsView(const QString & heaterName, QWidget *parent = 0);
public slots:
    void updateState(const QJsonObject & json);
    void setTemperature(double temperature);

public slots:
    void progressBarClicked();

private:
    void changeColorDeltaTemperatureLabel(double deltaTemperature);
    void setProgressBarText();

    QString name_;

    QColor defaultColorOn1LegIndicator;
    QColor defaultColorOn2LegIndicator;
    QColor defaultColorOff1LegIndicator;
    QColor defaultColorOff2LegIndicator;

    QLedIndicator * statusHeaterLed;
    QGroupBox * heaterGroupBox;
    QLabel * heaterLabel;
    QLabel * temperatureLabel;
    QLabel * regulatorMode;
    QLabel * outPower;
    QLabel * setPoint;
    QLabel * delta;
    QLabel * segmentInfoLabel;
    QProgressBar * progressBar;
    double durationTimeProgress;
    double elapsedTimeProgress;
    bool leftTimeProgressBarViewMode;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // SIGNALSVIEW_H
