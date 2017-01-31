#include "Devices.h"
#include <QTimer>
#include <QApplication>
#include <QDebug>

Covers::Covers(QObject *parent) :
    QObject(parent)
{
    topCoverIsOpen = false;
    bottomCoverIsOpen = false;
    timerBottomCoverIsStop = true;
    timerTopCoverIsStop = true;
}

void Covers::setLTR43(Ltr43 * ltr43)
{
    ltr43_ = ltr43;
}

void Covers::openTopCoverByTimer()
{
    if (!timerTopCoverIsStop){
        emit message(tr("Верхняя крышка открыта."),Shared::information);
        emit openTopCoverByTimerSignal();
    }
}

void Covers::openBottomCoverByTimer()
{
    if (!timerBottomCoverIsStop){
        emit message(tr("Нижняя крышка открыта."),Shared::information);
        emit openBottomCoverByTimerSignal();
    }
}

void Covers::closeTopCoverByTimer()
{
    if (!timerTopCoverIsStop){
        emit message(tr("Верхняя крышка закрыта."),Shared::information);
        emit closeTopCoverByTimerSignal();
    }
}

void Covers::closeBottomCoverByTimer()
{
    if (!timerBottomCoverIsStop){
        emit message(tr("Нижняя крышка закрыта."),Shared::information);
        emit closeBottomCoverByTimerSignal();
    }
}

void Covers::openTopCoverByLTR()
{
    emit message(tr("Верхняя крышка открыта."),Shared::information);
    emit openTopCoverSignal();
    timerTopCoverIsStop = true;
    topCoverIsOpen = true;
}

void Covers::openBottomCoverByLTR()
{
    emit message(tr("Нижняя крышка открыта."),Shared::information);
    emit openBottomCoverSignal();
    timerBottomCoverIsStop = true;
    bottomCoverIsOpen = true;
}

void Covers::closeTopCoverByLTR()
{
    timerTopCoverIsStop = true;
    topCoverIsOpen = false;
    emit message(tr("Верхняя крышка закрыта."),Shared::information);
    emit closeTopCoverSignal();
}

void Covers::closeBottomCoverByLTR()
{
    timerBottomCoverIsStop = true;
    bottomCoverIsOpen = false;
    emit message(tr("Нижняя крышка закрыта."),Shared::information);
    emit closeBottomCoverSignal();
}

void Covers::statusPortLtr43(DWORD status)
{
    unsigned char port;

    port= status >> 16;
    unsigned char portInvert = port^0xFF;
    unsigned char topCoverPin = 1 << 5;
    unsigned char bottomCoverPin = 1 << 2;

    bool topCoverStateOpen = ((portInvert & topCoverPin)== topCoverPin) ? true : false;
    bool bottomCoverStateOpen = ((portInvert & bottomCoverPin)== bottomCoverPin) ? true : false;


    if(topCoverIsOpen && !topCoverStateOpen){
        closeTopCoverByLTR();
    }

    if(bottomCoverIsOpen && !bottomCoverStateOpen){
        closeBottomCoverByLTR();
    }

    if (!topCoverIsOpen && topCoverStateOpen){
        openTopCoverByLTR();
    }

    if(!bottomCoverIsOpen && bottomCoverStateOpen){
        openBottomCoverByLTR();
    }

}

void Covers::openTopCover()
{
    higthVoltage();
    ltr43_->writePort(3, 4, true);
    QTimer::singleShot(1000,this,SLOT(lowVoltage()));
    QTimer::singleShot(2000,this,SLOT(openTopCoverByTimer()));
    timerTopCoverIsStop = false;
}

void Covers::closeTopCover()
{
    ltr43_->writePort(3, 4, false);
    QTimer::singleShot(1000,this,SLOT(closeTopCoverByTimer()));
    timerTopCoverIsStop = false;
}

void Covers::openBottomCover()
{
    higthVoltage();
    ltr43_->writePort(3, 1, true);
    QTimer::singleShot(1000,this,SLOT(lowVoltage()));
    QTimer::singleShot(2000,this,SLOT(openBottomCoverByTimer()));
    timerBottomCoverIsStop = false;
}

