#include "dialogParametersRegulator.h"
#include "ui_dialogParametersRegulator.h"

DialogParametersRegulator::DialogParametersRegulator(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogParametersRegulator)
{
    ui->setupUi(this);
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(acceptButtonClicked()));
}

DialogParametersRegulator::~DialogParametersRegulator()
{
    delete ui;
}

void DialogParametersRegulator::acceptButtonClicked()
{
    RegulatorParameters newParameters;
    newParameters.gP = ui->propSpinBox->value();
    newParameters.gD = ui->deffSpinBox->value();
    newParameters.gI = ui->integrSpinBox->value();
    newParameters.maxIntegralValue = ui->maxIntegSpinBox->value();
    newParameters.offset = ui->offsetSpinBox->value();
    newParameters.minPower = ui->minPowerSpinBox->value();
    newParameters.maxPower = ui->maxPowerSpinBox->value();

    newParameters.averageCount = 1;
    newParameters.averagePowerCount = 1;
    newParameters.procentPerSec = 0.5;
    newParameters.maxProportionalValue = 20;

    emit parameters(newParameters);
}

void DialogParametersRegulator::setParameters(const RegulatorParameters & parameters)
{
    ui->propSpinBox->setValue(parameters.gP);
    ui->deffSpinBox->setValue(parameters.gD);
    ui->integrSpinBox->setValue(parameters.gI);
    ui->maxIntegSpinBox->setValue(parameters.maxIntegralValue);
    ui->offsetSpinBox->setValue(parameters.offset);
    ui->minPowerSpinBox->setValue(parameters.minPower);
    ui->maxPowerSpinBox->setValue(parameters.maxPower);
}
