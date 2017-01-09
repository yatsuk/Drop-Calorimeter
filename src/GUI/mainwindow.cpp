#include "mainwindow.h"
#include <QLabel>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>

#include "terconData.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
setWindowState(Qt::WindowMaximized);

furnace = new Furnace(this);

logView = new LogView();
connect(logView,SIGNAL(sendMessageToFile(QString)),
        furnace,SLOT(writeLogMessageToFile(QString)));
logView->appendMessage(tr("Программа запущена."),Shared::warning);
logView->appendMessage(tr("Регулятор печи переведен в ручной режим управления."),Shared::information);

furnaceSignalsView = new FurnaceSignalsView;

calorimetrBlockWidget = new CalorimetrBlockWidget;


measurerTabs = new QTabWidget();
createPlots(furnace->getSettings());

tab = new QTabWidget();
tab->setTabPosition(QTabWidget::West);
tab->addTab(logView,tr("Сообщения"));
tab->addTab(calorimetrBlockWidget,tr("Калориметрический блок"));
tab->addTab(furnaceSignalsView,tr("Печь калориметра"));

QVBoxLayout * graphLayout = new QVBoxLayout();
graphLayout->addWidget(measurerTabs);

QHBoxLayout * hLayout = new QHBoxLayout();
hLayout->setSpacing(15);
hLayout->setMargin(5);
hLayout->addLayout(graphLayout,3);
hLayout->addWidget(tab,1);

setLayout(hLayout);

//Connection signals to slots

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

    connect(furnace,SIGNAL(AdcTerconDataSend(TerconData)),
            furnaceSignalsView,SLOT(setTemperature(TerconData)));
    connect(furnace,SIGNAL(AdcTerconDataSend(TerconData)),
            calorimetrBlockWidget,SLOT(setValue(TerconData)));

    connect(furnace->regulatorFurnace(),SIGNAL(state(QJsonObject)),
            furnaceSignalsView,SLOT(updateState(QJsonObject)));
    connect(furnace->regulatorUpHeater(),SIGNAL(state(QJsonObject)),
            furnaceSignalsView,SLOT(updateState(QJsonObject)));
    connect(furnace->regulatorDownHeater(),SIGNAL(state(QJsonObject)),
            furnaceSignalsView,SLOT(updateState(QJsonObject)));
    connect(furnace->regulatorThermostat(),SIGNAL(state(QJsonObject)),
            calorimetrBlockWidget,SLOT(updateState(QJsonObject)));

}

void MainWindow::loadExperiment()
{

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
