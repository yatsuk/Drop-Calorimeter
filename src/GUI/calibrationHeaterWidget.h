#ifndef CALIBRATIONHEATERWIDGET_H
#define CALIBRATIONHEATERWIDGET_H

#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <qledindicator.h>

class CalibrationHeaterWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit CalibrationHeaterWidget(QWidget *parent = 0);
    
signals:
    void turnOn(int);
    void turnOff();
    
public slots:
    void setOff();

private slots:
    void turnOnButtonClicked();
    void acceptButtonClicked();

private:
    void heaterSwitchOn();
    void heaterSwitchOff();

    QLedIndicator * statusLedIndicator;
    QLabel  * statusLabel;
    QPushButton * turnOnButton;
    QPushButton * acceptButton;
    QSpinBox * durationSpinBox;

    bool heaterOn;
    QColor defaultColorOn1LegIndicator;
    QColor defaultColorOn2LegIndicator;
    QColor defaultColorOff1LegIndicator;
    QColor defaultColorOff2LegIndicator;
};

#endif // CALIBRATIONHEATERWIDGET_H
