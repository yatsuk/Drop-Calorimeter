#ifndef SIGNALSVIEW_H
#define SIGNALSVIEW_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QString>
#include <QVector>
#include <QVBoxLayout>
#include <include/externals/nlohmann/json/json.hpp>
#include <include/externals/qledindicator/qledindicator.h>
#include "src/terconData.h"
#include "widgetRegulatorFurnace.h"
#include "additionalHeatersWidget.h"

using json = nlohmann::json;

class HeaterSignalsView;

class FurnaceSignalsView : public QGroupBox
{
    Q_OBJECT
public:
    explicit FurnaceSignalsView(QWidget *parent = 0);

public slots:
    void updateState(const json & state);
    void setTemperature(TerconData terconData);

private slots:
    void showMainRegulatorSettingsWidget();
    void showAdditionalRegulatorSettingsWidget();

private:   
    HeaterSignalsView * mainHeater;
    HeaterSignalsView * upHeater;
    HeaterSignalsView * downHeater;
    QLabel * sampleTemperatureLabel;
    QLabel * furnaceInertBlockTemperatureLabel;
    QLabel * sampleTemperatureValueLabel;
    QLabel * furnaceInertBlockTemperatureValueLabel;
    WidgetRegulatorFurnace * widgetRegulatorFurnace;
    AdditionalHeatersWidget * additionalHeatersWidget;
};

class CalorimeterSignalsView : public QGroupBox
{
    Q_OBJECT
public:
    explicit CalorimeterSignalsView(QWidget *parent = 0);

public slots:
    void updateState(const json & state);
    void setValue(TerconData terconData);

public slots:
    void showThermostatRegulatorSettingsWidget();

private:
    HeaterSignalsView * thermostatHeater;
    QLabel * resistanceLabel;
    QLabel * resistanceValueLabel;
    QLabel * diffTemperatureLabel;
    QLabel * diffTemperatureValueLabel;
    WidgetRegulatorFurnace * widgetRegulatorThermostat;
};

class HeaterSignalsView : public QWidget
{
    Q_OBJECT
public:
    explicit HeaterSignalsView(const QString & heaterName, QWidget *parent = 0);

public slots:
    void updateState(const json & state);
    void setTemperature(double temperature);
    void setValuePresision(int valuePresision) {valuePresision_ = valuePresision;}

private slots:
    void progressBarClicked();

signals:
    void statusLedClicked();

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
    QLabel * pLabel;
    QLabel * dLabel;
    QLabel * iLabel;
    QProgressBar * progressBar;
    double durationTimeProgress;
    double elapsedTimeProgress;
    bool leftTimeProgressBarViewMode;

    int valuePresision_;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // SIGNALSVIEW_H
