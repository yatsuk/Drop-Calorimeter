#ifndef ADDTEMPSEGMENTDIALOG_H
#define ADDTEMPSEGMENTDIALOG_H

#include "src/temperatureSegment.h"
#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

class AddTempSegmentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddTempSegmentDialog(QWidget *parent = 0);
    
signals:
    void newSegment(Segment *);

private slots:
    void typeSegmentChanged(int index);
    void cancelButtonClicked();
    void acceptButtonClicked();
    void isChanged();

private:
    QStringList typesSegment;

    QComboBox * typeSegment;
    QLabel * typeLabel;

    QGroupBox * dynSegmentBox;
    QDoubleSpinBox * maxTemperatureDynSegment;
    QDoubleSpinBox * velocityDynSegment;
    QLabel * dynTemperLabel;
    QLabel * dynVelocityLabel;

    QGroupBox * isotermSegmentBox;
    QSpinBox * timeIsotermSegment;
    QLabel * isotermTimeLabel;

    QPushButton * acceptButton;
    QPushButton * cancelButton;  

};

#endif // ADDTEMPSEGMENTDIALOG_H
