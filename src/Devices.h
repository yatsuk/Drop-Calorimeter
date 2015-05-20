#ifndef DEVICES_H
#define DEVICES_H

#include <QObject>
#include "ltr43.h"
#include "shared.h"

class Covers : public QObject
{
    Q_OBJECT
public:
    explicit Covers(QObject *parent = 0);
    void setLTR43(Ltr43 * ltr43);

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void openTopCoverByTimerSignal();
    void openBottomCoverByTimerSignal();
    void closeTopCoverByTimerSignal();
    void closeBottomCoverByTimerSignal();
    void openTopCoverSignal();
    void openBottomCoverSignal();
    void closeTopCoverSignal();
    void closeBottomCoverSignal();

public slots:
    void openTopCover();
    void closeTopCover();
    void openBottomCover();
    void closeBottomCover();
    void statusPortLtr43(DWORD status);

private slots:
    void lowVoltage();
    void higthVoltage();
    void openTopCoverByTimer();
    void openBottomCoverByTimer();
    void closeTopCoverByTimer();
    void closeBottomCoverByTimer();
    void openTopCoverByLTR();
    void openBottomCoverByLTR();
    void closeTopCoverByLTR();
    void closeBottomCoverByLTR();


private:
    Ltr43 * ltr43_;
    bool topCoverIsOpen;
    bool bottomCoverIsOpen;
    bool timerTopCoverIsStop;
    bool timerBottomCoverIsStop;
};



class SafetyValve : public QObject
{
    Q_OBJECT
public:
    explicit SafetyValve(QObject *parent = 0);
    void setLTR43(Ltr43 * ltr43);

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void openSafetyValve();
    void closeSafetyValve();
    void undefSafetyValve();
    void remoteDropSignal();
    void remoteDropCompletedSignal();

public slots:
    void statusPortLtr43(DWORD status);
    void setRemoteDropEnable(bool enable);

private:
    Ltr43 * ltr43_;
    bool SafetyValveIsOpen;
    bool SafetyValveIsClose;
    bool SafetyValveIsUndef;
    bool remoteDropEnable;
};

class SampleLock : public QObject
{
    Q_OBJECT
public:
    explicit SampleLock(QObject *parent = 0);
    void setLTR43(Ltr43 * ltr43);

signals:
    void message(const QString & msg, Shared::MessageLevel msgLevel);
    void dropEnableSignal(bool);
    void openLockSignal();
    void closeLockSignal();

public slots:
    void setDropEnable(bool enable);
    void statusPortLtr43(DWORD status);
    void lockOpen();
    void lockClose();
    void drop();

private slots:
    void lowVoltage();
    void higthVoltage();

private:
    Ltr43 * ltr43_;
    bool dropEnable;
    bool coverIsOpen;
    bool safetyValveIsOpen;
    bool lockIsOpen;
    bool remoteOpenLockStatePrev;
};

#endif // DEVICES_H
