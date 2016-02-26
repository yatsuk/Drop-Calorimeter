#include "additionalHeatersWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFont>
#include <QDebug>
#include "furnace.h"

ConstValueRegulatorWidget::ConstValueRegulatorWidget(QWidget *parent) :
    QGroupBox(parent)
{
    setTitle(tr("Постоянное значение"));

    QFont captionFont;
    captionFont.setBold(true);

    enableRegulator = new QRadioButton(tr("Включить"));
    enableRegulator->setFont(captionFont);

    valueLabel = new QLabel(tr("Диф. температура (%1С)").arg(QChar(176)));
    valueSpinBox = new QDoubleSpinBox();
    valueSpinBox->setMaximum(3000);

    executionButton = new QPushButton (tr("Применить"));
    executionButton->setEnabled(false);

    QHBoxLayout * horLayout = new QHBoxLayout;
    horLayout->addWidget(valueLabel);
    horLayout->addWidget(valueSpinBox);
    horLayout->addStretch();
    horLayout->addWidget(executionButton);
    horLayout->addStretch();

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(enableRegulator);
    layout->addLayout(horLayout);

    setLayout(layout);

    connect (enableRegulator,SIGNAL(pressed()),this,SLOT(constValueModeOn()));
    connect (enableRegulator,SIGNAL(clicked()),this,SLOT(checkBoxReleased()));
    connect (valueSpinBox,SIGNAL(valueChanged(double)),this,SLOT(valueChange()));
    connect (executionButton,SIGNAL(clicked()),this,SLOT(executionButtonClicked()));
}

void ConstValueRegulatorWidget::setRegulatorOn(bool on){
    regulatorOn = on;
}

void ConstValueRegulatorWidget::constValueModeOn()
{
    if(!enableRegulator->isChecked()){
        int ret = QMessageBox::Yes;
        if(regulatorOn)
            ret = QMessageBox::warning(this,
                                       tr("Переключение регулятора в режим постоянной температуры."),
                                       tr("Подтвердите переключение регулятора в режим постоянной температуры"),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );
        if (ret == QMessageBox::Yes){
            setEnabledWidget(true);
            emit constValueRegulatorEnabled();
        }
    }
}

void ConstValueRegulatorWidget::checkBoxReleased()
{
    enableRegulator->setChecked(true);
}

void ConstValueRegulatorWidget::valueChange()
{
    executionButton->setEnabled(true);
}

void ConstValueRegulatorWidget::executionButtonClicked()
{
    executionButton->setEnabled(false);
    emit value(valueSpinBox->value());
}

void ConstValueRegulatorWidget::setEnabledWidget(bool enabled){
    enableRegulator->setChecked(enabled);
    valueLabel->setEnabled(enabled);
    valueSpinBox->setEnabled(enabled);
}

bool ConstValueRegulatorWidget::isRegulatorEnabled(){
    return enableRegulator->isChecked();
}





AdditionalHeaterGroupBox::AdditionalHeaterGroupBox(QWidget *parent) :
    QGroupBox(parent)
{
    dialogParametersRegulator = new DialogParametersRegulator(this);

    QFont captionFont;
    captionFont.setBold(true);
    defaultColorOn1LegIndicator = QColor(0,255,0);
    defaultColorOn2LegIndicator = QColor(0,192,0);
    defaultColorOff1LegIndicator = QColor(0,28,0);
    defaultColorOff2LegIndicator = QColor(0,128,0);
    statusLed = new QLedIndicator();
    statusLed->setCheckable(false);
    outPowerLabel = new QLabel (tr("Выходная мощность (%) = 0.00"));
    outPowerLabel->setFont(captionFont);
    settingsButton = new QPushButton (QIcon(":/images/Settings.ico"),"");
    QHBoxLayout * hLayout = new QHBoxLayout;
    hLayout->addWidget(statusLed);
    hLayout->addWidget(outPowerLabel);
    hLayout->addStretch(1);
    hLayout->addWidget(settingsButton);


    manualRegulatorWidget = new ManualRegulatorWidget;
    constValueRegulatorWidget = new ConstValueRegulatorWidget;

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addLayout(hLayout);
    layout->addWidget(manualRegulatorWidget);
    layout->addWidget(constValueRegulatorWidget);

    setLayout(layout);

    connect (manualRegulatorWidget, SIGNAL(manualRegulatorEnabled()), this, SLOT(setRegulatorModeManual()));
    connect (constValueRegulatorWidget, SIGNAL(constValueRegulatorEnabled()), this, SLOT(setRegulatorModeConstValue()));
    connect (settingsButton, SIGNAL(clicked()),this,SLOT(settingsRegulatorButtonClicked()));

    setRegulatorModeManual();
}

