#ifndef COVERSWIDGET_H
#define COVERSWIDGET_H

#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include "src/GUI/qledindicator.h"

class CoversWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit CoversWidget(QWidget *parent = 0);
    ~CoversWidget();

signals:
    void openTopCover();
    void closeTopCover();
    void openBottomCover();
    void closeBottomCover();
    void closeCovers();
    void openCovers();

private slots:
    void    bothCoversButtonClicked();
    void    topCoverButtonClicked();
    void    bottomCoverButtonClicked();
    void    topCoverLedIndicatorClicked();
    void    bottomCoverLedIndicatorClicked();
    void    openTopCoverByTimer();
    void    closeTopCoverByTimer();
    void    openBottomCoverByTimer();
    void    closeBottomCoverByTimer();
    void    openTopCoverByLTR();
    void    closeTopCoverByLTR();
    void    openBottomCoverByLTR();
    void    closeBottomCoverByLTR();

private:
    QPushButton * topCoverButton;
    QPushButton * bottomCoverButton;
    QPushButton * bothCoversButton;
    QLedIndicator * topCoverLedIndicator;
    QLedIndicator * bottomCoverLedIndicator;
    QLabel * topCoverLabel;
    QLabel * bottomCoverLabel;

    bool topCoverIsOpen;
    bool bottomCoverIsOpen;
    QColor defaultColorOn1LegIndicator;
    QColor defaultColorOn2LegIndicator;
};

class SafetyValveWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit SafetyValveWidget(QWidget *parent = 0);
    ~SafetyValveWidget();

signals:


public slots:
    void    valveOpen();
    void    valveClose();
    void    valveUndef();

private:
    QLedIndicator * ledIndicator;
    QLabel * label;

    QColor defaultColorOn1LegIndicator;
    QColor defaultColorOn2LegIndicator;
    QColor defaultColorOff1LegIndicator;
    QColor defaultColorOff2LegIndicator;
};

class SampleLockWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit SampleLockWidget(QWidget *parent = 0);
    ~SampleLockWidget();

signals:
    void openSampleLock();
    void closeSampleLock();

private slots:
    void openLockButtonClicked();
    void setLockStatusOpen();
    void setLockStatusClose();
    void setInterlock(bool interlockEnable);

private:
    QLedIndicator * ledIndicator;
    QPushButton * openLockButton;
    QPushButton * interlockButton;

    QColor defaultColorOn1LegIndicator;
    QColor defaultColorOn2LegIndicator;
    QColor defaultColorOff1LegIndicator;
    QColor defaultColorOff2LegIndicator;

    bool lockIsOpen;
};

#endif // COVERSWIDGET_H
