/********************************************************************************
** Form generated from reading UI file 'constVelocityWidget.ui'
**
** Created: Fri 10. Apr 10:48:53 2015
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONSTVELOCITYWIDGET_H
#define UI_CONSTVELOCITYWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ConstVelocityWidget
{
public:
    QVBoxLayout *verticalLayout_3;
    QRadioButton *enableWidget;
    QHBoxLayout *horizontalLayout;
    QGroupBox *directionGroupBox;
    QVBoxLayout *verticalLayout;
    QRadioButton *heatRadioButton;
    QRadioButton *curTemperatureRadioButton;
    QRadioButton *coolRadioButton;
    QVBoxLayout *verticalLayout_2;
    QLabel *velocityLabel;
    QDoubleSpinBox *velocitySpinBox;
    QPushButton *acceptButton;

    void setupUi(QWidget *ConstVelocityWidget)
    {
        if (ConstVelocityWidget->objectName().isEmpty())
            ConstVelocityWidget->setObjectName(QString::fromUtf8("ConstVelocityWidget"));
        ConstVelocityWidget->resize(224, 126);
        verticalLayout_3 = new QVBoxLayout(ConstVelocityWidget);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        enableWidget = new QRadioButton(ConstVelocityWidget);
        enableWidget->setObjectName(QString::fromUtf8("enableWidget"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        enableWidget->setFont(font);

        verticalLayout_3->addWidget(enableWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        directionGroupBox = new QGroupBox(ConstVelocityWidget);
        directionGroupBox->setObjectName(QString::fromUtf8("directionGroupBox"));
        verticalLayout = new QVBoxLayout(directionGroupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        heatRadioButton = new QRadioButton(directionGroupBox);
        heatRadioButton->setObjectName(QString::fromUtf8("heatRadioButton"));
        heatRadioButton->setChecked(false);

        verticalLayout->addWidget(heatRadioButton);

        curTemperatureRadioButton = new QRadioButton(directionGroupBox);
        curTemperatureRadioButton->setObjectName(QString::fromUtf8("curTemperatureRadioButton"));
        curTemperatureRadioButton->setChecked(true);

        verticalLayout->addWidget(curTemperatureRadioButton);

        coolRadioButton = new QRadioButton(directionGroupBox);
        coolRadioButton->setObjectName(QString::fromUtf8("coolRadioButton"));

        verticalLayout->addWidget(coolRadioButton);


        horizontalLayout->addWidget(directionGroupBox);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        velocityLabel = new QLabel(ConstVelocityWidget);
        velocityLabel->setObjectName(QString::fromUtf8("velocityLabel"));
        velocityLabel->setEnabled(false);

        verticalLayout_2->addWidget(velocityLabel);

        velocitySpinBox = new QDoubleSpinBox(ConstVelocityWidget);
        velocitySpinBox->setObjectName(QString::fromUtf8("velocitySpinBox"));
        velocitySpinBox->setEnabled(false);
        QFont font1;
        font1.setKerning(true);
        velocitySpinBox->setFont(font1);
        velocitySpinBox->setAccelerated(true);
        velocitySpinBox->setSingleStep(0.5);

        verticalLayout_2->addWidget(velocitySpinBox);

        acceptButton = new QPushButton(ConstVelocityWidget);
        acceptButton->setObjectName(QString::fromUtf8("acceptButton"));

        verticalLayout_2->addWidget(acceptButton);


        horizontalLayout->addLayout(verticalLayout_2);


        verticalLayout_3->addLayout(horizontalLayout);


        retranslateUi(ConstVelocityWidget);

        QMetaObject::connectSlotsByName(ConstVelocityWidget);
    } // setupUi

    void retranslateUi(QWidget *ConstVelocityWidget)
    {
        ConstVelocityWidget->setWindowTitle(QApplication::translate("ConstVelocityWidget", "Form", 0, QApplication::UnicodeUTF8));
        enableWidget->setText(QApplication::translate("ConstVelocityWidget", "\320\222\320\272\320\273\321\216\321\207\320\270\321\202\321\214", 0, QApplication::UnicodeUTF8));
        directionGroupBox->setTitle(QString());
        heatRadioButton->setText(QApplication::translate("ConstVelocityWidget", "\320\235\320\260\320\263\321\200\320\265\320\262", 0, QApplication::UnicodeUTF8));
        curTemperatureRadioButton->setText(QApplication::translate("ConstVelocityWidget", "\320\241\321\202\320\276\321\217\320\275\320\272\320\260", 0, QApplication::UnicodeUTF8));
        coolRadioButton->setText(QApplication::translate("ConstVelocityWidget", "\320\236\321\205\320\273\320\260\320\266\320\264\320\265\320\275\320\270\320\265", 0, QApplication::UnicodeUTF8));
        velocityLabel->setText(QApplication::translate("ConstVelocityWidget", "\320\241\320\272\320\276\321\200\320\276\321\201\321\202\321\214 (\320\232/\320\274\320\270\320\275)", 0, QApplication::UnicodeUTF8));
        velocitySpinBox->setSuffix(QString());
        acceptButton->setText(QApplication::translate("ConstVelocityWidget", "\320\237\321\200\320\270\320\274\320\265\320\275\320\270\321\202\321\214", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ConstVelocityWidget: public Ui_ConstVelocityWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONSTVELOCITYWIDGET_H
