#include "diagnosticwidget.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QFont>

DiagnosticWidget::DiagnosticWidget(QWidget *parent) :
    QGroupBox(parent)
{
    settingWidget = new DiagnosticSettingWidget(this);
    settingWidget->setModal(true);

    alarmThermocouple = false;
    alarmWaterCooling = false;

    alarmTimer = new QTimer(this);
    alarmTimer->start(500);

    setTitle(tr("Диагностика"));

    QStringList headerDiagnosticTable;
    headerDiagnosticTable << tr("Объект контроля") << tr("Состояние");

    tableDiagnostic = new QTableWidget(4,2);
    tableDiagnostic->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableDiagnostic->setColumnWidth(0,150);
    tableDiagnostic->setHorizontalHeaderLabels(headerDiagnosticTable);

    settingDiagnosticButton = new QPushButton(tr("Настройка диагностики"));
    settingDiagnosticButton->setEnabled(false);

    QVBoxLayout * diagnosticLayout = new QVBoxLayout();
    diagnosticLayout->addWidget(tableDiagnostic);
    diagnosticLayout->addWidget(settingDiagnosticButton);

    setLayout(diagnosticLayout);

    initTable();

    connect(alarmTimer,SIGNAL(timeout()),this,SLOT(alarmSlot()));
    connect(settingDiagnosticButton,SIGNAL(clicked()),
            this,SLOT(openSettingDialog()));

    /*Tests*/
    setControlThermocouple(true);
    setControlWaterCooling(true);

}

void DiagnosticWidget::openSettingDialog(){
    settingWidget->show();
}

void DiagnosticWidget::setDiagnostic(Diagnostic *diagnostic){
    m_diagnostic = diagnostic;
    connect(m_diagnostic,SIGNAL(controlThermocouple(bool)),
            this,SLOT(setAlarmThermocouple(bool)));
    connect(m_diagnostic,SIGNAL(alarmLowerPressure()),
            this,SLOT(setAlarmWaterCoolingLower()));
    connect(m_diagnostic,SIGNAL(alarmNormalPressure()),
            this,SLOT(setAlarmWaterCoolingNormal()));
    connect(m_diagnostic,SIGNAL(alarmUpperPressure()),
            this,SLOT(setAlarmWaterCoolingUpper()));
    connect(m_diagnostic,SIGNAL(offControl()),
            this,SLOT(offControl()));
}

void DiagnosticWidget::initTable(){
    QFont font;
    font.setPointSize(22);

    QTableWidgetItem * controlThermocouple = new QTableWidgetItem(tr("Термопара в печи"));
    controlThermocouple->setTextAlignment(Qt::AlignCenter);

    controlThermocoupleValue = new QTableWidgetItem(tr("?"));
    controlThermocoupleValue->setTextAlignment(Qt::AlignCenter);
    controlThermocoupleValue->setFont(font);
    controlThermocoupleValue->setBackgroundColor(Qt::gray);

    QTableWidgetItem * controlWaterCooling = new QTableWidgetItem(tr("Система охлаждения"));
    controlWaterCooling->setTextAlignment(Qt::AlignCenter);

    controlWaterCoolingValue = new QTableWidgetItem(tr("?"));
    controlWaterCoolingValue->setFont(font);
    controlWaterCoolingValue->setTextAlignment(Qt::AlignCenter);
    controlWaterCoolingValue->setBackgroundColor(Qt::gray);

    QTableWidgetItem * controlTriac = new QTableWidgetItem(tr("Симистор"));
    controlTriac->setTextAlignment(Qt::AlignCenter);

    controlTriacValue = new QTableWidgetItem(tr("?"));
    controlTriacValue->setFont(font);
    controlTriacValue->setTextAlignment(Qt::AlignCenter);
    controlTriacValue->setBackgroundColor(Qt::gray);

    tableDiagnostic->setItem(0,0,controlThermocouple);
    tableDiagnostic->setItem(0,1,controlThermocoupleValue);
    tableDiagnostic->setItem(1,0,controlWaterCooling);
    tableDiagnostic->setItem(1,1,controlWaterCoolingValue);
    tableDiagnostic->setItem(2,0,controlTriac);
    tableDiagnostic->setItem(2,1,controlTriacValue);
}

void DiagnosticWidget::offControl(){
    alarmThermocouple = false;
    alarmWaterCooling= false;

    controlThermocoupleValue->setText(tr("?"));
    controlThermocoupleValue->setBackgroundColor(Qt::gray);

    controlWaterCoolingValue->setText(tr("?"));
    controlWaterCoolingValue->setBackgroundColor(Qt::gray);

    controlTriacValue->setText(tr("?"));
    controlTriacValue->setBackgroundColor(Qt::gray);
}

void DiagnosticWidget::alarmSlot(){
    if(alarmThermocouple){
        if(controlThermocoupleValue->backgroundColor()==Qt::red)
            controlThermocoupleValue->setBackgroundColor(Qt::white);
        else
            controlThermocoupleValue->setBackgroundColor(Qt::red);
    }
    if(alarmWaterCooling){
        if(controlWaterCoolingValue->backgroundColor()==Qt::red)
            controlWaterCoolingValue->setBackgroundColor(Qt::white);
        else
            controlWaterCoolingValue->setBackgroundColor(Qt::red);
    }
    if (alarmThermocouple||alarmWaterCooling)
        QApplication::beep();
}

void DiagnosticWidget::setControlThermocouple(bool enable){
    if(!enable){
        controlThermocoupleValue->setBackgroundColor(Qt::gray);
        controlThermocoupleValue->setText(tr("?"));
    }
}

void DiagnosticWidget::setControlWaterCooling(bool enable){
    if(!enable){
        controlWaterCoolingValue->setBackgroundColor(Qt::gray);
        controlWaterCoolingValue->setText(tr("?"));
    }
}

void DiagnosticWidget::setControlTriac(bool enable){

}

void DiagnosticWidget::setAlarmThermocouple(bool enable){
    alarmThermocouple = enable;
    if(enable){
        controlThermocoupleValue->setText(tr("Обрыв"));
    }
    else{
        controlThermocoupleValue->setBackgroundColor(Qt::green);
        controlThermocoupleValue->setText(tr("OK"));
    }
}

void DiagnosticWidget::setAlarmWaterCoolingLower(){
    alarmWaterCooling = true;
    controlWaterCoolingValue->setBackgroundColor(Qt::red);
    controlWaterCoolingValue->setText(tr("%1").arg(QChar(8595)));
}

void DiagnosticWidget::setAlarmWaterCoolingUpper(){
    alarmWaterCooling = true;
    controlWaterCoolingValue->setBackgroundColor(Qt::red);
    controlWaterCoolingValue->setText(tr("%1").arg(QChar(8593)));
}

void DiagnosticWidget::setAlarmWaterCoolingNormal(){
    alarmWaterCooling = false;
    controlWaterCoolingValue->setBackgroundColor(Qt::green);
    controlWaterCoolingValue->setText(tr("ОК"));
}

void DiagnosticWidget::setAlarmTriac(bool enable){
    alarmTriac = enable;
    if(!alarmTimer->isActive()&&enable)
        alarmTimer->start();
}
