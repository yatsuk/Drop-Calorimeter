#include "progPowerRegulatorWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

ProgPowerRegulatorWidget::ProgPowerRegulatorWidget(QWidget *parent) :
    QGroupBox(parent)
{
    regulatorOn = false;

    QFont captionFont;
    captionFont.setBold(true);

    setTitle(tr("Программирование мощности"));
    enableProgPowerRegulator = new QRadioButton(tr("Включить"));
    enableProgPowerRegulator->setFont(captionFont);

    applyButton = new QPushButton(tr("Применить"));

    temperatureLabel = new QLabel(tr("Температура (%1С)    ").arg(QChar(176)));
    durationLabel = new QLabel(tr("Длительность (мин)"));

    temperatureSpinBox = new QSpinBox();
    temperatureSpinBox->setMaximum(2000);

    durationSpinBox = new QSpinBox();
    durationSpinBox->setMaximum(1000);
    durationSpinBox->setMinimum(1);

    QHBoxLayout * temperatureHorLayout = new QHBoxLayout;
    temperatureHorLayout->addWidget(temperatureLabel);
    temperatureHorLayout->addWidget(temperatureSpinBox);
    temperatureHorLayout->addStretch();

    QHBoxLayout * durationHorLayout = new QHBoxLayout;
    durationHorLayout->addWidget(durationLabel);
    durationHorLayout->addWidget(durationSpinBox);
    durationHorLayout->addStretch();
    durationHorLayout->addWidget(applyButton);
    durationHorLayout->addStretch();

    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addWidget(enableProgPowerRegulator);
    mainLayout->addLayout(temperatureHorLayout);
    mainLayout->addLayout(durationHorLayout);

    setLayout(mainLayout);

    connect (enableProgPowerRegulator,SIGNAL(pressed()),this,SLOT(progPowerModeOn()));
    connect (enableProgPowerRegulator,SIGNAL(clicked()),this,SLOT(checkBoxReleased()));
    connect (temperatureSpinBox,SIGNAL(valueChanged(int)),this,SLOT(temperatureOrDurationChanged()));
    connect (durationSpinBox,SIGNAL(valueChanged(int)),this,SLOT(temperatureOrDurationChanged()));
    connect (applyButton,SIGNAL(clicked()),this,SLOT(applyButtonClicked()));
}

void ProgPowerRegulatorWidget::checkBoxReleased(){
    enableProgPowerRegulator->setChecked(true);
}

void ProgPowerRegulatorWidget::setRegulatorOn(bool on){
    regulatorOn = on;
}

void ProgPowerRegulatorWidget::progPowerModeOn(){
    if(!enableProgPowerRegulator->isChecked()){
        int ret = QMessageBox::Yes;
        if(regulatorOn)
            ret = QMessageBox::warning(this,
                                       tr("Переключение регулятора в режим программируемой мощности."),
                                       tr("Подтвердите переключение регулятора в режим программируемой мощности."),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );
        if (ret==QMessageBox::Yes){
            setEnabledWidget(true);
            emit progPowerRegulatorEnabled();
        }
    }
}

void ProgPowerRegulatorWidget::setEnabledWidget(bool enabled){
    enableProgPowerRegulator->setChecked(enabled);
    temperatureSpinBox->setEnabled(enabled);
    durationSpinBox->setEnabled(enabled);
    temperatureLabel->setEnabled(enabled);
    durationLabel->setEnabled(enabled);
}

void ProgPowerRegulatorWidget::applyButtonClicked(){
    applyButton->setEnabled(false);
    emit temperature(temperatureSpinBox->value());
    emit duration(durationSpinBox->value());
}

void ProgPowerRegulatorWidget::temperatureOrDurationChanged(){
    applyButton->setEnabled(true);
}
