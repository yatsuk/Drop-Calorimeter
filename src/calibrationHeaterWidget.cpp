#include "calibrationHeaterWidget.h"
#include <QGridLayout>
#include <QSpacerItem>
#include <QVBoxLayout>
#include "furnace.h"

CalibrationHeaterWidget::CalibrationHeaterWidget(QWidget *parent) :
    QGroupBox(parent)
{
    heaterOn = false;

    defaultColorOn1LegIndicator = QColor(0,255,0);
    defaultColorOn2LegIndicator = QColor(0,192,0);
    defaultColorOff1LegIndicator = QColor(0,28,0);
    defaultColorOff2LegIndicator = QColor(0,128,0);

    setTitle(tr("Калибровочный нагреватель"));
    statusLedIndicator = new QLedIndicator;
    statusLabel = new QLabel;

    turnOnButton = new QPushButton(tr("Включить"));

    acceptButton = new QPushButton(tr("Применить"));

    durationSpinBox = new QSpinBox;
    durationSpinBox->setMaximum(10000);
    durationSpinBox->setSuffix(tr(" сек"));
    durationSpinBox->setAlignment(Qt::AlignRight);

    QGridLayout * layout = new QGridLayout;
    layout->addWidget(statusLedIndicator,0,0);
    layout->addWidget(statusLabel,0,1);
    layout->addWidget(turnOnButton,0,2);

    layout->addWidget(durationSpinBox,1,1);
    layout->addWidget(acceptButton,1,2);

    setLayout(layout);

    connect(turnOnButton,SIGNAL(clicked()),this,SLOT(turnOnButtonClicked()));
    connect(acceptButton,SIGNAL(clicked()),this,SLOT(acceptButtonClicked()));

    connect(this,SIGNAL(turnOn(int)),
            Furnace::instance(),SLOT(turnOnCalibrationHeater(int)));
    connect(this,SIGNAL(turnOff()),
            Furnace::instance(),SLOT(turnOffCalibrationHeater()));
    connect(Furnace::instance(),SIGNAL(calibrationHeaterOff()),
            this,SLOT(setOff()));
}

void CalibrationHeaterWidget::turnOnButtonClicked()
{
    if (heaterOn){
        heaterSwitchOff();
        emit turnOff();
    } else {
        heaterSwitchOn();
        emit turnOn(-1);
    }
}

void CalibrationHeaterWidget::acceptButtonClicked()
{
    heaterSwitchOn();
    emit turnOn(durationSpinBox->value());
}

void CalibrationHeaterWidget::setOff()
{
    heaterSwitchOff();
}

void CalibrationHeaterWidget::heaterSwitchOn()
{
    acceptButton->setEnabled(false);
    turnOnButton->setText(tr("Выключить"));
    statusLedIndicator->setOffColor1(defaultColorOn1LegIndicator);
    statusLedIndicator->setOffColor2(defaultColorOn2LegIndicator);
    heaterOn = true;
}

void CalibrationHeaterWidget::heaterSwitchOff()
{
    acceptButton->setEnabled(true);
    turnOnButton->setText(tr("Включить"));
    statusLedIndicator->setOffColor1(defaultColorOff1LegIndicator);
    statusLedIndicator->setOffColor2(defaultColorOff2LegIndicator);
    heaterOn = false;
}
