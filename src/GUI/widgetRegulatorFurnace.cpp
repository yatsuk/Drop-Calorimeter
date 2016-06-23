#include "widgetRegulatorFurnace.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFont>
#include <QDebug>

WidgetRegulatorFurnace::WidgetRegulatorFurnace(QWidget *parent) :
    QDialog(parent)
{
    QFont captionFont;
    captionFont.setBold(true);

    dialogParametersRegulator = new DialogParametersRegulator(this);
    autoRegulatorWidget = new AutomaticRegulatorWidget();
    autoRegulatorWidget->setEnabledWidget(false);

    manualRegulatorWidget = new ManualRegulatorWidget();
    manualRegulatorWidget->setEnabledWidget(true);

    progPowerRegulatorWidget = new ProgPowerRegulatorWidget();
    progPowerRegulatorWidget->setEnabledWidget(false);

    diagnosticWidget = new DiagnosticWidget();


    /* Main Layout*/
    startStopButton =  new QPushButton(tr("Включение регулятора"));
    startStopButton->setCheckable(true);
    emergencyStopButton =  new QPushButton(tr("Аварийное выключение"));
    emergencyStopButton->setToolTip(tr("Выключение ЭМ пускателя"));
    emergencyStopButton->setFont(captionFont);
    emergencyStopButton->setEnabled(false);
    settingRegulatorButton = new QPushButton(QIcon(":/images/Settings.ico"),"");

    QHBoxLayout * hButtonLayout = new QHBoxLayout;
    hButtonLayout->addWidget(startStopButton);
    hButtonLayout->addWidget(emergencyStopButton);

    defaultColorOn1LegIndicator = QColor(0,255,0);
    defaultColorOn2LegIndicator = QColor(0,192,0);
    defaultColorOff1LegIndicator = QColor(0,28,0);
    defaultColorOff2LegIndicator = QColor(0,128,0);
    statusLed = new QLedIndicator();
    statusLed->setCheckable(false);
    outPowerLabel= new QLabel(tr("Выходная мощность (%) = 0.00"));
    outPowerLabel->setFont(captionFont);
    QHBoxLayout * outPowerAndSettingsLayout = new QHBoxLayout;
    outPowerAndSettingsLayout->addWidget(statusLed);
    outPowerAndSettingsLayout->addWidget(outPowerLabel);
    outPowerAndSettingsLayout->addStretch(1);
    outPowerAndSettingsLayout->addWidget(settingRegulatorButton);

    QVBoxLayout * vLayout = new QVBoxLayout;
    vLayout->addLayout(hButtonLayout);
    vLayout->addLayout(outPowerAndSettingsLayout);
    vLayout->addWidget(autoRegulatorWidget);
    vLayout->addWidget(manualRegulatorWidget);
    vLayout->addWidget(progPowerRegulatorWidget);
    vLayout->addWidget(diagnosticWidget);
    vLayout->addStretch(1);

    setLayout(vLayout);

    /*Connection signals to slots*/
    connect (startStopButton,SIGNAL(pressed()),this,SLOT(startRegulatorClicked()));
    connect (emergencyStopButton,SIGNAL(clicked()),this,SLOT(emergencyStopClicked()));

    connect (autoRegulatorWidget,SIGNAL(autoRegulatorEnabled()),this,SLOT(setRegulatorModeAutomatic()));
    connect (autoRegulatorWidget,SIGNAL(regCurrentTemperature(bool)),this,SLOT(regCurrentTemperature(bool)));

    connect (manualRegulatorWidget,SIGNAL(manualRegulatorEnabled()),this,SLOT(setRegulatorModeManual()));
    connect (progPowerRegulatorWidget,SIGNAL(progPowerRegulatorEnabled()),this,SLOT(setRegulatorModeProgPower()));

    connect (startStopButton,SIGNAL(toggled(bool)),autoRegulatorWidget,SLOT(setRegulatorOn(bool)));
    connect (startStopButton,SIGNAL(toggled(bool)),manualRegulatorWidget,SLOT(setRegulatorOn(bool)));
    connect (startStopButton,SIGNAL(toggled(bool)),progPowerRegulatorWidget,SLOT(setRegulatorOn(bool)));
    connect(settingRegulatorButton,SIGNAL(clicked()),this,SLOT(settingsRegulatorButtonClicked()));

    diagnosticWidget->hide();

    emit regulatorModeChange(Shared::manual);
}

void WidgetRegulatorFurnace::setRegulator(Regulator *regulator){
    regulatorOfFurnace = regulator;
    autoRegulatorWidget->setTemperatureProgramm(regulatorOfFurnace->getTemperatureProgramm());
    connect (autoRegulatorWidget,SIGNAL(goToSegment(int)),
             regulatorOfFurnace,SLOT(goToSegment(int)));
    connect (this,SIGNAL(regulatorModeChange(Shared::RegulatorMode)),
             regulatorOfFurnace,SLOT(setMode(Shared::RegulatorMode)));
    connect(this,SIGNAL(regulatorStart()),regulatorOfFurnace,SLOT(regulatorStart()));
    connect(this,SIGNAL(regulatorStop()),regulatorOfFurnace,SLOT(regulatorStop()));
    connect(this,SIGNAL(regulatorEmergencyStop()),regulatorOfFurnace,SLOT(regulatorEmergencyStop()));
    connect(manualRegulatorWidget,SIGNAL(manualPower(double)),
            regulatorOfFurnace,SLOT(setValueManual(double)));
    connect(regulatorOfFurnace,SIGNAL(outPower(double)),this,SLOT(setPowerLabel(double)));
    connect(regulatorOfFurnace,SIGNAL(outPower(double)),
            manualRegulatorWidget,SLOT(setValue(double)));
    connect(regulatorOfFurnace,SIGNAL(currentTemperatureSegment(int)),
            autoRegulatorWidget,SLOT(setCurrentTemperatureSegment(int)));
    connect(regulatorOfFurnace,SIGNAL(manualMode()),this,SLOT(setRegulatorModeManual()));
    connect(regulatorOfFurnace,SIGNAL(emergencyStopRegulator()),this,SLOT(emergencyStop()));
    connect(regulatorOfFurnace,SIGNAL(stopRegulator()),this,SLOT(offRegulator()));

    connect(progPowerRegulatorWidget,SIGNAL(temperature(double)),regulatorOfFurnace,SLOT(setTemperatureProgMode(double)));
    connect(progPowerRegulatorWidget,SIGNAL(duration(double)),regulatorOfFurnace,SLOT(setDurationProgMode(double)));

    connect(dialogParametersRegulator,SIGNAL(parameters(RegulatorParameters)),
            regulatorOfFurnace,SLOT(setParameters(RegulatorParameters)));
}