void AdditionalHeaterGroupBox::setPowerLabel(double value){
    outPowerLabel->setText(tr("Выходная мощность (%) = %1").arg(QString::number(value,'f',2)));
}


void AdditionalHeaterGroupBox::turnOnRegulator()
{
    constValueRegulatorWidget->setRegulatorOn(true);
    manualRegulatorWidget->setRegulatorOn(true);
    statusLed->setOffColor1(defaultColorOn1LegIndicator);
    statusLed->setOffColor2(defaultColorOn2LegIndicator);
}

void AdditionalHeaterGroupBox::turnOffRegulator()
{
    constValueRegulatorWidget->setRegulatorOn(false);
    manualRegulatorWidget->setRegulatorOn(false);
    statusLed->setOffColor1(defaultColorOff1LegIndicator);
    statusLed->setOffColor2(defaultColorOff2LegIndicator);
}

void AdditionalHeaterGroupBox::setRegulatorModeManual()
{
    constValueRegulatorWidget->setEnabledWidget(false);
    emit message(tr("Регулятор \"%1\" переведен в ручной режим управления.").arg(name_),Shared::information);
    emit regulatorModeChange(Shared::manual);
}

void AdditionalHeaterGroupBox::setRegulatorModeConstValue()
{
    manualRegulatorWidget->setEnabledWidget(false);
    emit message(tr("Регулятор \"%1\" переведен в режим постоянной температуры.").arg(name_),Shared::information);
    emit regulatorModeChange(Shared::constValue);
}

void AdditionalHeaterGroupBox::setNameRegulator(const QString &name)
{
    name_ = name;
}

void AdditionalHeaterGroupBox::setRegulator(Regulator *regulator)
{
    if (regulator == 0)
        return;

    m_regulator = regulator;

    connect(dialogParametersRegulator,SIGNAL(parameters(RegulatorParameters)),
            m_regulator,SLOT(setParameters(RegulatorParameters)));

    connect (constValueRegulatorWidget, SIGNAL(value(double)),
             m_regulator, SLOT(setTargetValue(double)));
}

void AdditionalHeaterGroupBox::settingsRegulatorButtonClicked()
{
    if (m_regulator)
        dialogParametersRegulator->setParameters(m_regulator->parameters());

    dialogParametersRegulator->exec();
}




