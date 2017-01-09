#include "coversWidget.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include "furnace.h"

CoversWidget::CoversWidget(QWidget *parent)
    : QGroupBox(parent)
{
    topCoverIsOpen = false;
    bottomCoverIsOpen = false;

    defaultColorOn1LegIndicator = QColor(0,255,0);
    defaultColorOn2LegIndicator = QColor(0,192,0);

    setTitle(tr("Крышки калориметра"));

    topCoverLedIndicator = new QLedIndicator();
    bottomCoverLedIndicator = new QLedIndicator;

    topCoverLabel = new QLabel(tr("Верхняя"));
    bottomCoverLabel = new QLabel(tr("Нижняя"));

    bothCoversButton = new QPushButton(tr("Открыть обе крышки"));
    topCoverButton = new QPushButton(tr("Открыть"));
    bottomCoverButton = new QPushButton(tr("Открыть"));

    QGridLayout * layout = new QGridLayout;
    layout->addWidget(topCoverLedIndicator,0,0,1,1);
    layout->addWidget(topCoverLabel,0,1,1,1);
    layout->addWidget(topCoverButton,0,2,1,1);
    layout->addWidget(bottomCoverLedIndicator,1,0,1,1);
    layout->addWidget(bottomCoverLabel,1,1,1,1);
    layout->addWidget(bottomCoverButton,1,2,1,1);
    layout->addWidget(bothCoversButton,2,0,1,3);
    setLayout(layout);

    connect(topCoverButton, SIGNAL(clicked()),this,SLOT(topCoverButtonClicked()));
    connect(bottomCoverButton, SIGNAL(clicked()),this,SLOT(bottomCoverButtonClicked()));
    connect(bothCoversButton, SIGNAL(clicked()),this,SLOT(bothCoversButtonClicked()));
    connect(topCoverLedIndicator, SIGNAL(clicked()),this,SLOT(topCoverLedIndicatorClicked()));
    connect(bottomCoverLedIndicator, SIGNAL(clicked()),this,SLOT(bottomCoverLedIndicatorClicked()));

    connect(this, SIGNAL(openTopCover()), Furnace::instance()->getCovers(), SLOT(openTopCover()));
    connect(this, SIGNAL(closeTopCover()), Furnace::instance()->getCovers(), SLOT(closeTopCover()));
    connect(this, SIGNAL(openBottomCover()), Furnace::instance()->getCovers(), SLOT(openBottomCover()));
    connect(this, SIGNAL(closeBottomCover()), Furnace::instance()->getCovers(), SLOT(closeBottomCover()));
    connect(this, SIGNAL(openCovers()), Furnace::instance()->getCovers(), SLOT(openCovers()));
    connect(this, SIGNAL(closeCovers()), Furnace::instance()->getCovers(), SLOT(closeCovers()));
    connect(Furnace::instance()->getCovers(),SIGNAL(openTopCoverByTimerSignal()),this,SLOT(openTopCoverByTimer()));
    connect(Furnace::instance()->getCovers(),SIGNAL(openBottomCoverByTimerSignal()),this,SLOT(openBottomCoverByTimer()));
    connect(Furnace::instance()->getCovers(),SIGNAL(closeTopCoverByTimerSignal()),this,SLOT(closeTopCoverByTimer()));
    connect(Furnace::instance()->getCovers(),SIGNAL(closeBottomCoverByTimerSignal()),this,SLOT(closeBottomCoverByTimer()));
    connect(Furnace::instance()->getCovers(),SIGNAL(openTopCoverSignal()),this,SLOT(openTopCoverByLTR()));
    connect(Furnace::instance()->getCovers(),SIGNAL(closeTopCoverSignal()),this,SLOT(closeTopCoverByLTR()));
    connect(Furnace::instance()->getCovers(),SIGNAL(openBottomCoverSignal()),this,SLOT(openBottomCoverByLTR()));
    connect(Furnace::instance()->getCovers(),SIGNAL(closeBottomCoverSignal()),this,SLOT(closeBottomCoverByLTR()));
}

CoversWidget::~CoversWidget()
{

}

void CoversWidget::topCoverButtonClicked()
{
    if (topCoverIsOpen){
        emit closeTopCover();
    } else {
        emit openTopCover();
    }
    topCoverButton->setEnabled(false);
    bothCoversButton->setEnabled(false);
}

void CoversWidget::bottomCoverButtonClicked()
{
    if (bottomCoverIsOpen){
        emit closeBottomCover();
    } else {
        emit openBottomCover();
    }
    bottomCoverButton->setEnabled(false);
    bothCoversButton->setEnabled(false);
}

void CoversWidget::bothCoversButtonClicked()
{
    if (bottomCoverIsOpen && topCoverIsOpen){
        emit closeCovers();
    } else {
        emit openCovers();
    }

    topCoverButton->setEnabled(false);
    bottomCoverButton->setEnabled(false);
    bothCoversButton->setEnabled(false);
}

