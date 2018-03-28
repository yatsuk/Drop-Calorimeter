#ifndef DIAGNOSTICWIDGET_H
#define DIAGNOSTICWIDGET_H

#include <QGroupBox>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>
#include "src/diagnostic.h"
#include "diagnosticSettingWidget.h"

class DiagnosticWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit DiagnosticWidget(QWidget *parent = 0);
    void setDiagnostic(Diagnostic * diagnostic);

signals:
    
public slots:
    void setControlThermocouple(bool enable);
    void setControlWaterCooling(bool enable);
    void setControlTriac(bool enable);
    void setAlarmThermocouple(bool enable);

    void setAlarmWaterCoolingUpper();
    void setAlarmWaterCoolingLower();
    void setAlarmWaterCoolingNormal();

    void setAlarmTriac(bool enable);
    void offControl();

private slots:
    void alarmSlot();
    void openSettingDialog();
    
private:
    DiagnosticSettingWidget * settingWidget;
    QTableWidget * tableDiagnostic;
    QPushButton * settingDiagnosticButton;
    QTimer * alarmTimer;

    QTableWidgetItem * controlThermocoupleValue;
    QTableWidgetItem * controlWaterCoolingValue;
    QTableWidgetItem * controlTriacValue;

    bool alarmThermocouple;
    bool alarmWaterCooling;
    bool alarmTriac;

    Diagnostic * m_diagnostic;
    void initTable();

};

#endif // DIAGNOSTICWIDGET_H
