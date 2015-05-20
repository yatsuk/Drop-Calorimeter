#ifndef ADCSETTINGWIDGET_H
#define ADCSETTINGWIDGET_H

#include <QDialog>
#include "parameters.h"

namespace Ui {
class AdcSettingWidget;
}

class AdcSettingWidget : public QDialog
{
    Q_OBJECT

public:
    explicit AdcSettingWidget(QWidget *parent = 0);
    ~AdcSettingWidget();

signals:
    void parameters(AdcParameters);

public slots:
    void setParameters(const AdcParameters & parameters);

private slots:
    void acceptButtonClicked();

private:
    Ui::AdcSettingWidget *ui;
};

#endif // ADCSETTINGWIDGET_H
