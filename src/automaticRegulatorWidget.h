#ifndef AUTOMATICREGULATORWIDGET_H
#define AUTOMATICREGULATORWIDGET_H

#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QTableWidget>
#include <QPushButton>
#include <QRadioButton>
#include "temperatureSegment.h"
#include "addTempSegmentDialog.h"
#include "segments.h"

class AutomaticRegulatorWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit AutomaticRegulatorWidget(QWidget *parent = 0);
    void setTemperatureProgramm(Segments * tProgramm);
    
signals:
    void autoRegulatorEnabled();
    void regCurrentTemperature(bool);
    void goToSegment(int);
    
public slots:
    void setEnabledWidget(bool enabled);
    void setRegulatorOn(bool on);
    bool isEnabledWidget();
    void setCurrentTemperatureSegment(int segmentNumber);
    void setTotalDuration(double duration);
    void setRemainingTime(double time);

private slots:
    void autoModeOn();
    void checkBoxReleased();
    void addSegment();
    void deleteSegment();
    void updateTable();
    void goToSegmentClicked();
    void saveTemperatureProgramm();
    void loadTemperatureProgramm();
    void editTemperatureProgramm();

private:
    QLabel * autoRegulatorLabel;
    QLabel * autoRegulatorOperationLabel;
    QLabel * totalDurationLabel;
    QLabel * remainingTimeLabel;
    QRadioButton * enableAutoRegulator;
    QTableWidget * tableSegments;
    QPushButton * addSegmentButton;
    QPushButton * deleteSegmentButton;
    QPushButton * modifSegmentButton;
    QPushButton * goToSelectedSegmentButton;
    QPushButton * saveProgrammButton;
    QPushButton * loadProgrammButton;
    QPushButton * stopCurrentTemperature;

    AddTempSegmentDialog * addSegmentDialog;

    Segments * temperatureSegments;
    bool regulatorOn;
};

#endif // AUTOMATICREGULATORWIDGET_H
