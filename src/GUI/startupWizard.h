#ifndef STARTUPWIZARD_H
#define STARTUPWIZARD_H

#include <QWidget>

namespace Ui {
class StartupWizard;
}

class StartupWizard : public QWidget
{
    Q_OBJECT
    
public:
    explicit StartupWizard(QWidget *parent = 0);
    ~StartupWizard();
    
private:
    Ui::StartupWizard *ui;
};

#endif // STARTUPWIZARD_H
