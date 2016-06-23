#include "Devices.h"
#include <QTimer>
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
        emit openTopCoverByTimerSignal();
        emit message(tr("Верхняя крышка открыта."),Shared::information);
    }
}

void Covers::openBottomCoverByTimer()
{
    if (!timerBottomCoverIsStop){
        emit openBottomCoverByTimerSignal();
        emit message(tr("Нижняя крышка открыта."),Shared::information);
    }
}

void Covers::closeTopCoverByTimer()
{
    if (!timerTopCoverIsStop){
        emit closeTopCoverByTimerSignal();
        emit message(tr("Верхняя крышка закрыта."),Shared::information);
    }
}

void Covers::closeBottomCoverByTimer()
{
    if (!timerBottomCoverIsStop){
        emit closeBottomCoverByTimerSignal();
        emit message(tr("Нижняя крышка закрыта."),Shared::information);
    }
}

void Covers::openTopCoverByLTR()
{
    timerTopCoverIsStop = true;
    topCoverIsOpen = true;
    QTimer::singleShot(1000,this,SLOT(lowVoltage()));
    emit openTopCoverSignal();
    emit message(tr("Верхняя крышка открыта."),Shared::information);
}

void Covers::openBottomCoverByLTR()
{
    timerBottomCoverIsStop = true;
    bottomCoverIsOpen = true;
    QTimer::singleShot(1000,this,SLOT(lowVoltage()));
    emit openBottomCoverSignal();
    emit message(tr("Нижняя крышка открыта."),Shared::information);
}

void Covers::closeTopCoverByLTR()
{
    timerTopCoverIsStop = true;
    topCoverIsOpen = false;
    emit closeTopCoverSignal();
    emit message(tr("Верхняя крышка закрыта."),Shared::information);
}

void Covers::closeBottomCoverByLTR()
{
    timerBottomCoverIsStop = true;
    bottomCoverIsOpen = false;
    emit closeBottomCoverSignal();
    emit message(tr("Нижняя крышка закрыта."),Shared::information);
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
    ltr43_->writePort(3, 2, true);
    QTimer::singleShot(1000,this,SLOT(lowVoltage()));
    QTimer::singleShot(2000,this,SLOT(openTopCoverByTimer()));
    timerTopCoverIsStop = false;
}

void Covers::closeTopCover()
{
    ltr43_->writePort(3, 2, false);
    QTimer::singleShot(1000,this,SLOT(closeTopCoverByTimer()));
    timerTopCoverIsStop = false;
}

void Covers::openBottomCover()
{
    higthVoltage();
    ltr43_->writePort(3, 0, true);
    QTimer::singleShot(1000,this,SLOT(lowVoltage()));
    QTimer::singleShot(2000,this,SLOT(openBottomCoverByTimer()));
    timerBottomCoverIsStop = false;
}

void Covers::closeBottomCover()
{
    ltr43_->writePort(3, 0, false);
    QTimer::singleShot(1000,this,SLOT(closeBottomCoverByTimer()));
    timerBottomCoverIsStop = false;
}

void Covers::lowVoltage()
{
    ltr43_->writePort(3, 1, false);
}