void WidgetRegulatorFurnace::setPowerLabel(double value){
    outPowerLabel->setText(tr("Выходная мощность (%) = %1").arg(QString::number(value,'f',2)));
}

void WidgetRegulatorFurnace::regCurrentTemperature(bool enable){
    if (enable)
        emit regulatorModeChange(Shared::stopCurrentTemperature);
    else
        emit regulatorModeChange(Shared::automatic);
}

void WidgetRegulatorFurnace::setRegulatorModeAutomatic(){ 
    manualRegulatorWidget->setEnabledWidget(false);
    progPowerRegulatorWidget->setEnabledWidget(false);
    emit message(tr("Регулятор печи переведен в автоматический режим управления."),Shared::information);
    emit regulatorModeChange(Shared::automatic);
}

void WidgetRegulatorFurnace::setRegulatorModeManual(){
    autoRegulatorWidget->setEnabledWidget(false);
    //manualRegulatorWidget->setEnabledWidget(true);
    progPowerRegulatorWidget->setEnabledWidget(false);
    emit message(tr("Регулятор печи переведен в ручной режим управления."),Shared::information);
    emit regulatorModeChange(Shared::manual);
}

void WidgetRegulatorFurnace::setRegulatorModeProgPower(){
    autoRegulatorWidget->setEnabledWidget(false);
    manualRegulatorWidget->setEnabledWidget(false);
    emit message(tr("Регулятор печи переведен в режим программируемой мощности."),Shared::information);
    emit regulatorModeChange(Shared::programPower);
}

void WidgetRegulatorFurnace::startRegulatorClicked(){
    if (!startStopButton->isChecked()){
        int ret = QMessageBox::warning(this,
                                       tr("Включение регулятора."),
                                       tr("Включите ЭМ пускатель, затем нажмите на кнопку \"Да\"."),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );

        if (ret==QMessageBox::Yes){
            startStopButton->setChecked(true);
            emergencyStopButton->setEnabled(true);
            startStopButton->setText(tr("Выключение регулятора"));
            statusLed->setOffColor1(defaultColorOn1LegIndicator);
            statusLed->setOffColor2(defaultColorOn2LegIndicator);
            emit regulatorStart();
            emit message(tr("Включение регулятора."),Shared::warning);
        }
    }
    else{
        int ret = QMessageBox::warning(this,
                                       tr("Выключение регулятора."),
                                       tr("Подтвердите выключение регулятора."),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );

        if (ret==QMessageBox::Yes){
            startStopButton->setText(tr("Включение регулятора"));
            statusLed->setOffColor1(defaultColorOff1LegIndicator);
            statusLed->setOffColor2(defaultColorOff2LegIndicator);
            emit message(tr("Выключение регулятора."),Shared::warning);
            emit regulatorStop();
            offRegulator();
        }
    }
}

void WidgetRegulatorFurnace::emergencyStopClicked(){
    if(startStopButton->isChecked()){
        int ret = QMessageBox::critical(this,
                                        tr("Аварийное выключение регулятора."),
                                        tr("Подтвердите аварийное выключение регулятора."),
                                        QMessageBox::Yes|QMessageBox::No,
                                        QMessageBox::No
                                        );

        if (ret==QMessageBox::Yes){
            emergencyStop();
        }
    }
}

void WidgetRegulatorFurnace::emergencyStop(){
    emit message(tr("Аварийное выключение регулятора."),Shared::critical);
    emit regulatorEmergencyStop();
    emit regulatorStop();
    offRegulator();
}

void WidgetRegulatorFurnace::settingsRegulatorButtonClicked()
{
    dialogParametersRegulator->setParameters(regulatorOfFurnace->parameters());
    dialogParametersRegulator->exec();
}

void WidgetRegulatorFurnace::offRegulator(){
    if(!manualRegulatorWidget->isManualRegulatorEnabled()){
        manualRegulatorWidget->setEnabledWidget(true);
        emit message(tr("Регулятор печи переведен в ручной режим управления."),Shared::information);
        emit regulatorModeChange(Shared::manual);
    }
    startStopButton->setChecked(false);
    startStopButton->setText(tr("Включение регулятора"));
    statusLed->setOffColor1(defaultColorOff1LegIndicator);
    statusLed->setOffColor2(defaultColorOff2LegIndicator);
    emergencyStopButton->setEnabled(false);
    autoRegulatorWidget->setEnabledWidget(false);
    progPowerRegulatorWidget->setEnabledWidget(false);
}
