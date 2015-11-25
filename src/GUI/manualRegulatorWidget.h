#ifndef MANUALREGULATORWIDGET_H
#define MANUALREGULATORWIDGET_H

#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include "regulator.h"

class ManualRegulatorWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit ManualRegulatorWidget(QWidget *parent = 0);
    bool isManualRegulatorEnabled();
    
signals:
    void manualRegulatorEnabled();
    void manualPower(double);

public slots:
    void setEnabledWidget(bool enabled);
    void setRegulatorOn(bool on);
    void setValue(double value);

private slots:
    void manualModeOn();
    void checkBoxReleased();
    void powerChange();
    void executionButtonClicked();

private:
    QRadioButton * enableManualRegulator;
    QLabel * manualRegulatorLabel;
    QDoubleSpinBox * setterOutPower;
    QPushButton * executionButton;

    bool regulatorOn;
};

#endif // MANUALREGULATORWIDGET_H