void CoversWidget::topCoverLedIndicatorClicked()
{
    if (topCoverIsOpen){
        topCoverLedIndicator->setChecked(true);
    } else {
        topCoverLedIndicator->setChecked(false);
    }
}

void CoversWidget::bottomCoverLedIndicatorClicked()
{
    if (bottomCoverIsOpen){
        bottomCoverLedIndicator->setChecked(true);
    } else {
        bottomCoverLedIndicator->setChecked(false);
    }
}

void CoversWidget::openTopCoverByTimer()
{
    if(!topCoverIsOpen){
        topCoverButton->setEnabled(true);
        topCoverIsOpen = true;
        topCoverLedIndicator->setChecked(true);
        topCoverLedIndicator->setOnColor1(Qt::red);
        topCoverLedIndicator->setOnColor2(Qt::red);
        topCoverButton->setText(tr("Закрыть"));
        if (bottomCoverIsOpen){
            bothCoversButton->setEnabled(true);
            bothCoversButton->setText(tr("Закрыть обе крышки"));
        }
    }
}

void CoversWidget::closeTopCoverByTimer()
{
    if(topCoverIsOpen){
        topCoverButton->setEnabled(true);
        topCoverIsOpen = false;
        topCoverLedIndicator->setChecked(false);
        topCoverButton->setText(tr("Открыть"));
        if (!bottomCoverIsOpen){
            bothCoversButton->setEnabled(true);
            bothCoversButton->setText(tr("Открыть обе крышки"));
        }
    }
}

void CoversWidget::openBottomCoverByTimer()
{
    if(!bottomCoverIsOpen){
        bottomCoverButton->setEnabled(true);
        bottomCoverIsOpen = true;
        bottomCoverLedIndicator->setChecked(true);
        bottomCoverLedIndicator->setOnColor1(Qt::red);
        bottomCoverLedIndicator->setOnColor2(Qt::red);
        bottomCoverButton->setText(tr("Закрыть"));
        if (topCoverIsOpen){
            bothCoversButton->setEnabled(true);
            bothCoversButton->setText(tr("Закрыть обе крышки"));
        }
    }
}

void CoversWidget::closeBottomCoverByTimer()
{
    if(bottomCoverIsOpen){
        bottomCoverButton->setEnabled(true);
        bottomCoverIsOpen = false;
        bottomCoverLedIndicator->setChecked(false);
        bottomCoverButton->setText(tr("Открыть"));
        if (!topCoverIsOpen){
            bothCoversButton->setEnabled(true);
            bothCoversButton->setText(tr("Открыть обе крышки"));
        }
    }
}

void    CoversWidget::openTopCoverByLTR()
{
    topCoverButton->setEnabled(true);
    topCoverIsOpen = true;
    topCoverLedIndicator->setChecked(true);
    topCoverLedIndicator->setOnColor1(defaultColorOn1LegIndicator);
    topCoverLedIndicator->setOnColor2(defaultColorOn2LegIndicator);
    topCoverButton->setText(tr("Закрыть"));
    if (bottomCoverIsOpen){
        bothCoversButton->setEnabled(true);
        bothCoversButton->setText(tr("Закрыть обе крышки"));
    }
}

void    CoversWidget::closeTopCoverByLTR()
{
    topCoverButton->setEnabled(true);
    topCoverIsOpen = false;
    topCoverLedIndicator->setChecked(false);
    topCoverButton->setText(tr("Открыть"));
    if (!bottomCoverIsOpen){
        bothCoversButton->setEnabled(true);
        bothCoversButton->setText(tr("Открыть обе крышки"));
    }
}

void    CoversWidget::openBottomCoverByLTR()
{
    bottomCoverButton->setEnabled(true);
    bottomCoverIsOpen = true;
    bottomCoverLedIndicator->setChecked(true);
    bottomCoverLedIndicator->setOnColor1(defaultColorOn1LegIndicator);
    bottomCoverLedIndicator->setOnColor2(defaultColorOn2LegIndicator);
    bottomCoverButton->setText(tr("Закрыть"));
    if (topCoverIsOpen){
        bothCoversButton->setEnabled(true);
        bothCoversButton->setText(tr("Закрыть обе крышки"));
    }
}

void    CoversWidget::closeBottomCoverByLTR()
{
    bottomCoverButton->setEnabled(true);
    bottomCoverIsOpen = false;
    bottomCoverLedIndicator->setChecked(false);
    bottomCoverButton->setText(tr("Открыть"));
    if (!topCoverIsOpen){
        bothCoversButton->setEnabled(true);
        bothCoversButton->setText(tr("Открыть обе крышки"));
    }
}









