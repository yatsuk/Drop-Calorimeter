#ifndef CALORIMETRBLOCKWIDGET_H
#define CALORIMETRBLOCKWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QGroupBox>
#include "calibrationHeaterWidget.h"
#include "signalsView.h"
#include "coversWidget.h"
#include <GUI\qledindicator.h>

class ExperimentRecorderWidget;

class CalorimetrBlockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CalorimetrBlockWidget(QWidget *parent = 0);

signals:

public slots:
    void updateState(const QJsonObject & json);
    void setValue(TerconData terconData);

private:
    ExperimentRecorderWidget * experimentRecorderWidget;
    CalibrationHeaterWidget * calibrationHeaterWidget;
    CoversWidget * coversWidget;
    SafetyValveWidget * safetyValveWidget;
    SampleLockWidget * sampleLockWidget;
    CalorimeterSignalsView * calorimeterSignalsView;

};

class ExperimentRecorderWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit ExperimentRecorderWidget(QWidget *parent = 0);

private slots:
    void beginDataRecordClicked();
    void endDataRecordClicked();

private:
    QPushButton * beginDataRecordButton;
    QPushButton * endDataRecordButton;
    bool isExperimentRecordStart;

    QLedIndicator * statusLedIndicator;

    QColor defaultColorOn1LegIndicator;
    QColor defaultColorOn2LegIndicator;
    QColor defaultColorOff1LegIndicator;
    QColor defaultColorOff2LegIndicator;

};

#endif // CALORIMETRBLOCKWIDGET_H
