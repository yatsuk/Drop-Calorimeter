#ifndef WIDGETREGULATORFURNACE_H
#define WIDGETREGULATORFURNACE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "shared.h"
#include "automaticRegulatorWidget.h"
#include "manualRegulatorWidget.h"
#include "progPowerRegulatorWidget.h"
#include "diagnosticwidget.h"
#include "dialogParametersRegulator.h"
#include "regulator.h"

class WidgetRegulatorFurnace : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetRegulatorFurnace(QWidget *parent = 0);
    void setRegulator(Regulator * regulator);

    DiagnosticWidget * diagnosticWidget;
    
signals:
    void regulatorStop();
    void regulatorEmergencyStop();
    void regulatorStart();
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void regulatorModeChange(Shared::RegulatorMode mode);
    
public slots:


private slots:
    void setRegulatorModeAutomatic();
    void setRegulatorModeManual();
    void setRegulatorModeProgPower();
    void startRegulatorClicked();
    void emergencyStopClicked();
    void settingsRegulatorButtonClicked();
    void emergencyStop();
    void regCurrentTemperature(bool enable);
    void setPowerLabel(double value);
    void offRegulator();

private:
    Regulator * regulatorOfFurnace;

    QPushButton * startStopButton;
    QPushButton * emergencyStopButton;
    QPushButton * settingRegulatorButton;
    DialogParametersRegulator * dialogParametersRegulator;
    QLabel * outPowerLabel;

    AutomaticRegulatorWidget * autoRegulatorWidget;
    ManualRegulatorWidget * manualRegulatorWidget;
    ProgPowerRegulatorWidget * progPowerRegulatorWidget;


};

#endif // WIDGETREGULATORFURNACE_H