SafetyValveWidget::SafetyValveWidget(QWidget *parent)
    : QGroupBox(parent)
{
    defaultColorOn1LegIndicator = QColor(0,255,0);
    defaultColorOn2LegIndicator = QColor(0,192,0);
    defaultColorOff1LegIndicator = QColor(0,28,0);
    defaultColorOff2LegIndicator = QColor(0,128,0);

    setTitle(tr("Отсекатель"));

    ledIndicator = new QLedIndicator();
    ledIndicator->setOffColor1(Qt::yellow);
    ledIndicator->setOffColor2(Qt::yellow);
    ledIndicator->setCheckable(false);
    label = new QLabel(tr("?"));

    QGridLayout * layout = new QGridLayout;
    layout->addWidget(ledIndicator,0,0,1,1);
    layout->addWidget(label,0,1,1,2);
    setLayout(layout);

    connect(Furnace::instance()->getSafetyValve(), SIGNAL(openSafetyValve()), this, SLOT(valveOpen()));
    connect(Furnace::instance()->getSafetyValve(), SIGNAL(closeSafetyValve()), this, SLOT(valveClose()));
    connect(Furnace::instance()->getSafetyValve(), SIGNAL(undefSafetyValve()), this, SLOT(valveUndef()));
}

SafetyValveWidget::~SafetyValveWidget()
{

}

void    SafetyValveWidget::valveOpen()
{
    ledIndicator->setOffColor1(defaultColorOn1LegIndicator);
    ledIndicator->setOffColor2(defaultColorOn2LegIndicator);
    label->setText(tr("Открыт"));
}

void    SafetyValveWidget::valveClose()
{
    ledIndicator->setOffColor1(defaultColorOff1LegIndicator);
    ledIndicator->setOffColor2(defaultColorOff2LegIndicator);
    label->setText(tr("Закрыт"));
}

void    SafetyValveWidget::valveUndef()
{
    ledIndicator->setOffColor1(Qt::red);
    ledIndicator->setOffColor2(Qt::red);
    label->setText(tr("?"));
}




SampleLockWidget::SampleLockWidget(QWidget *parent)
    : QGroupBox(parent)
{
    lockIsOpen = false;

    defaultColorOn1LegIndicator = QColor(0,255,0);
    defaultColorOn2LegIndicator = QColor(0,192,0);
    defaultColorOff1LegIndicator = QColor(0,28,0);
    defaultColorOff2LegIndicator = QColor(0,128,0);

    setTitle(tr("Замок ампулы"));

    openLockButton = new QPushButton(tr("Открыть"));
    openLockButton->setEnabled(false);
    interlockButton = new QPushButton(tr("Блокировка"));
    interlockButton->setCheckable(true);

    ledIndicator = new QLedIndicator();
    ledIndicator->setCheckable(false);

    QGridLayout * layout = new QGridLayout;
    layout->addWidget(ledIndicator,0,0);
    layout->addWidget(openLockButton,0,1);
    layout->addWidget(interlockButton,0,2);
    setLayout(layout);

    connect (openLockButton, SIGNAL(clicked()), this, SLOT(openLockButtonClicked()));
    connect (interlockButton, SIGNAL(toggled(bool)), Furnace::instance()->getSampleLock(), SLOT(setDropEnable(bool)));
    connect (this, SIGNAL(openSampleLock()), Furnace::instance()->getSampleLock(), SLOT(lockOpen()));
    connect (this, SIGNAL(closeSampleLock()), Furnace::instance()->getSampleLock(), SLOT(lockClose()));
    connect (Furnace::instance()->getSampleLock(), SIGNAL(closeLockSignal()), this, SLOT(setLockStatusClose()));
    connect (Furnace::instance()->getSampleLock(), SIGNAL(openLockSignal()), this, SLOT(setLockStatusOpen()));
    connect (Furnace::instance()->getSampleLock(), SIGNAL(dropEnableSignal(bool)), this, SLOT(setInterlock(bool)));

}

SampleLockWidget::~SampleLockWidget()
{

}

void SampleLockWidget::openLockButtonClicked()
{
    if (lockIsOpen){
        openLockButton->setText(tr("Открыть"));
        ledIndicator->setOffColor1(defaultColorOff1LegIndicator);
        ledIndicator->setOffColor2(defaultColorOff2LegIndicator);
        lockIsOpen = false;
        emit closeSampleLock();
    } else {
        openLockButton->setText(tr("Закрыть"));
        ledIndicator->setOffColor1(defaultColorOn1LegIndicator);
        ledIndicator->setOffColor2(defaultColorOn2LegIndicator);
        lockIsOpen = true;
        emit openSampleLock();
    }
}

void SampleLockWidget::setLockStatusOpen()
{
    openLockButton->setText(tr("Закрыть"));
    ledIndicator->setOffColor1(defaultColorOn1LegIndicator);
    ledIndicator->setOffColor2(defaultColorOn2LegIndicator);
    lockIsOpen = true;
}

void SampleLockWidget::setLockStatusClose()
{
    openLockButton->setText(tr("Открыть"));
    ledIndicator->setOffColor1(defaultColorOff1LegIndicator);
    ledIndicator->setOffColor2(defaultColorOff2LegIndicator);
    lockIsOpen = false;
}

void SampleLockWidget::setInterlock(bool interlockEnable)
{
    openLockButton->setEnabled(interlockEnable);
    interlockButton->setChecked(interlockEnable);
}







