﻿#include "mainwindow.h"
#include <QLabel>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>

#include "terconData.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    isExperimentRecordStart(false)
{
setWindowState(Qt::WindowMaximized);

furnace = new Furnace(this);

logView = new LogView();
connect(logView,SIGNAL(sendMessageToFile(QString)),
        furnace,SLOT(writeLogMessageToFile(QString)));
logView->appendMessage(tr("Программа запущена."),Shared::warning);
logView->appendMessage(tr("Регулятор печи переведен в ручной режим управления."),Shared::information);

signalsView = new SignalsView;
coversAndCalHeaterWidget = new CoversAndCalHeaterWidget;


measurerTabs = new QTabWidget();
createPlots(furnace->getSettings());

tab = new QTabWidget();
tab->setTabPosition(QTabWidget::West);
tab->addTab(logView,tr("Сообщения"));
tab->addTab(coversAndCalHeaterWidget,tr("Доп. устройства"));
tab->addTab(signalsView,tr("Сигналы"));

beginDataRecordButton = new QPushButton(tr("Запись вкл"));
beginDataRecordButton->setCheckable(true);
endDataRecordButton = new QPushButton(tr("Запись выкл"));
endDataRecordButton->setCheckable(true);
endDataRecordButton->setChecked(true);

QHBoxLayout * recordButtonLayout = new QHBoxLayout();
recordButtonLayout->setMargin(5);
recordButtonLayout->addWidget(beginDataRecordButton);
recordButtonLayout->addWidget(endDataRecordButton);
recordButtonLayout->addStretch(1);
QGroupBox * recordButtonBox = new QGroupBox;
recordButtonBox->setLayout(recordButtonLayout);

QVBoxLayout * graphLayout = new QVBoxLayout();
graphLayout->addWidget(recordButtonBox);
graphLayout->addWidget(measurerTabs);

QHBoxLayout * hLayout = new QHBoxLayout();
hLayout->setSpacing(15);
hLayout->setMargin(5);
hLayout->addLayout(graphLayout,3);
hLayout->addWidget(tab,1);

setLayout(hLayout);

//Connection signals to slots


connect(beginDataRecordButton,SIGNAL(clicked()),this,SLOT(beginDataRecordClicked()));
connect(endDataRecordButton,SIGNAL(clicked()),this,SLOT(endDataRecordClicked()));

/*connect(widgetRegulatorFurnace,SIGNAL(message(QString,Shared::MessageLevel)),
            logView,SLOT(appendMessage(QString,Shared::MessageLevel)));
    connect(widgetRegulatorThermostat,SIGNAL(message(QString,Shared::MessageLevel)),
            logView,SLOT(appendMessage(QString,Shared::MessageLevel)));
    connect(additionalHeatersWidget,SIGNAL(message(QString,Shared::MessageLevel)),
            logView,SLOT(appendMessage(QString,Shared::MessageLevel)));*/

connect(furnace,SIGNAL(message(QString,Shared::MessageLevel)),
        logView,SLOT(appendMessage(QString,Shared::MessageLevel)));

beginNewExperiment();

}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(furnace->closeAppRequest()){
        event->accept();
    }else{
        QMessageBox msgBox;
        msgBox.setText(tr("Сейчас нельзя закрыть приложение."));
        msgBox.setInformativeText(tr("Попробуйте позже."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();

        event->ignore();
    }
}

void MainWindow::beginNewExperiment()
{
    furnace->run();

    connect(beginDataRecordButton,SIGNAL(clicked()),furnace,SLOT(beginDataRecord()));
    connect(endDataRecordButton,SIGNAL(clicked()),furnace,SLOT(endDataRecord()));

    connect(furnace,SIGNAL(AdcTerconDataSend(TerconData)),
            signalsView,SLOT(addValue(TerconData)));

    connect(furnace->regulatorFurnace(),SIGNAL(state(QJsonObject)),
            signalsView,SLOT(updateState(QJsonObject)));
    connect(furnace->regulatorUpHeater(),SIGNAL(state(QJsonObject)),
            signalsView,SLOT(updateState(QJsonObject)));
    connect(furnace->regulatorDownHeater(),SIGNAL(state(QJsonObject)),
            signalsView,SLOT(updateState(QJsonObject)));
    connect(furnace->regulatorThermostat(),SIGNAL(state(QJsonObject)),
            signalsView,SLOT(updateState(QJsonObject)));

}

void MainWindow::loadExperiment()
{

}

void MainWindow::beginDataRecordClicked()
{
    if (!isExperimentRecordStart){
        isExperimentRecordStart = true;
        beginDataRecordButton->setChecked(true);
        endDataRecordButton->setChecked(false);
        logView->appendMessage(tr("Запись в файл включена."),Shared::information);
    }
}

void MainWindow::endDataRecordClicked()
{
    if (isExperimentRecordStart){
        isExperimentRecordStart = false;
        beginDataRecordButton->setChecked(false);
        endDataRecordButton->setChecked(true);
        logView->appendMessage(tr("Запись в файл выключена."),Shared::information);
    }

}

void MainWindow::createPlots(const QJsonObject & settings)
{
    if(settings.isEmpty()) return;
    QJsonObject guiSettings = settings["GUI"].toObject();
    QJsonArray plotsArray = guiSettings["plots"].toArray();
    for (int indexPlots = 0; indexPlots < plotsArray.size(); ++indexPlots) {
        QJsonObject plotJsonObject = plotsArray[indexPlots].toObject();
        if(plotJsonObject.isEmpty()) continue;

        ChartWidget * plot = new ChartWidget();
        plot->setObjectName(plotJsonObject["id"].toString());
        plot->setPlotTitle(plotJsonObject["title"].toString());

        QJsonArray axesArray = plotJsonObject["axes"].toArray();
        for (int indexAxes = 0; indexAxes < axesArray.size(); ++indexAxes) {
            QJsonObject axesJsonObject = axesArray[indexAxes].toObject();
            if(axesJsonObject.isEmpty()) continue;
            plot->addAxesFromJSON(axesJsonObject);
        }

        QJsonArray graphArray = plotJsonObject["graphs"].toArray();
        for (int indexGraph = 0; indexGraph < graphArray.size(); ++indexGraph) {
            QJsonObject graphJsonObject = graphArray[indexGraph].toObject();
            if(graphJsonObject.isEmpty()) continue;
            plot->addSignalFromJSON(graphJsonObject);
        }


        measurerTabs->addTab(plot, plotJsonObject["shortTitle"].toString());
        connect(furnace,SIGNAL(AdcTerconDataSend(TerconData)),
                plot,SLOT(addDataTercon(TerconData)));

        plots.push_back(plot);
    }

}
