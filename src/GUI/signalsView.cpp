#include "signalsView.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFont>

SignalsView::SignalsView(QWidget *parent) :
    QWidget(parent)
{
    layout = new QVBoxLayout;
    layout->addStretch();

    setLayout(layout);

    addSignal("ThermostatDiffTemperature");
    addSignal("ThermostatTemperature");
    addSignal("Resistance");
    addSignal("SampleTemperature");
    addSignal("FurnaceTemperatureInert");
    addSignal("FurnaceUpAddHeater");
    addSignal("FurnaceDownAddHeater");
    addSignal("FurnaceSetTemperature");
    addSignal("FurnaceTemperature");
}

void SignalsView::addSignal(const QString &signalName){

    QGroupBox * box = new QGroupBox;

    box->setFont(QFont("Times", 16, QFont::Bold,true));

    if (signalName=="FurnaceTemperature"){
        box->setTitle(tr("Температура в печи"));
    }
    else if (signalName=="FurnaceSetTemperature"){
        box->setTitle(tr("Уставка печи"));
    }
    else if (signalName=="SampleTemperature"){
        box->setTitle(tr("Температура образца"));
    }
    else if (signalName=="Resistance"){
        box->setTitle(tr("Сопротивление МТС блока"));
    }
    else if (signalName=="ThermostatTemperature"){
        box->setTitle(tr("Температура термостата"));
    }
    else if (signalName=="ThermostatDiffTemperature"){
        box->setTitle(tr("Дифф. тем-ра термостата"));
    }
    else if (signalName=="FurnaceTemperatureInert"){
        box->setTitle(tr("Темперетура вырав. блока"));
    }
    else if (signalName=="FurnaceUpAddHeater"){
        box->setTitle(tr("Темперетура верх. охр."));
    }
    else if (signalName=="FurnaceDownAddHeater"){
        box->setTitle(tr("Темперетура нижн. охр."));
    }

    QLabel * label = new QLabel;
    label->setObjectName(signalName);
    labels.append(label);

    QVBoxLayout * boxLayout = new QVBoxLayout;
    boxLayout->addWidget(label,0,Qt::AlignRight);

    box->setLayout(boxLayout);

    layout->insertWidget(0,box);
}

void SignalsView::addValue(TerconData terconData){
    if (terconData.deviceNumber==5&&terconData.channel==2){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="FurnaceTemperature")
                label->setText(tr("<H1>%1 %2C</H1>").arg(QString::number(terconData.value,'f',1)).arg(QChar(176)));
        }

    }
    else if (terconData.deviceNumber==5&&terconData.channel==5){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="SampleTemperature")
                label->setText(tr("<H1> %1 %2C</H1>").arg(QString::number(terconData.value,'f',1)).arg(QChar(176)));
        }
    }
    else if (terconData.deviceNumber==1&&terconData.channel==1){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="Resistance")
                label->setText(tr("<H1> %1 Ом</H1>").arg(QString::number(terconData.value,'f',3)));
        }
    }
    else if (terconData.deviceNumber==2&&terconData.channel==2){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="ThermostatTemperature")
                label->setText(tr("<H1> %1 %2C</H1>").arg(QString::number(terconData.value,'f',3)).arg(QChar(176)));
        }
    }
    else if (terconData.deviceNumber==2&&terconData.channel==1){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="ThermostatDiffTemperature")
                label->setText(tr("<H1> %1 мВ</H1>").arg(QString::number(terconData.value,'f',3)));
        }
    }
    else if (terconData.deviceNumber==terconData.virtualDeviceNumber&&terconData.channel==0){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="FurnaceSetTemperature")
                label->setText(tr("<H1> %1 %2C</H1>").arg(QString::number(terconData.value,'f',1)).arg(QChar(176)));
        }
    }
    else if (terconData.deviceNumber==5&&terconData.channel==4){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="FurnaceTemperatureInert")
                label->setText(tr("<H1>%1 %2C</H1>").arg(QString::number(terconData.value,'f',1)).arg(QChar(176)));
        }

    }
    else if (terconData.deviceNumber==5&&terconData.channel==1){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="FurnaceUpAddHeater")
                label->setText(tr("<H1>%1 %2C</H1>").arg(QString::number(terconData.value,'f',1)).arg(QChar(176)));
        }

    }
    else if (terconData.deviceNumber==5&&terconData.channel==3){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="FurnaceDownAddHeater")
                label->setText(tr("<H1>%1 %2C</H1>").arg(QString::number(terconData.value,'f',1)).arg(QChar(176)));
        }

    }
}
