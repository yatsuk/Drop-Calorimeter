/********************************************************************************
** Form generated from reading UI file 'dialogParametersRegulator.ui'
**
** Created: Fri 10. Apr 10:48:52 2015
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGPARAMETERSREGULATOR_H
#define UI_DIALOGPARAMETERSREGULATOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogParametersRegulator
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *propLabel;
    QSpacerItem *horizontalSpacer;
    QDoubleSpinBox *propSpinBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *deffLabel;
    QSpacerItem *horizontalSpacer_2;
    QDoubleSpinBox *deffSpinBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *integrLabel;
    QSpacerItem *horizontalSpacer_3;
    QDoubleSpinBox *integrSpinBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *maxIntegLabel;
    QSpacerItem *horizontalSpacer_4;
    QDoubleSpinBox *maxIntegSpinBox;
    QHBoxLayout *horizontalLayout_5;
    QLabel *offsetLabel;
    QSpacerItem *horizontalSpacer_5;
    QDoubleSpinBox *offsetSpinBox;
    QHBoxLayout *horizontalLayout_6;
    QLabel *minPowerLabel;
    QSpacerItem *horizontalSpacer_6;
    QDoubleSpinBox *minPowerSpinBox;
    QHBoxLayout *horizontalLayout_7;
    QLabel *maxPowerLabel;
    QSpacerItem *horizontalSpacer_7;
    QDoubleSpinBox *maxPowerSpinBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogParametersRegulator)
    {
        if (DialogParametersRegulator->objectName().isEmpty())
            DialogParametersRegulator->setObjectName(QString::fromUtf8("DialogParametersRegulator"));
        DialogParametersRegulator->resize(257, 270);
        verticalLayout_2 = new QVBoxLayout(DialogParametersRegulator);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(DialogParametersRegulator);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        propLabel = new QLabel(groupBox);
        propLabel->setObjectName(QString::fromUtf8("propLabel"));

        horizontalLayout->addWidget(propLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        propSpinBox = new QDoubleSpinBox(groupBox);
        propSpinBox->setObjectName(QString::fromUtf8("propSpinBox"));
        propSpinBox->setSingleStep(0.01);

        horizontalLayout->addWidget(propSpinBox);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        deffLabel = new QLabel(groupBox);
        deffLabel->setObjectName(QString::fromUtf8("deffLabel"));

        horizontalLayout_2->addWidget(deffLabel);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        deffSpinBox = new QDoubleSpinBox(groupBox);
        deffSpinBox->setObjectName(QString::fromUtf8("deffSpinBox"));
        deffSpinBox->setMaximum(300);
        deffSpinBox->setSingleStep(0.01);

        horizontalLayout_2->addWidget(deffSpinBox);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        integrLabel = new QLabel(groupBox);
        integrLabel->setObjectName(QString::fromUtf8("integrLabel"));

        horizontalLayout_3->addWidget(integrLabel);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);

        integrSpinBox = new QDoubleSpinBox(groupBox);
        integrSpinBox->setObjectName(QString::fromUtf8("integrSpinBox"));
        integrSpinBox->setDecimals(4);
        integrSpinBox->setMaximum(10);
        integrSpinBox->setSingleStep(0.0001);

        horizontalLayout_3->addWidget(integrSpinBox);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        maxIntegLabel = new QLabel(groupBox);
        maxIntegLabel->setObjectName(QString::fromUtf8("maxIntegLabel"));

        horizontalLayout_4->addWidget(maxIntegLabel);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_4);

        maxIntegSpinBox = new QDoubleSpinBox(groupBox);
        maxIntegSpinBox->setObjectName(QString::fromUtf8("maxIntegSpinBox"));

        horizontalLayout_4->addWidget(maxIntegSpinBox);


        verticalLayout->addLayout(horizontalLayout_4);


        verticalLayout_2->addWidget(groupBox);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        offsetLabel = new QLabel(DialogParametersRegulator);
        offsetLabel->setObjectName(QString::fromUtf8("offsetLabel"));

        horizontalLayout_5->addWidget(offsetLabel);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_5);

        offsetSpinBox = new QDoubleSpinBox(DialogParametersRegulator);
        offsetSpinBox->setObjectName(QString::fromUtf8("offsetSpinBox"));

        horizontalLayout_5->addWidget(offsetSpinBox);


        verticalLayout_2->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        minPowerLabel = new QLabel(DialogParametersRegulator);
        minPowerLabel->setObjectName(QString::fromUtf8("minPowerLabel"));

        horizontalLayout_6->addWidget(minPowerLabel);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_6);

        minPowerSpinBox = new QDoubleSpinBox(DialogParametersRegulator);
        minPowerSpinBox->setObjectName(QString::fromUtf8("minPowerSpinBox"));

        horizontalLayout_6->addWidget(minPowerSpinBox);


        verticalLayout_2->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        maxPowerLabel = new QLabel(DialogParametersRegulator);
        maxPowerLabel->setObjectName(QString::fromUtf8("maxPowerLabel"));

        horizontalLayout_7->addWidget(maxPowerLabel);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_7);

        maxPowerSpinBox = new QDoubleSpinBox(DialogParametersRegulator);
        maxPowerSpinBox->setObjectName(QString::fromUtf8("maxPowerSpinBox"));

        horizontalLayout_7->addWidget(maxPowerSpinBox);


        verticalLayout_2->addLayout(horizontalLayout_7);

        buttonBox = new QDialogButtonBox(DialogParametersRegulator);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);

        buttonBox->raise();
        groupBox->raise();

        retranslateUi(DialogParametersRegulator);
        QObject::connect(buttonBox, SIGNAL(accepted()), DialogParametersRegulator, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DialogParametersRegulator, SLOT(reject()));

        QMetaObject::connectSlotsByName(DialogParametersRegulator);
    } // setupUi

    void retranslateUi(QDialog *DialogParametersRegulator)
    {
        DialogParametersRegulator->setWindowTitle(QApplication::translate("DialogParametersRegulator", "Dialog", 0));
        groupBox->setTitle(QApplication::translate("DialogParametersRegulator", "\320\237\320\260\321\200\320\260\320\274\320\265\321\202\321\200\321\213 \321\200\320\265\320\263\321\203\320\273\320\270\321\200\320\276\320\262\320\260\320\275\320\270\321\217", 0));
        propLabel->setText(QApplication::translate("DialogParametersRegulator", "\320\237\321\200\320\276\320\277\320\276\321\200\321\206\320\270\320\276\320\275\320\260\320\273\321\214\320\275\321\213\320\271", 0));
        deffLabel->setText(QApplication::translate("DialogParametersRegulator", "\320\224\320\270\321\204\321\204\320\265\321\200\320\265\320\275\321\206\320\270\320\260\320\273\321\214\320\275\321\213\320\271", 0));
        integrLabel->setText(QApplication::translate("DialogParametersRegulator", "\320\230\320\275\321\202\320\265\320\263\321\200\320\260\320\273\321\214\320\275\321\213\320\271", 0));
        maxIntegLabel->setText(QApplication::translate("DialogParametersRegulator", "Max \320\270\320\275\321\202\320\265\320\263\321\200\320\260\320\273\321\214\320\275\320\276\320\265 (%)", 0));
        maxIntegSpinBox->setSuffix(QString());
        offsetLabel->setText(QApplication::translate("DialogParametersRegulator", "\320\241\320\274\320\265\321\211\320\265\320\275\320\270\320\265 (%)", 0));
        minPowerLabel->setText(QApplication::translate("DialogParametersRegulator", "Min \320\274\320\276\321\211\320\275\320\276\321\201\321\202\321\214 (%)", 0));
        maxPowerLabel->setText(QApplication::translate("DialogParametersRegulator", "Max \320\274\320\276\321\211\320\275\320\276\321\201\321\202\321\214 (%)", 0));
    } // retranslateUi

};

namespace Ui {
    class DialogParametersRegulator: public Ui_DialogParametersRegulator {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGPARAMETERSREGULATOR_H
