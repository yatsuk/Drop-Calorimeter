/********************************************************************************
** Form generated from reading UI file 'adcSettingWidget.ui'
**
** Created: Fri 10. Apr 10:48:53 2015
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADCSETTINGWIDGET_H
#define UI_ADCSETTINGWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AdcSettingWidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_5;
    QLabel *portLabel;
    QSpacerItem *horizontalSpacer;
    QComboBox *portComboBox;
    QHBoxLayout *horizontalLayout;
    QLabel *typeThermocoupleLabel;
    QSpacerItem *horizontalSpacer_2;
    QComboBox *typeThermocoupleComboBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *coldThermocoupleLabel;
    QSpacerItem *horizontalSpacer_3;
    QDoubleSpinBox *coldThermocoupleSpinBox;
    QCheckBox *filtrCheckBox;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *enableAverageCheckBox;
    QSpinBox *averageSpinBox;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *cancelButton;
    QPushButton *acceptButton;

    void setupUi(QWidget *AdcSettingWidget)
    {
        if (AdcSettingWidget->objectName().isEmpty())
            AdcSettingWidget->setObjectName(QString::fromUtf8("AdcSettingWidget"));
        AdcSettingWidget->resize(225, 230);
        verticalLayout_2 = new QVBoxLayout(AdcSettingWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(AdcSettingWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        portLabel = new QLabel(groupBox);
        portLabel->setObjectName(QString::fromUtf8("portLabel"));

        horizontalLayout_5->addWidget(portLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer);

        portComboBox = new QComboBox(groupBox);
        portComboBox->setObjectName(QString::fromUtf8("portComboBox"));

        horizontalLayout_5->addWidget(portComboBox);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        typeThermocoupleLabel = new QLabel(groupBox);
        typeThermocoupleLabel->setObjectName(QString::fromUtf8("typeThermocoupleLabel"));
        typeThermocoupleLabel->setWordWrap(true);

        horizontalLayout->addWidget(typeThermocoupleLabel);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        typeThermocoupleComboBox = new QComboBox(groupBox);
        typeThermocoupleComboBox->setObjectName(QString::fromUtf8("typeThermocoupleComboBox"));
        typeThermocoupleComboBox->setAutoFillBackground(false);

        horizontalLayout->addWidget(typeThermocoupleComboBox);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        coldThermocoupleLabel = new QLabel(groupBox);
        coldThermocoupleLabel->setObjectName(QString::fromUtf8("coldThermocoupleLabel"));
        coldThermocoupleLabel->setWordWrap(true);

        horizontalLayout_2->addWidget(coldThermocoupleLabel);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        coldThermocoupleSpinBox = new QDoubleSpinBox(groupBox);
        coldThermocoupleSpinBox->setObjectName(QString::fromUtf8("coldThermocoupleSpinBox"));
        coldThermocoupleSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(coldThermocoupleSpinBox);


        verticalLayout->addLayout(horizontalLayout_2);

        filtrCheckBox = new QCheckBox(groupBox);
        filtrCheckBox->setObjectName(QString::fromUtf8("filtrCheckBox"));

        verticalLayout->addWidget(filtrCheckBox);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        enableAverageCheckBox = new QCheckBox(groupBox);
        enableAverageCheckBox->setObjectName(QString::fromUtf8("enableAverageCheckBox"));

        horizontalLayout_3->addWidget(enableAverageCheckBox);

        averageSpinBox = new QSpinBox(groupBox);
        averageSpinBox->setObjectName(QString::fromUtf8("averageSpinBox"));
        averageSpinBox->setEnabled(false);
        averageSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(averageSpinBox);


        verticalLayout->addLayout(horizontalLayout_3);


        verticalLayout_2->addWidget(groupBox);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        cancelButton = new QPushButton(AdcSettingWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout_4->addWidget(cancelButton);

        acceptButton = new QPushButton(AdcSettingWidget);
        acceptButton->setObjectName(QString::fromUtf8("acceptButton"));

        horizontalLayout_4->addWidget(acceptButton);


        verticalLayout_2->addLayout(horizontalLayout_4);


        retranslateUi(AdcSettingWidget);
        QObject::connect(cancelButton, SIGNAL(clicked()), AdcSettingWidget, SLOT(close()));
        QObject::connect(enableAverageCheckBox, SIGNAL(toggled(bool)), averageSpinBox, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(AdcSettingWidget);
    } // setupUi

    void retranslateUi(QWidget *AdcSettingWidget)
    {
        AdcSettingWidget->setWindowTitle(QApplication::translate("AdcSettingWidget", "\320\235\320\260\321\201\321\202\321\200\320\276\320\271\320\272\320\260", 0));
        groupBox->setTitle(QApplication::translate("AdcSettingWidget", "\320\237\320\260\321\200\320\260\320\274\320\265\321\202\321\200\321\213 \320\220\320\246\320\237", 0));
        portLabel->setText(QApplication::translate("AdcSettingWidget", "\320\235\320\276\320\274\320\265\321\200 \320\277\320\276\321\200\321\202\320\260", 0));
        portComboBox->clear();
        portComboBox->insertItems(0, QStringList()
         << QApplication::translate("AdcSettingWidget", "COM1", 0)
        );
        typeThermocoupleLabel->setText(QApplication::translate("AdcSettingWidget", "\320\242\320\270\320\277 \321\202\320\265\321\200\320\274\320\276\320\277\320\260\321\200\321\213", 0));
        typeThermocoupleComboBox->clear();
        typeThermocoupleComboBox->insertItems(0, QStringList()
         << QApplication::translate("AdcSettingWidget", "A1", 0)
         << QApplication::translate("AdcSettingWidget", "K", 0)
        );
        coldThermocoupleLabel->setText(QApplication::translate("AdcSettingWidget", "\320\242\320\265\320\274\320\277\320\265\321\200\320\260\321\202\321\203\321\200\320\260 \321\205\320\276\320\273\320\276\320\264\320\275\321\213\321\205 \321\201\320\277\320\260\320\265\320\262 (\320\241)", 0));
        filtrCheckBox->setText(QApplication::translate("AdcSettingWidget", "\320\244\320\270\320\273\321\214\321\202\321\200\320\260\321\206\320\270\321\217 \320\262\321\213\320\261\321\200\320\276\321\201\320\276\320\262", 0));
        enableAverageCheckBox->setText(QApplication::translate("AdcSettingWidget", "\320\243\321\201\321\200\320\265\320\264\320\275\320\265\320\275\320\270\320\265", 0));
        cancelButton->setText(QApplication::translate("AdcSettingWidget", "\320\236\321\202\320\274\320\265\320\275\320\260", 0));
        acceptButton->setText(QApplication::translate("AdcSettingWidget", "\320\237\321\200\320\270\320\274\320\265\320\275\320\270\321\202\321\214", 0));
    } // retranslateUi

};

namespace Ui {
    class AdcSettingWidget: public Ui_AdcSettingWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADCSETTINGWIDGET_H