void Covers::higthVoltage()
{
    ltr43_->writePort(3, 1, true);
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
        emit openSafetyValve();
        emit message(tr("Отсекатель открыт."),Shared::information);
        if (remoteDropEnable)
            emit remoteDropSignal();
    }

    if(valveStateClose && !SafetyValveIsClose){
        SafetyValveIsClose = true;
        SafetyValveIsUndef = false;
        emit closeSafetyValve();
        emit remoteDropCompletedSignal();
        emit message(tr("Отсекатель закрыт."),Shared::information);
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
    coverIsOpen = false;
    safetyValveIsOpen = false;
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

void SampleLock::drop()
{
    static int timesDrop;
    if (dropEnable){
        statusPortLtr43(ltr43_->readPorts());
        if(!coverIsOpen && timesDrop < 5){
            timesDrop++;
            QTimer::singleShot(500,this,SLOT(drop()));
            return;
        }
        if (coverIsOpen && safetyValveIsOpen){
            timesDrop = 0;
            lockOpen();
            QTimer::singleShot(1000,this,SLOT(lockClose()));
        }
    }
}

void SampleLock::lockOpen()
{
    if (dropEnable){
        higthVoltage();
        ltr43_->writePort(3, 3, true);
        QTimer::singleShot(1000,this,SLOT(lowVoltage()));
        lockIsOpen = true;
        emit openLockSignal();
        emit message(tr("Замок открыт."),Shared::information);
    }
}

void SampleLock::lockClose()
{
    ltr43_->writePort(3, 3, false);
    lockIsOpen = false;
    //setDropEnable(false);
    emit message(tr("Замок закрыт."),Shared::information);
    emit closeLockSignal();
}

void SampleLock::lowVoltage()
{
    ltr43_->writePort(3, 1, false);
}

void SampleLock::higthVoltage()
{
    ltr43_->writePort(3, 1, true);
}

void SampleLock::statusPortLtr43(DWORD status)
{
    unsigned char port;

    port= status >> 16;
    unsigned char portInvert = port^0xFF;
    unsigned char topCoverPin = 1 << 5;
    unsigned char bottomCoverPin = 1 << 2;
    unsigned char valveOpenPin = 1 << 4;
    unsigned char remoteOpenLockPin = 1 << 1;

    bool topCoverStateOpen = ((portInvert & topCoverPin)== topCoverPin) ? true : false;
    bool bottomCoverStateOpen = ((portInvert & bottomCoverPin)== bottomCoverPin) ? true : false;
    bool remoteOpenLockState = ((portInvert & remoteOpenLockPin)== remoteOpenLockPin) ? true : false;
    safetyValveIsOpen = ((portInvert & valveOpenPin)== valveOpenPin) ? true : false;
    coverIsOpen = topCoverStateOpen && bottomCoverStateOpen;

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
    dropReady(false),
    timerOpenedCoversIsStopped(true)
{

}

bool DropDevice::init()
{
    if (covers && dropSensor && sampleLock && safetyValve){
        isInited = true;

        connect (safetyValve, SIGNAL(remoteDropSignal()), this, SLOT(drop()));
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
        covers->openTopCover();
        covers->openBottomCover();
        dropReady = true;
        emit message(tr("Автоматический сброс ампулы по сигналу отсекателя."),Shared::information);
    }
}

void DropDevice::dropped()
{
    if (isInited && sampleLockState == SampleLockState::Open){
        covers->closeTopCover();
        covers->closeBottomCover();
        sampleLock->lockClose();
        emit message(tr("Время падения ампулы = %1 мс").arg(timeDrop.elapsed()- delayDrop),Shared::warning);
        emit message(tr("Сброс ампулы выполнен."),Shared::information);
        dropReady = false;
    } else if (isInited && sampleLockState == SampleLockState::Close){
        emit message(tr("Ложное срабатывание датчика пролета"),Shared::warning);
    }
}

void DropDevice::openLockSample()
{
    if (topCoverState == CoverState::Open && bottomCoverState == CoverState::Open){
        QTimer::singleShot(delayDropSensor,dropSensor,SLOT(waitDrop()));
        QTimer::singleShot(delayDrop,sampleLock,SLOT(lockOpen()));
        timeDrop.start();
        timeOpenCovers.start();
        timerOpenedCoversIsStopped = false;
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
    if (!timerOpenedCoversIsStopped){
        timerOpenedCoversIsStopped = true;
        emit message(tr("Время засветки калориметрического блока= %1 мс").arg(timeOpenCovers.elapsed()),Shared::warning);
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
    if (!timerOpenedCoversIsStopped){
        timerOpenedCoversIsStopped = true;
        emit message(tr("Время засветки калориметрического блока= %1 мс").arg(timeOpenCovers.elapsed()),Shared::warning);
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








