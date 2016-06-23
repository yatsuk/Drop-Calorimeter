#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QJsonObject>
#include "logView.h"
#include "calibrationHeaterWidget.h"
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
    void createPlots(const QJsonObject & settings);

protected:
    void closeEvent(QCloseEvent *event);

private:
    StartupWizard * startupWizard;
    QTabWidget * tab;
    QTabWidget * measurerTabs;
    QPushButton * beginDataRecordButton;
    QPushButton * endDataRecordButton;
    SignalsView * signalsView;
    LogView * logView;
    CoversAndCalHeaterWidget * coversAndCalHeaterWidget;

    QVector <ChartWidget *> plots;

    Furnace * furnace;

    bool isExperimentRecordStart;
};

#endif // MAINWINDOW_H
