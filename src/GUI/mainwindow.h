#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <include/externals/nlohmann/json/json.hpp>
#include "logView.h"
#include "calibrationHeaterWidget.h"
#include "chartWidget.h"
#include "src/furnace.h"
#include "calorimetrBlockWidget.h"
#include "signalsView.h"

using json = nlohmann::json;

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
    void createPlots(const json & settings);

protected:
    void closeEvent(QCloseEvent *event);

private:
    QTabWidget * tab;
    QTabWidget * measurerTabs;
    FurnaceSignalsView * furnaceSignalsView;
    LogView * logView;
    CalorimetrBlockWidget * calorimetrBlockWidget;

    QVector <ChartWidget *> plots;

    Furnace * furnace;
};

#endif // MAINWINDOW_H
