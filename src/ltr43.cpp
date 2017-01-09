#include "ltr43.h"
#include <QDebug>
#include <QBitArray>

Ltr43::Ltr43(QObject *parent) :
    QObject(parent)
{
    ltr43 = new TLTR43;
    readPortsTimer = new QTimer(this);

    calibrHeaterTimer = new QTimer(this);
    calibrHeaterTimer->setSingleShot(true);
    calibrHeaterTimer->setTimerType(Qt::VeryCoarseTimer);
    workTimeCalibrHeater = new QElapsedTimer();

    connect (readPortsTimer,SIGNAL(timeout()),this,SLOT(readPorts()));
    connect (calibrHeaterTimer,SIGNAL(timeout()),this,SLOT(turnOffCalibrationHeater()));
}

Ltr43::~Ltr43(){
    DWORD outputWord = 0;
    if(ltr43 && LTR43_IsOpened(ltr43)){
        LTR43_WritePort(ltr43,outputWord);
        LTR43_Close(ltr43);
    }
    delete ltr43;
    delete workTimeCalibrHeater;
}

void Ltr43::initializationLTR43(){

    int err;
    int slotNum = 8;

    err = LTR43_Init(ltr43);
    if(err){
        emit message(tr("Ошибка (модуль LTR43(LTR43_Init). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR43_GetErrorString(err)),Shared::warning);
        return;
    }

    err = LTR43_Open(ltr43,SADDR_DEFAULT,SPORT_DEFAULT,(char *)"",slotNum);
    if(err){
        if(err==LTR_WARNING_MODULE_IN_USE)
            emit message(tr("Предупреждение (модуль LTR43(LTR43_Open)). Код предупреждения:%1 (%2).").arg(err)
                         .arg(LTR43_GetErrorString(err)),Shared::warning);
        else{
            emit message(tr("Ошибка (модуль LTR43(LTR43_Open)). Код ошибки:%1 (%2).").arg(err)
                         .arg(LTR43_GetErrorString(err)),Shared::warning);
            return;
        }
    }

    emit message("",Shared::empty);
    emit message(tr("Модуль: %1").arg(ltr43->ModuleInfo.Name),Shared::information);
    emit message(tr("Серийный номер:%1").arg(ltr43->ModuleInfo.Serial),Shared::information);
    emit message(tr("Версия прошивки AVR:%1").arg(ltr43->ModuleInfo.FirmwareVersion),Shared::information);
    emit message(tr("Дата создания прошивки AVR:%1").arg(ltr43->ModuleInfo.FirmwareDate),Shared::information);

    ltr43->IO_Ports.Port1 = 0;
    ltr43->IO_Ports.Port2 = 0;
    ltr43->IO_Ports.Port3 = 0;
    ltr43->IO_Ports.Port4 = 1;
    ltr43->Marks.StartMark_Mode = 0;

    err = LTR43_Config(ltr43);
    if(err){
        emit message(tr("Ошибка (модуль LTR43(LTR43_Config)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR43_GetErrorString(err)),Shared::warning);
        return;
    }

    readPortsTimer->start(200);
}

void Ltr43::writePort (int port, int pins, bool value)
{

    static DWORD outputWord;
    if (value){
        outputWord |= pins << port*8;
    } else {
        DWORD mask = 0xFFFFFFFF;
        mask ^= pins << port*8;
        outputWord &= mask;
    }

    int err=LTR43_WritePort(ltr43,outputWord);
    if(err){
        emit message(tr("Ошибка (модуль LTR43(LTR43_WritePort)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR43_GetErrorString(err)),Shared::warning);
    }
}

void Ltr43::turnOnCalibrationHeater()
{
    writePort (3, 16, true);
    workTimeCalibrHeater->start();
}

void Ltr43::turnOnCalibrationHeaterTimer(int duration)
{
    turnOnCalibrationHeater();
    calibrHeaterTimer->start(duration * 1000);
}

void Ltr43::turnOffCalibrationHeater()
{
    calibrHeaterTimer->stop();
    writePort (3, 16, false);

    message(QString(tr("Калибровочный нагреватель выключен. Время работы %1 сек"))
            .arg(workTimeCalibrHeater->elapsed()/1000.0),Shared::information);
    emit calibrationHeaterOff(workTimeCalibrHeater->elapsed());
}

DWORD Ltr43::readPorts ()
{
    DWORD dataPort = 0;

    int err=LTR43_ReadPort(ltr43,&dataPort);
    if(err){
        emit message(tr("Ошибка (модуль LTR43(LTR43_ReadPort)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR43_GetErrorString(err)),Shared::warning);
    }

    emit readPortsSignal(dataPort);
    lastStatusPort = dataPort;
    return dataPort;
}

DWORD Ltr43::getLastStatusPort()
{
    return lastStatusPort;
}
