#ifndef COVERSANDCALHEATERWIDGET_H
#define COVERSANDCALHEATERWIDGET_H

#include <QWidget>
#include "calibrationHeaterWidget.h"
#include "coversWidget.h"

class CoversAndCalHeaterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CoversAndCalHeaterWidget(QWidget *parent = 0);

signals:

public slots:

private:
    CalibrationHeaterWidget * calibrationHeaterWidget;
    CoversWidget * coversWidget;
    SafetyValveWidget * safetyValveWidget;
    SampleLockWidget * sampleLockWidget;

};

#endif // COVERSANDCALHEATERWIDGET_H
