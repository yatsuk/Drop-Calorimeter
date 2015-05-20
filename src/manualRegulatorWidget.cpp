#include "manualRegulatorWidget.h"
#include <QFont>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

ManualRegulatorWidget::ManualRegulatorWidget(QWidget *parent) :
    QGroupBox(parent)
{
    regulatorOn = false;

    QFont captionFont;
    captionFont.setBold(true);

    setTitle(tr("Ручное регулирование"));
    enableManualRegulator = new QRadioButton(tr("Включить"));
    enableManualRegulator->setFont(captionFont);
    enableManualRegulator->setChecked(true);

    manualRegulatorLabel = new QLabel(tr("Выходная мощность (%)"));
    setterOutPower = new QDoubleSpinBox();
    setterOutPower->setMaximum(100);

    executionButton = new QPushButton (tr("Применить"));
    executionButton->setEnabled(false);

    QHBoxLayout * ManualHorRegulatorLayout = new QHBoxLayout;
    ManualHorRegulatorLayout->addWidget(manualRegulatorLabel);
    ManualHorRegulatorLayout->addWidget(setterOutPower);
    ManualHorRegulatorLayout->addStretch();
    ManualHorRegulatorLayout->addWidget(executionButton);
    ManualHorRegulatorLayout->addStretch();

    QVBoxLayout * ManualRegulatorLayout = new QVBoxLayout;
    ManualRegulatorLayout->addWidget(enableManualRegulator);
    ManualRegulatorLayout->addLayout(ManualHorRegulatorLayout);

    setLayout(ManualRegulatorLayout);

    connect (enableManualRegulator,SIGNAL(pressed()),this,SLOT(manualModeOn()));
    connect (enableManualRegulator,SIGNAL(clicked()),this,SLOT(checkBoxReleased()));
    connect (setterOutPower,SIGNAL(valueChanged(double)),this,SLOT(powerChange()));
    connect (executionButton,SIGNAL(clicked()),this,SLOT(executionButtonClicked()));
}

void ManualRegulatorWidget::setValue(double value){
    setterOutPower->setValue(value);
}

void ManualRegulatorWidget::executionButtonClicked(){
    executionButton->setEnabled(false);
    emit manualPower(setterOutPower->value());
}

void ManualRegulatorWidget::powerChange(){
    executionButton->setEnabled(true);
}

void ManualRegulatorWidget::checkBoxReleased(){
    enableManualRegulator->setChecked(true);
}

void ManualRegulatorWidget::setRegulatorOn(bool on){
    regulatorOn = on;
}

void ManualRegulatorWidget::manualModeOn(){
    if(!enableManualRegulator->isChecked()){
        int ret = QMessageBox::Yes;
        if(regulatorOn)
            ret = QMessageBox::warning(this,
                                       tr("Переключение регулятора в ручной режим."),
                                       tr("Подтвердите переключение регулятора в ручной режим"),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );
        if (ret==QMessageBox::Yes){
            setEnabledWidget(true);
            emit manualRegulatorEnabled();
        }
    }
}

void ManualRegulatorWidget::setEnabledWidget(bool enabled){
    enableManualRegulator->setChecked(enabled);
    manualRegulatorLabel->setEnabled(enabled);
    setterOutPower->setEnabled(enabled);
}

bool ManualRegulatorWidget::isManualRegulatorEnabled(){
    return enableManualRegulator->isChecked();
}
