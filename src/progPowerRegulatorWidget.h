#ifndef PROGPOWERREGULATORWIDGET_H
#define PROGPOWERREGULATORWIDGET_H

#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>

class ProgPowerRegulatorWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit ProgPowerRegulatorWidget(QWidget *parent = 0);
    
signals:
    void progPowerRegulatorEnabled();
    void temperature(double);
    void duration(double);

public slots:
    void setRegulatorOn(bool on);
    void setEnabledWidget(bool enabled);

private slots:
    void progPowerModeOn();
    void checkBoxReleased();
    void applyButtonClicked();
    void temperatureOrDurationChanged();

private:
    QRadioButton * enableProgPowerRegulator;
    QPushButton * applyButton;
    QSpinBox * temperatureSpinBox;
    QSpinBox * durationSpinBox;
    QLabel * temperatureLabel;
    QLabel * durationLabel;

    bool regulatorOn;
};

#endif // PROGPOWERREGULATORWIDGET_H
