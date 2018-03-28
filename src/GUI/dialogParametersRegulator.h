#ifndef DIALOGPARAMETERSREGULATOR_H
#define DIALOGPARAMETERSREGULATOR_H

#include <QDialog>
#include "src/parameters.h"

namespace Ui {
class DialogParametersRegulator;
}

class DialogParametersRegulator : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogParametersRegulator(QWidget *parent = 0);
    ~DialogParametersRegulator();

signals:
    void parameters(RegulatorParameters);

public slots:
    void setParameters(const RegulatorParameters & parameters);

private slots:
    void acceptButtonClicked();
private:
    Ui::DialogParametersRegulator *ui;
};

#endif // DIALOGPARAMETERSREGULATOR_H
