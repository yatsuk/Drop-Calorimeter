/********************************************************************************
** Form generated from reading UI file 'startupWizard.ui'
**
** Created: Fri 10. Apr 10:48:53 2015
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STARTUPWIZARD_H
#define UI_STARTUPWIZARD_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StartupWizard
{
public:
    QPushButton *pushButton;

    void setupUi(QWidget *StartupWizard)
    {
        if (StartupWizard->objectName().isEmpty())
            StartupWizard->setObjectName(QString::fromUtf8("StartupWizard"));
        StartupWizard->resize(400, 300);
        pushButton = new QPushButton(StartupWizard);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(120, 130, 75, 23));

        retranslateUi(StartupWizard);

        QMetaObject::connectSlotsByName(StartupWizard);
    } // setupUi

    void retranslateUi(QWidget *StartupWizard)
    {
        StartupWizard->setWindowTitle(QApplication::translate("StartupWizard", "Form", 0));
        pushButton->setText(QApplication::translate("StartupWizard", "PushButton", 0));
    } // retranslateUi

};

namespace Ui {
    class StartupWizard: public Ui_StartupWizard {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STARTUPWIZARD_H
