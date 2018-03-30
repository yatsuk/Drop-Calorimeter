#include "calorimetrBlockWidget.h"
#include "src/furnace.h"
#include <QVBoxLayout>

CalorimetrBlockWidget::CalorimetrBlockWidget(QWidget *parent) :
    QWidget(parent)
{
    calibrationHeaterWidget = new CalibrationHeaterWidget;
    coversWidget = new CoversWidget;
    safetyValveWidget = new SafetyValveWidget;
    sampleLockWidget = new SampleLockWidget;
    calorimeterSignalsView = new CalorimeterSignalsView;
    experimentRecorderWidget = new ExperimentRecorderWidget;


    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(experimentRecorderWidget);
    layout->addWidget(calibrationHeaterWidget);
    layout->addWidget(sampleLockWidget);
    layout->addWidget(coversWidget);
    layout->addWidget(safetyValveWidget);
    layout->addWidget(calorimeterSignalsView);
    layout->addStretch();

    setLayout(layout);
}

void CalorimetrBlockWidget::updateState(const json & json)
{
    calorimeterSignalsView->updateState(json);
}

void CalorimetrBlockWidget::setValue(TerconData terconData)
{
    calorimeterSignalsView->setValue(terconData);
}



ExperimentRecorderWidget::ExperimentRecorderWidget(QWidget *parent) :
    QGroupBox(parent)
{
    isExperimentRecordStart = false;

    defaultColorOn1LegIndicator = QColor(0,255,0);
    defaultColorOn2LegIndicator = QColor(0,192,0);
    defaultColorOff1LegIndicator = QColor(0,28,0);
    defaultColorOff2LegIndicator = QColor(0,128,0);

    setTitle(tr("Запись эксперимента"));
    statusLedIndicator = new QLedIndicator;
    statusLedIndicator->setCheckable(false);
    statusLedIndicator->setOffColor1(defaultColorOff1LegIndicator);
    statusLedIndicator->setOffColor2(defaultColorOff2LegIndicator);

    beginDataRecordButton = new QPushButton(tr("Запись вкл"));
    beginDataRecordButton->setCheckable(true);
    endDataRecordButton = new QPushButton(tr("Запись выкл"));
    endDataRecordButton->setCheckable(true);
    endDataRecordButton->setChecked(true);

    QGridLayout * layout = new QGridLayout;
    layout->addWidget(statusLedIndicator,0,0);
    layout->addWidget(beginDataRecordButton,0,1);
    layout->addWidget(endDataRecordButton,0,2);

    setLayout(layout);

    connect(beginDataRecordButton,SIGNAL(clicked()),this,SLOT(beginDataRecordClicked()));
    connect(endDataRecordButton,SIGNAL(clicked()),this,SLOT(endDataRecordClicked()));
    connect(beginDataRecordButton,SIGNAL(clicked()),Furnace::instance(),SLOT(beginDataRecord()));
    connect(endDataRecordButton,SIGNAL(clicked()),Furnace::instance(),SLOT(endDataRecord()));
}

void ExperimentRecorderWidget::beginDataRecordClicked()
{
    statusLedIndicator->setOffColor1(defaultColorOn1LegIndicator);
    statusLedIndicator->setOffColor2(defaultColorOn2LegIndicator);
    if (!isExperimentRecordStart){
        isExperimentRecordStart = true;
        beginDataRecordButton->setChecked(true);
        endDataRecordButton->setChecked(false);
    }
}

void ExperimentRecorderWidget::endDataRecordClicked()
{
    statusLedIndicator->setOffColor1(defaultColorOff1LegIndicator);
    statusLedIndicator->setOffColor2(defaultColorOff2LegIndicator);
    if (isExperimentRecordStart){
        isExperimentRecordStart = false;
        beginDataRecordButton->setChecked(false);
        endDataRecordButton->setChecked(true);
    }

}