void Covers::closeBottomCover()
{
    ltr43_->writePort(3, 1, false);
    QTimer::singleShot(1000,this,SLOT(closeBottomCoverByTimer()));
    timerBottomCoverIsStop = false;
}

void Covers::closeCovers()
{
    ltr43_->writePort(3, 1 | 4, false);
    QTimer::singleShot(1000,this,SLOT(closeTopCoverByTimer()));
    timerTopCoverIsStop = false;
    QTimer::singleShot(1000,this,SLOT(closeBottomCoverByTimer()));
    timerBottomCoverIsStop = false;
}

void Covers::openCovers()
{
    higthVoltage();
    ltr43_->writePort(3, 1 | 4, true);
    QTimer::singleShot(1000,this,SLOT(lowVoltage()));
    QTimer::singleShot(1000,this,SLOT(closeTopCoverByTimer()));
    QTimer::singleShot(2000,this,SLOT(openBottomCoverByTimer()));
    timerTopCoverIsStop = false;
    timerBottomCoverIsStop = false;
}

void Covers::lowVoltage()
{
    ltr43_->writePort(3, 2, false);
}

void Covers::higthVoltage()
{
    ltr43_->writePort(3, 2, true);
}










SafetyValve::SafetyValve(QObject *parent) :
    QObject(parent)
{
    SafetyValveIsOpen = false;
    SafetyValveIsClose = false;
    SafetyValveIsUndef = false;
    remoteDropEnable = false;
}

void SafetyValve::setLTR43(Ltr43 * ltr43)
{
    ltr43_ = ltr43;
}

void SafetyValve::setRemoteDropEnable(bool enable)
{
    remoteDropEnable = enable;
}

void SafetyValve::statusPortLtr43(DWORD status)
{
    unsigned char port;

    port= status >> 16;
    unsigned char portInvert = port^0xFF;
    unsigned char valveOpenPin = 1 << 4;
    unsigned char valveClosePin = 1 << 3;


    bool valveStateOpen = ((portInvert & valveOpenPin)== valveOpenPin) ? true : false;
    bool valveStateClose = ((portInvert & valveClosePin)== valveClosePin) ? true : false;
    bool valveStatusUndef = (valveStateOpen || valveStateClose) ? false : true;

    if(valveStateOpen && !SafetyValveIsOpen){
        SafetyValveIsOpen = true;
        SafetyValveIsUndef = false;
        emit message(tr("Отсекатель открыт."),Shared::information);
        emit openSafetyValve();
        if (remoteDropEnable)
            emit remoteDropSignal();
    }

    if(valveStateClose && !SafetyValveIsClose){
        SafetyValveIsClose = true;
        SafetyValveIsUndef = false;
        emit message(tr("Отсекатель закрыт."),Shared::information);
        emit closeSafetyValve();
        emit remoteDropCompletedSignal();
    }

    if (valveStatusUndef && !SafetyValveIsUndef){
        SafetyValveIsOpen = false;
        SafetyValveIsClose = false;
        SafetyValveIsUndef = true;
        emit undefSafetyValve();
    }

}










SampleLock::SampleLock(QObject *parent) :
    QObject(parent)
{
    dropEnable = false;
    lockIsOpen = false;
}

void SampleLock::setLTR43(Ltr43 * ltr43)
{
    ltr43_ = ltr43;
}

void SampleLock::setDropEnable(bool enable)
{
    dropEnable = enable;
    emit dropEnableSignal(enable);
}

void SampleLock::lockOpen()
{
    if (dropEnable){
        higthVoltage();
        ltr43_->writePort(3, 8, true);
        lockIsOpen = true;
        emit message(tr("Замок открыт."),Shared::information);
        emit openLockSignal();
        QTimer::singleShot(1500,this,SLOT(lowVoltage()));
    }
}

void SampleLock::lockClose()
{
    ltr43_->writePort(3, 8, false);
    lockIsOpen = false;
    emit message(tr("Замок закрыт."),Shared::information);
    emit closeLockSignal();
}

void SampleLock::lowVoltage()
{
    ltr43_->writePort(3, 2, false);
}

void SampleLock::higthVoltage()
{
    ltr43_->writePort(3, 2, true);
}

