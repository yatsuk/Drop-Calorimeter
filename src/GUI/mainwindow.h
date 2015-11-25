#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QTextEdit>
#include "widgetRegulatorFurnace.h"
#include "logView.h"
#include "calibrationHeaterWidget.h"
#include "additionalHeatersWidget.h"
#include "chartWidget.h"
#include "furnace.h"
#include "coversAndCalHeaterWidget.h"
#include "signalsView.h"
#include "startupWizard.h"

class MainWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void beginNewExperiment();
    void loadExperiment();

private slots:
    void beginDataRecordClicked();
    void endDataRecordClicked();

protected:
    void closeEvent(QCloseEvent *event);

private:
    StartupWizard * startupWizard;
    QTabWidget * tab;
    QTabWidget * measurerTabs;
    QPushButton * beginDataRecordButton;
    QPushButton * endDataRecordButton;
    WidgetRegulatorFurnace * widgetRegulatorFurnace;
    WidgetRegulatorFurnace * widgetRegulatorThermostat;
    AdditionalHeatersWidget * additionalHeatersWidget;
    SignalsView * signalsView;
    LogView * logView;
    CoversAndCalHeaterWidget * coversAndCalHeaterWidget;
    ChartWidget * temperatureFurnace;
    ChartWidget * temperatureSample;
    ChartWidget * resistance;
    ChartWidget * calibrationHeater;
    ChartWidget * temperatureThermostat;
    ChartWidget * difTemperatureThermostat;

    Furnace * furnace;
};

#endif // MAINWINDOW_H
