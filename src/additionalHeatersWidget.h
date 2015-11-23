#ifndef ADDITIONALHEATERSWIDGET_H
#define ADDITIONALHEATERSWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <qledindicator.h>

#include "dialogParametersRegulator.h"
#include "manualRegulatorWidget.h"
#include "regulator.h"

class ConstValueRegulatorWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit ConstValueRegulatorWidget(QWidget *parent = 0);
    bool isRegulatorEnabled();

signals:
    void constValueRegulatorEnabled();
    void value(double);

public slots:
    void setEnabledWidget(bool enabled);
    void setRegulatorOn(bool on);

private slots:
    void constValueModeOn();
    void checkBoxReleased();
    void valueChange();
    void executionButtonClicked();

private:
    QRadioButton * enableRegulator;
    QLabel * valueLabel;
    QDoubleSpinBox * valueSpinBox;
    QPushButton * executionButton;

    bool regulatorOn;
};

class AdditionalHeaterGroupBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit AdditionalHeaterGroupBox(QWidget *parent = 0);
    void setNameRegulator(const QString & name);
    void setRegulator(Regulator * regulator);

    ManualRegulatorWidget * manualRegulatorWidget;
    ConstValueRegulatorWidget * constValueRegulatorWidget;

signals:
    void regulatorStop();
    void regulatorStart();
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void regulatorModeChange(Shared::RegulatorMode mode);

public slots:
    void turnOnRegulator();
    void turnOffRegulator();

private slots:
    void setRegulatorModeManual();
    void setRegulatorModeConstValue();
    void setPowerLabel(double value);
    void settingsRegulatorButtonClicked();

private:
    QLabel * outPowerLabel;
    QPushButton * settingsButton;
    DialogParametersRegulator * dialogParametersRegulator;
    Regulator * m_regulator;
    QString name_;

    QColor defaultColorOn1LegIndicator;
    QColor defaultColorOn2LegIndicator;
    QColor defaultColorOff1LegIndicator;
    QColor defaultColorOff2LegIndicator;
    QLedIndicator * statusLed;

};

class AdditionalHeatersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AdditionalHeatersWidget(QWidget *parent = 0);

signals:
    void regulatorStart();
    void regulatorStop();
    void message(const QString & msg, Shared::MessageLevel msgLevel);

public slots:

private slots:
    void startRegulatorClicked();
    void connectRegulator();

private:
    AdditionalHeaterGroupBox * upHeater;
    AdditionalHeaterGroupBox * downHeater;

    QPushButton * onHeaterButton;

};

#endif // ADDITIONALHEATERSWIDGET_H
