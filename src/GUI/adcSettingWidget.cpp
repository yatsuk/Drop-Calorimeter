#include "adcSettingWidget.h"
#include "ui_adcSettingWidget.h"
#include "furnace.h"
#include <QDebug>

AdcSettingWidget::AdcSettingWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdcSettingWidget)
{
    ui->setupUi(this);

    //setParameters(Furnace::instance()->parametersAdc());
    connect(this,SIGNAL(parameters(AdcParameters)),
            Furnace::instance(),SLOT(setParametersAdc(AdcParameters)));
    connect(ui->acceptButton,SIGNAL(clicked()),this,SLOT(acceptButtonClicked()));
}

AdcSettingWidget::~AdcSettingWidget()
{
    delete ui;
}

void AdcSettingWidget::acceptButtonClicked()
{
    AdcParameters newParameters;
    newParameters.roomTemperature = ui->coldThermocoupleSpinBox->value();
    newParameters.filter = ui->filtrCheckBox->isChecked();
    newParameters.averageCount = ui->averageSpinBox->value();
    newParameters.thermocoupleType = ui->typeThermocoupleComboBox->currentText();
    newParameters.portName = ui->portComboBox->currentText();
    emit parameters(newParameters);
    close();
}

void AdcSettingWidget::setParameters(const AdcParameters & parameters)
{
    ui->portComboBox->clear();

    if (parameters.averageCount>0){
        ui->enableAverageCheckBox->setChecked(true);
        ui->averageSpinBox->setEnabled(true);
    }else{
        ui->enableAverageCheckBox->setChecked(false);
        ui->averageSpinBox->setEnabled(false);
    }
    ui->averageSpinBox->setValue(parameters.averageCount);
    ui->filtrCheckBox->setChecked(parameters.filter);
    ui->coldThermocoupleSpinBox->setValue(parameters.roomTemperature);

    for (int i=0;i<ui->typeThermocoupleComboBox->count();++i){
        if (ui->typeThermocoupleComboBox->itemText(i)==parameters.thermocoupleType)
            ui->typeThermocoupleComboBox->setCurrentIndex(i);
    }

    for (int i=0;i<ui->portComboBox->count();++i){
        if (ui->portComboBox->itemText(i)==parameters.portName)
            ui->portComboBox->setCurrentIndex(i);
    }
}
