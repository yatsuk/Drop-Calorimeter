#include "addTempSegmentDialog.h"
#include <QMessageBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTime>
#include <QDebug>

AddTempSegmentDialog::AddTempSegmentDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Добавление сегмента"));

    /*Group Box Dynamic segment*/
    dynSegmentBox = new QGroupBox();

    maxTemperatureDynSegment= new QDoubleSpinBox();
    maxTemperatureDynSegment->setValue(20);
    maxTemperatureDynSegment->setMaximum(2500);
    maxTemperatureDynSegment->setDecimals(1);
    velocityDynSegment = new QDoubleSpinBox();
    velocityDynSegment->setMinimum(0.01);
    velocityDynSegment->setMaximum(50);
    velocityDynSegment->setValue(5);

    dynTemperLabel = new QLabel(tr("Конечная температура (%1С)").arg(QChar(176)));
    dynVelocityLabel= new QLabel(tr("Скорость изменения температуры (%1С/Мин)").arg(QChar(176)));
    dynVelocityLabel->setWordWrap(true);

    QGridLayout *dynSegmentLayout = new QGridLayout();
    dynSegmentLayout->addWidget(dynTemperLabel,0,0,1,3);
    dynSegmentLayout->addWidget(maxTemperatureDynSegment,0,3);
    dynSegmentLayout->addWidget(dynVelocityLabel,1,0,1,3);
    dynSegmentLayout->addWidget(velocityDynSegment,1,3);
    dynSegmentBox->setLayout(dynSegmentLayout);

    /*Group Box Isoterm segment*/
    isotermSegmentBox = new QGroupBox();

    timeIsotermSegment = new QSpinBox();
    timeIsotermSegment->setMinimum(1);
    timeIsotermSegment->setValue(600);
    timeIsotermSegment->setMaximum(600);

    isotermTimeLabel= new QLabel(tr("Время изотермического сегмента (Мин)"));
    isotermTimeLabel->setWordWrap(true);

    QGridLayout *isotermSegmentLayout = new QGridLayout();
    isotermSegmentLayout->addWidget(isotermTimeLabel,1,0,1,3);
    isotermSegmentLayout->addWidget(timeIsotermSegment,1,3);
    isotermSegmentBox->setLayout(isotermSegmentLayout);
    isotermSegmentBox->setHidden(true);
    /*Main Layout*/

    typeSegment = new QComboBox();
    typesSegment << tr("Нагрев")<< tr("Охлаждение")<< tr("Изотермический");
    typeSegment->addItems(typesSegment);

    typeLabel = new QLabel(tr("Тип температурного сегмента"));

    QHBoxLayout * typeLayout = new QHBoxLayout;
    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(typeSegment);

    acceptButton = new QPushButton(tr("Добавить"));
    acceptButton->setEnabled(false);
    cancelButton = new QPushButton(tr("Закрыть"));

    QHBoxLayout * buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addLayout(typeLayout);
    mainLayout->addWidget(dynSegmentBox);
    mainLayout->addWidget(isotermSegmentBox);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    connect(typeSegment,SIGNAL(currentIndexChanged(int)),this,SLOT(typeSegmentChanged(int)));
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(cancelButtonClicked()));
    connect(acceptButton,SIGNAL(clicked()),this,SLOT(acceptButtonClicked()));
    connect(maxTemperatureDynSegment,SIGNAL(valueChanged(double)),this,SLOT(isChanged()));
    connect(velocityDynSegment,SIGNAL(valueChanged(double)),this,SLOT(isChanged()));
    connect(timeIsotermSegment,SIGNAL(valueChanged(int)),this,SLOT(isChanged()));
}

void AddTempSegmentDialog::isChanged(){
    acceptButton->setEnabled(true);
}

void AddTempSegmentDialog::acceptButtonClicked(){
    switch (typeSegment->currentIndex()){
    case 0:{
        emit newSegment(new HeatingSegment(0,maxTemperatureDynSegment->value(),velocityDynSegment->value()));
        break;
    }
    case 1:{
        emit newSegment(new CoolingSegment(0,maxTemperatureDynSegment->value(),velocityDynSegment->value()));
        break;
    }

    case 2:{
        emit newSegment(new IsotermalSegment(0,timeIsotermSegment->value() * 60));
        break;
    }
    }
    acceptButton->setEnabled(false);

}

void AddTempSegmentDialog::typeSegmentChanged(int index){
    switch (index){
    case 2:{
        dynSegmentBox->setHidden(true);
        isotermSegmentBox->setHidden(false);
        break;
    }
    default:{
        dynSegmentBox->setHidden(false);
        isotermSegmentBox->setHidden(true);
        break;
    }
    }
    acceptButton->setEnabled(true);
}

void AddTempSegmentDialog::cancelButtonClicked(){
    close();
}