void SampleLock::statusPortLtr43(DWORD status)
{
    unsigned char port;

    port= status >> 16;
    unsigned char portInvert = port^0xFF;
    unsigned char remoteOpenLockPin = 1 << 1;

    bool remoteOpenLockState = ((portInvert & remoteOpenLockPin)== remoteOpenLockPin) ? true : false;

    if (remoteOpenLockState!=remoteOpenLockStatePrev){
        if (lockIsOpen){
            lockClose();
        } else {
            lockOpen();
        }
        remoteOpenLockStatePrev = remoteOpenLockState;
    }
}






DropDevice::DropDevice(QObject *parent) :
    QObject(parent),
    topCoverState(CoverState::Undef),
    bottomCoverState(CoverState::Undef),
    sampleLockState(SampleLockState::Undef),
    covers(0),
    dropSensor(0),
    sampleLock(0),
    safetyValve(0),
    isInited(false),
    dropReady(false)
{

}

bool DropDevice::init()
{
    if (covers && dropSensor && sampleLock && safetyValve){
        isInited = true;

        connect (safetyValve, SIGNAL(remoteDropSignal()), this, SLOT(drop()));
        connect (safetyValve, SIGNAL(closeSafetyValve()), this, SLOT(closeCovers()));
        connect (covers, SIGNAL(openTopCoverSignal()), this, SLOT(topCoverOpened()));
        connect (covers, SIGNAL(closeTopCoverSignal()), this, SLOT(topCoverClosed()));
        connect (covers, SIGNAL(openBottomCoverSignal()), this, SLOT(bottomCoverOpened()));
        connect (covers, SIGNAL(closeBottomCoverSignal()), this, SLOT(bottomCoverClosed()));
        connect (sampleLock, SIGNAL(openLockSignal()), this, SLOT(sampleLockOpened()));
        connect (sampleLock, SIGNAL(closeLockSignal()), this, SLOT(sampleLockClosed()));
        connect (dropSensor, SIGNAL(droped()), this, SLOT(dropped()));
    }

    return isInited;
}

void DropDevice::drop()
{
    if (isInited){
        covers->openCovers();
        dropReady = true;
        emit message(tr("Автоматический сброс ампулы по сигналу отсекателя."),Shared::information);
    }
}

void DropDevice::dropped()
{
    if (isInited){
        QApplication::beep();
        emit message(tr("Время падения ампулы = %1 мс").arg(timeDrop.elapsed()),Shared::warning);
        covers->closeCovers();
        sampleLock->lockClose();
    }
}

void DropDevice::closeCovers()
{
    if (isInited && dropReady){
        covers->closeCovers();
        sampleLock->lockClose();
    }
}

void DropDevice::openLockSample()
{
    if (topCoverState == CoverState::Open && bottomCoverState == CoverState::Open){
        timeOpenCovers.start();

        sampleLock->lockOpen();
        timeDrop.start();

        dropSensor->waitDrop();
    }
}

void DropDevice::topCoverOpened()
{
    topCoverState = CoverState::Open;
    if (dropReady)
        openLockSample();
}

void DropDevice::topCoverClosed()
{
    topCoverState = CoverState::Close;
    if (dropReady){
        if (bottomCoverState == CoverState::Open){
            emit message(tr("Время засветки калориметрического блока= %1 мс").arg(timeOpenCovers.elapsed()),Shared::warning);
        } else {
            emit message(tr("Сброс ампулы выполнен."),Shared::information);
            dropReady = false;
        }
    }
}

void DropDevice::bottomCoverOpened()
{
    bottomCoverState = CoverState::Open;
    if (dropReady)
        openLockSample();
}

void DropDevice::bottomCoverClosed()
{
    bottomCoverState = CoverState::Close;
    if (dropReady){
        if (topCoverState == CoverState::Open){
            emit message(tr("Время засветки калориметрического блока= %1 мс").arg(timeOpenCovers.elapsed()),Shared::warning);
        } else {
            emit message(tr("Сброс ампулы выполнен."),Shared::information);
            dropReady = false;
        }
    }
}

void DropDevice::sampleLockOpened()
{
    sampleLockState = SampleLockState::Open;
}

void DropDevice::sampleLockClosed()
{
    sampleLockState = SampleLockState::Close;
}








