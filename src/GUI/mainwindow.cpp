#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QMessageBox>

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

    signalsView = new SignalsView;
    coversAndCalHeaterWidget = new CoversAndCalHeaterWidget;

    widgetRegulatorFurnace = new WidgetRegulatorFurnace();
    widgetRegulatorFurnace->setRegulator(furnace->regulatorFurnace());
    widgetRegulatorFurnace->diagnosticWidget->setDiagnostic(furnace->diagnostic());

    widgetRegulatorThermostat = new WidgetRegulatorFurnace();
    widgetRegulatorThermostat->setRegulator(furnace->regulatorTermostat());

    additionalHeatersWidget = new AdditionalHeatersWidget;

    temperatureSample = new ChartWidget();
    temperatureSample->setPlotTitle(tr("Температура образца"));
    temperatureSample->addSignal(5, 5, true, Qt::black, tr("Температура образца"));
    temperatureSample->setYLeftAxisTitle(tr("Температура, %1C").arg(QChar(176)));

    resistance = new ChartWidget();
    resistance->setPlotTitle(tr("Сопротивление МТС калориметрического блока"));
    resistance->addSignal(1, 1, true, Qt::black, tr("Сопротивление МТС"));
    resistance->setYLeftAxisTitle(tr("Сопротивление, Ом"));

    calibrationHeater = new ChartWidget();
    calibrationHeater->setPlotTitle(tr("Калибровочный нагреватель"));
    calibrationHeater->addSignal(3, 1, true, Qt::black, tr("1 канал"));
    calibrationHeater->addSignal(3, 2, true, Qt::red, tr("2 канал"));
    calibrationHeater->setYLeftAxisTitle(tr("Напряжение, мВ"));

    temperatureThermostat = new ChartWidget();
    temperatureThermostat->setPlotTitle(tr("Температура термостата калориметра"));
    temperatureThermostat->addSignal(2, 2, true, Qt::black, tr("температура термостата"));
    temperatureThermostat->addSignal(2, 1, false, Qt::red, tr("диф. температура термостата"));
    temperatureThermostat->setYLeftAxisTitle(tr("Температура, %1C").arg(QChar(176)));
    temperatureThermostat->setYRigthAxisTitle(tr("Напряжение, мВ"));

    temperatureFurnace = new ChartWidget();
    temperatureFurnace->setPlotTitle(tr("Температура печи"));
    temperatureFurnace->addSignal(5, 1, true, Qt::black, tr("Верхний охр нагреватель"));
    temperatureFurnace->addSignal(5, 2, true, Qt::red, tr("Температура осн. нагревателя"));
    temperatureFurnace->addSignal(5, 3, true, Qt::green, tr("Нижний охр. нагреватель"));
    temperatureFurnace->addSignal(5, 4, true, Qt::blue, tr("Выравнивающий блок"));
    temperatureFurnace->addSignal(5, 5, true, Qt::magenta, tr("Температура образца"));
    temperatureFurnace->setYLeftAxisTitle(tr("Температура, %1C").arg(QChar(176)));

    tab = new QTabWidget();
    tab->setTabPosition(QTabWidget::West);
    tab->addTab(logView,tr("Сообщения"));
    tab->addTab(widgetRegulatorFurnace,tr("Регулятор печи"));
    tab->addTab(widgetRegulatorThermostat,tr("Регулятор термостата"));
    tab->addTab(additionalHeatersWidget,tr("Охранные нагреватели"));
    tab->addTab(coversAndCalHeaterWidget,tr("Доп. устройства"));
    tab->addTab(signalsView,tr("Сигналы"));

    measurerTabs = new QTabWidget();
    measurerTabs->addTab(temperatureSample,tr("Температура образца"));
    measurerTabs->addTab(temperatureFurnace,tr("Температура печи"));
    measurerTabs->addTab(resistance,tr("Сопротивление блока"));
    measurerTabs->addTab(temperatureThermostat,tr("Температура термостата"));
    measurerTabs->addTab(calibrationHeater,tr("Калибровочный нагреватель"));


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
    hLayout->addLayout(graphLayout,4);
    hLayout->addWidget(tab,1);

    setLayout(hLayout);

    //Connection signals to slots


    connect(beginDataRecordButton,SIGNAL(clicked()),this,SLOT(beginDataRecordClicked()));
    connect(endDataRecordButton,SIGNAL(clicked()),this,SLOT(endDataRecordClicked()));

    connect(widgetRegulatorFurnace,SIGNAL(message(QString,Shared::MessageLevel)),
            logView,SLOT(appendMessage(QString,Shared::MessageLevel)));
    connect(widgetRegulatorThermostat,SIGNAL(message(QString,Shared::MessageLevel)),
            logView,SLOT(appendMessage(QString,Shared::MessageLevel)));
    connect(additionalHeatersWidget,SIGNAL(message(QString,Shared::MessageLevel)),
            logView,SLOT(appendMessage(QString,Shared::MessageLevel)));

    connect(furnace,SIGNAL(message(QString,Shared::MessageLevel)),
            logView,SLOT(appendMessage(QString,Shared::MessageLevel)));

    connect(furnace,SIGNAL(AdcTerconDataSend(TerconData)),
            temperatureSample,SLOT(addDataTercon(TerconData)));
    connect(furnace,SIGNAL(AdcTerconDataSend(TerconData)),
            resistance,SLOT(addDataTercon(TerconData)));
    connect(furnace,SIGNAL(AdcTerconDataSend(TerconData)),
            temperatureThermostat,SLOT(addDataTercon(TerconData)));
    connect(furnace,SIGNAL(AdcTerconDataSend(TerconData)),
            calibrationHeater,SLOT(addDataTercon(TerconData)));
    connect(furnace,SIGNAL(AdcTerconDataSend(TerconData)),
            temperatureFurnace,SLOT(addDataTercon(TerconData)));

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

}

void MainWindow::loadExperiment()
{

}

void MainWindow::beginDataRecordClicked()
{
    beginDataRecordButton->setChecked(true);
    endDataRecordButton->setChecked(false);
    logView->appendMessage(tr("Запись в файл включена."),Shared::information);
}

void MainWindow::endDataRecordClicked()
{
    beginDataRecordButton->setChecked(false);
    endDataRecordButton->setChecked(true);
    logView->appendMessage(tr("Запись в файл выключена."),Shared::information);
}