AdditionalHeatersWidget::AdditionalHeatersWidget(QWidget *parent) :
    QWidget(parent)
{   
    QFont bigBoldFont;
    bigBoldFont.setPointSize(16);
    bigBoldFont.setBold(true);

    upHeater = new AdditionalHeaterGroupBox;
    upHeater->setTitle(tr("Верхний нагреватель"));
    upHeater->setNameRegulator(tr("Верхний охранный нагреватель"));
    upHeater->setRegulator(Furnace::instance()->regulatorUpHeater());

    downHeater = new AdditionalHeaterGroupBox;
    downHeater->setTitle(tr("Нижний нагреватель"));
    downHeater->setNameRegulator(tr("Нижний охранный нагреватель"));
    downHeater->setRegulator(Furnace::instance()->regulatorDownHeater());

    onHeaterButton = new QPushButton (tr("Включение нагревателей"));
    onHeaterButton->setFont(bigBoldFont);
    onHeaterButton->setCheckable(true);

    QVBoxLayout * layout =  new QVBoxLayout;
    layout->addWidget(onHeaterButton);
    layout->addWidget(upHeater);
    layout->addWidget(downHeater);
    layout->addStretch();

    setLayout(layout);

    connect (onHeaterButton,SIGNAL(pressed()),this,SLOT(startRegulatorClicked()));
    connect (upHeater,SIGNAL(message(QString,Shared::MessageLevel)),this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect (downHeater,SIGNAL(message(QString,Shared::MessageLevel)),this,SIGNAL(message(QString,Shared::MessageLevel)));

    connectRegulator();
}

void AdditionalHeatersWidget::startRegulatorClicked()
{
    if (!onHeaterButton->isChecked()){
        int ret = QMessageBox::warning(this,
                                       tr("Включение охранных нагревателей."),
                                       tr("Включите ЭМ пускатель, затем нажмите на кнопку \"Да\"."),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );

        if (ret==QMessageBox::Yes){
            onHeaterButton->setChecked(true);
            onHeaterButton->setText(tr("Выключение нагревателей"));
            upHeater->turnOnRegulator();
            downHeater->turnOnRegulator();
            emit regulatorStart();
            emit message(tr("Включение охранных нагревателей."),Shared::warning);
        }
    }
    else{
        int ret = QMessageBox::warning(this,
                                       tr("Выключение нагревателей."),
                                       tr("Подтвердите выключение нагревателей."),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );

        if (ret==QMessageBox::Yes){
            onHeaterButton->setText(tr("Включение нагревателей"));
            emit message(tr("Выключение охранных нагревателей."),Shared::warning);
            emit regulatorStop();
            upHeater->turnOffRegulator();
            downHeater->turnOffRegulator();
            onHeaterButton->setChecked(false);
        }
    }
}


void AdditionalHeatersWidget::connectRegulator()
{
    connect (upHeater,SIGNAL(regulatorModeChange(Shared::RegulatorMode)),
             Furnace::instance()->regulatorUpHeater(),SLOT(setMode(Shared::RegulatorMode)));
    connect (downHeater,SIGNAL(regulatorModeChange(Shared::RegulatorMode)),
             Furnace::instance()->regulatorDownHeater(),SLOT(setMode(Shared::RegulatorMode)));

    connect(this,SIGNAL(regulatorStart()),
            Furnace::instance()->regulatorUpHeater(),SLOT(regulatorStart()));
    connect(this,SIGNAL(regulatorStart()),
            Furnace::instance()->regulatorDownHeater(),SLOT(regulatorStart()));
    connect(this,SIGNAL(regulatorStop()),
            Furnace::instance()->regulatorUpHeater(),SLOT(regulatorStop()));
    connect(this,SIGNAL(regulatorStop()),
            Furnace::instance()->regulatorDownHeater(),SLOT(regulatorStop()));

    connect(upHeater->manualRegulatorWidget,SIGNAL(manualPower(double)),
            Furnace::instance()->regulatorUpHeater(),SLOT(setValueManual(double)));
    connect(downHeater->manualRegulatorWidget,SIGNAL(manualPower(double)),
            Furnace::instance()->regulatorDownHeater(),SLOT(setValueManual(double)));

    connect(Furnace::instance()->regulatorUpHeater(),SIGNAL(outPower(double)),
            upHeater,SLOT(setPowerLabel(double)));
    connect(Furnace::instance()->regulatorDownHeater(),SIGNAL(outPower(double)),
            downHeater,SLOT(setPowerLabel(double)));

    connect(Furnace::instance()->regulatorUpHeater(),SIGNAL(stopRegulator()),
            this,SLOT(turnOffRegulators()));

    connect(Furnace::instance()->regulatorUpHeater(),SIGNAL(outPower(double)),
            upHeater->manualRegulatorWidget,SLOT(setValue(double)));
    connect(Furnace::instance()->regulatorDownHeater(),SIGNAL(outPower(double)),
            downHeater->manualRegulatorWidget,SLOT(setValue(double)));
}

void AdditionalHeatersWidget::turnOffRegulators()
{
    onHeaterButton->setText(tr("Включение нагревателей"));
    emit message(tr("Выключение охранных нагревателей."),Shared::warning);
    upHeater->turnOffRegulator();
    downHeater->turnOffRegulator();
    onHeaterButton->setChecked(false);
}
