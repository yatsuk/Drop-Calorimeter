/********************************************************************************
** Form generated from reading UI file 'safetyValveWidget.ui'
**
** Created: Fri 10. Apr 10:48:53 2015
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAFETYVALVEWIDGET_H
#define UI_SAFETYVALVEWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_safetyValveWidget
{
public:

    void setupUi(QGroupBox *safetyValveWidget)
    {
        if (safetyValveWidget->objectName().isEmpty())
            safetyValveWidget->setObjectName(QString::fromUtf8("safetyValveWidget"));
        safetyValveWidget->resize(400, 300);

        retranslateUi(safetyValveWidget);

        QMetaObject::connectSlotsByName(safetyValveWidget);
    } // setupUi

    void retranslateUi(QGroupBox *safetyValveWidget)
    {
        safetyValveWidget->setTitle(QApplication::translate("safetyValveWidget", "GroupBox", 0, QApplication::UnicodeUTF8));
        safetyValveWidget->setWindowTitle(QApplication::translate("safetyValveWidget", "GroupBox", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class safetyValveWidget: public Ui_safetyValveWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAFETYVALVEWIDGET_H
