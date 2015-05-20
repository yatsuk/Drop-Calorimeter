#include "ltr43.h"
#include <QDebug>
#include <QBitArray>

Ltr43::Ltr43(QObject *parent) :
    QObject(parent)
{
    ltr43 = new TLTR43;
    readPortsTimer = new QTimer(this);
    connect (readPortsTimer,SIGNAL(timeout()),this,SLOT(readPorts()));
}

Ltr43::~Ltr43(){
    DWORD outputWord = 0;
    if(ltr43 && LTR43_IsOpened(ltr43)){
        LTR43_WritePort(ltr43,outputWord);
        LTR43_Close(ltr43);
    }
    delete ltr43;
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

    readPortsTimer->start(300);
}

void Ltr43::writePort (int port, int pin, bool value)
{
    static DWORD outputWord;
    if (value){
        outputWord |= (1 << (port*8 + pin));
    } else {
        DWORD mask = 0xFFFFFFFF;
        mask ^= (1 << (port*8 + pin));
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
    writePort (3, 4, true);
}

void Ltr43::turnOnCalibrationHeaterTimer(int duration)
{
    turnOnCalibrationHeater();
    QTimer::singleShot(duration*1000,this,SLOT(turnOffCalibrationHeater()));
}

void Ltr43::turnOffCalibrationHeater()
{
    writePort (3, 4, false);
    emit calibrationHeaterOff();
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

void Ltr43::testSlotTimer(){
    /*DWORD inputWord;
    int err;

    unsigned char port1;
    unsigned char port2;
    unsigned char port3;
    unsigned char port4;

    err=LTR43_ReadPort(ltr43,&inputWord);
    if(err){
        emit message(tr("Ошибка (модуль LTR43). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR43_GetErrorString(err)),Shared::warning);
        return;
    }

    port1= inputWord;
    port2= inputWord >> 8;
    port3= inputWord >> 16;
    port4= inputWord >> 24;

    if (port2==254)
        emit lowerPressure();
    else if (port2==255)
        emit normalPressure();
    else if (port2==253)
        emit upperPressure();*/

}
