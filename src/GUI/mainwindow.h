#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QJsonObject>
#include "logView.h"
#include "calibrationHeaterWidget.h"
#include "chartWidget.h"
#include "furnace.h"
#include "calorimetrBlockWidget.h"
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
    void createPlots(const QJsonObject & settings);

protected:
    void closeEvent(QCloseEvent *event);

private:
    StartupWizard * startupWizard;
    QTabWidget * tab;
    QTabWidget * measurerTabs;
    FurnaceSignalsView * furnaceSignalsView;
    LogView * logView;
    CalorimetrBlockWidget * calorimetrBlockWidget;

    QVector <ChartWidget *> plots;

    Furnace * furnace;
};

#endif // MAINWINDOW_H
