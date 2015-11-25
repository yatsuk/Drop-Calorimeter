#include "coversAndCalHeaterWidget.h"
#include <QVBoxLayout>

CoversAndCalHeaterWidget::CoversAndCalHeaterWidget(QWidget *parent) :
    QWidget(parent)
{
    calibrationHeaterWidget = new CalibrationHeaterWidget;
    coversWidget = new CoversWidget;
    safetyValveWidget = new SafetyValveWidget;
    sampleLockWidget = new SampleLockWidget;


    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(calibrationHeaterWidget);
    layout->addWidget(sampleLockWidget);
    layout->addWidget(coversWidget);
    layout->addWidget(safetyValveWidget);
    layout->addStretch();

    setLayout(layout);
}
