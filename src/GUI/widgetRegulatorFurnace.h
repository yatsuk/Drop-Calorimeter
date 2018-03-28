#ifndef WIDGETREGULATORFURNACE_H
#define WIDGETREGULATORFURNACE_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include "src/GUI/qledindicator.h"
#include "src/shared.h"
#include "automaticRegulatorWidget.h"
#include "manualRegulatorWidget.h"
#include "progPowerRegulatorWidget.h"
#include "diagnosticwidget.h"
#include "dialogParametersRegulator.h"
#include "src/regulator.h"

class WidgetRegulatorFurnace : public QDialog
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

    QColor defaultColorOn1LegIndicator;
    QColor defaultColorOn2LegIndicator;
    QColor defaultColorOff1LegIndicator;
    QColor defaultColorOff2LegIndicator;
    QLedIndicator * statusLed;
};

#endif // WIDGETREGULATORFURNACE_H
