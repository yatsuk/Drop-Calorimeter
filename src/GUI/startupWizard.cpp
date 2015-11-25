#include "startupWizard.h"
#include "ui_startupWizard.h"

StartupWizard::StartupWizard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartupWizard)
{
    ui->setupUi(this);
}

StartupWizard::~StartupWizard()
{
    delete ui;
}
