#include "ltr114.h"
#include <QTimer>
#include <math.h>
#include <QDebug>


Ltr114::Ltr114(QObject *parent) :
    QObject(parent)
{

    ltr114Worker = new Ltr114Worker;
    workThread = new QThread;
    ltr114Worker->moveToThread(workThread);

    connect(ltr114Worker,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)),Qt::BlockingQueuedConnection);
    connect(ltr114Worker,SIGNAL(dataSend(TerconData)),
            this,SIGNAL(dataSend(TerconData)),Qt::BlockingQueuedConnection);
    //connect(this,Ltr114::)
}

Ltr114::~Ltr114()
{
    if (workThread->isRunning()){
        workThread->quit();
        if(!workThread->wait(5000)){
            workThread->terminate();
            qDebug() << "terminate ltr114";
        }
    }

    delete ltr114Worker;
    delete workThread;
}

void Ltr114::initialization()
{
    if (!workThread->isRunning()){
        connect(workThread,SIGNAL(started()),ltr114Worker,SLOT(initialization()));
        connect(ltr114Worker,SIGNAL(endInitialization()),workThread,SLOT(quit()));
        connect(ltr114Worker,SIGNAL(endCalibration()),workThread,SLOT(quit()));
        workThread->start();
    }else {
        QTimer::singleShot(10000,this,SLOT(initialization()));
    }
}

void Ltr114::start()
{
    if (!workThread->isRunning()){
        workThread->disconnect(ltr114Worker);
        connect(workThread,SIGNAL(started()),ltr114Worker,SLOT(start()));
        connect(ltr114Worker,SIGNAL(stopAck()),workThread,SLOT(quit()));
        workThread->start();
    }else {
        QTimer::singleShot(10000,this,SLOT(start()));
    }
}

bool Ltr114::stop()
{

    ltr114Worker->stopWorker();
    return !workThread->isRunning();
}

Ltr114Worker::Ltr114Worker(QObject *parent) :
    QObject(parent)
{
    ltr114 = new TLTR114;
    isConfugurationOk = false;
    isStart=false;
    isWaitTimeout = false;
    isStopAck = false;
    recvData = 0;
    procData = 0;

    parameters_.roomTemperature = 30;
    parameters_.averageCount = 1;
    parameters_.filter = false;
    parameters_.thermocoupleType = "A1";

    if (parameters_.thermocoupleType == "A1")
        offsetVoltRoomTemperature = convertTemperature2VoltTypeA1(parameters_.roomTemperature);
    else if (parameters_.thermocoupleType == "K")
        offsetVoltRoomTemperature = convertTemperature2VoltTypeK(parameters_.roomTemperature);
}

Ltr114Worker::~Ltr114Worker()
{
    LTR114_Close(ltr114);
    delete ltr114;
}

bool Ltr114Worker::initialization()
{
    int err;
    int slotNum = 6;

    err = LTR114_Init(ltr114);
    if(err){
        emit message(tr("Ошибка (модуль LTR114(LTR114_Init)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR114_GetErrorString(err)),Shared::warning);
        emit endInitialization();
        return false;
    }

    err = LTR114_Open(ltr114,SADDR_DEFAULT,SPORT_DEFAULT,"",slotNum);
    if(err){
        if(err==LTR_WARNING_MODULE_IN_USE)
            emit message(tr("Предупреждение (модуль LTR114). Код предупреждения:%1 (%2).").arg(err)
                         .arg(LTR114_GetErrorString(err)),Shared::warning);
        else{
            emit message(tr("Ошибка (модуль LTR114(LTR114_Open)). Код ошибки:%1 (%2).").arg(err)
                         .arg(LTR114_GetErrorString(err)),Shared::warning);
            emit endInitialization();
            return false;
        }
    }

    err = LTR114_GetConfig(ltr114);
    if(err){
        emit message(tr("Ошибка (модуль LTR114(LTR114_GetConfig)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR114_GetErrorString(err)),Shared::warning);
        emit endInitialization();
        return false;
    }

    emit message("",Shared::empty);
    emit message(tr("Модуль: %1").arg(ltr114->ModuleInfo.Name),Shared::information);
    emit message(tr("Серийный номер:%1").arg(ltr114->ModuleInfo.Serial),Shared::information);
    emit message(tr("Версия прошивки MCU:%1.%2").arg(ltr114->ModuleInfo.VerMCU&0xFF00).arg(ltr114->ModuleInfo.VerMCU&0xFF),Shared::information);
    emit message(tr("Версия прошивки ПЛИС:%1").arg(ltr114->ModuleInfo.VerPLD),Shared::information);
    emit message(tr("Дата создания ПО:%1").arg(ltr114->ModuleInfo.Date),Shared::information);
    emit message(tr("Модуль: %1. Инициализация завершена.").arg(ltr114->ModuleInfo.Name),Shared::information);
    emit endInitialization();
    return configure();
}

bool Ltr114Worker::configure()
{
    ltr114->FreqDivider=2000;
    ltr114->LChQnt = 5;
    ltr114->LChTbl[0]=LTR114_CreateLChannel(LTR114_MEASMODE_U,0,LTR114_URANGE_04);
    ltr114->LChTbl[1]=LTR114_CreateLChannel(LTR114_MEASMODE_U,1,LTR114_URANGE_04);
    ltr114->LChTbl[2]=LTR114_CreateLChannel(LTR114_MEASMODE_U,2,LTR114_URANGE_04);
    ltr114->LChTbl[3]=LTR114_CreateLChannel(LTR114_MEASMODE_U,3,LTR114_URANGE_04);
    ltr114->LChTbl[4]=LTR114_CreateLChannel(LTR114_MEASMODE_U,4,LTR114_URANGE_04);
    ltr114->SyncMode = LTR114_SYNCMODE_INTERNAL;
    ltr114->Interval = 1;

    int err = LTR114_SetADC(ltr114);
    if(err){
        emit message(tr("Ошибка (модуль LTR114 (LTR114_SetADC)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR114_GetErrorString(err)),Shared::warning);
        emit endCalibration();
        return false;
    }

    emit message(tr("Модуль: %1. Конфигурация завершена.").arg(ltr114->ModuleInfo.Name),Shared::information);
    emit endConfiguration();

    err = LTR114_Calibrate(ltr114);
    if(err){
        emit message(tr("Ошибка (модуль LTR114 (LTR114_Calibrate)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR114_GetErrorString(err)),Shared::warning);
        emit endCalibration();
        return false;
    }

    isConfugurationOk = true;


    emit message(tr("Модуль: %1. Калибровка завершена.").arg(ltr114->ModuleInfo.Name),Shared::information);
    emit endCalibration();

    return true;
}

bool Ltr114Worker::start()
{
    emit message(tr("Модуль: %1. Опрос начат.").arg(ltr114->ModuleInfo.Name),Shared::information);
    int err = LTR114_Start(ltr114);
    if (err){
        emit message(tr("Ошибка (модуль LTR114 (LTR114_Start)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR114_GetErrorString(err)),Shared::warning);
        stop();
        return false;
    }

    isStart=true;
    isStopAck = false;

    recvData = new DWORD [ltr114->FrameLength];
    procData = new double [ltr114->LChQnt];

    while(true){

        sizeBuf = ltr114->FrameLength;

        int recvByteCount = LTR114_Recv(ltr114, recvData, NULL, sizeBuf, 5000);
        if (recvByteCount==0){
            emit message(tr("Ошибка (модуль LTR114 (LTR114_Recv)). Не приняты данные"),Shared::warning);
            stop();
            return false;
        }else if (recvByteCount>0){
            if (recvByteCount<sizeBuf){
                emit message(tr("Ошибка (модуль LTR114 (LTR114_Recv)). Приняты не все данные"),Shared::warning);
                sizeBuf = recvByteCount;
            }
            err = LTR114_ProcessData(ltr114, recvData, procData, &sizeBuf, LTR114_CORRECTION_MODE_AUTO, LTR114_PROCF_VALUE);
            if(err){
                emit message(tr("Ошибка (модуль LTR114 (LTR114_ProcessData)). Код ошибки:%1 (%2).").arg(err)
                             .arg(LTR114_GetErrorString(err)),Shared::warning);
                stop();
                return false;
            }

            TerconData dataCh2; // термопара основного нагреватель
            dataCh2.channel=2;
            dataCh2.deviceNumber=5;
            dataCh2.unit='T';
            dataCh2.value=convertVolt2TemperatureTypeA1( procData[1]*(1e+3) +offsetVoltRoomTemperature);

            emit dataSend(dataCh2);

            //измерения термопар охранных нагреватели должны отправлться  в обработку после измерений термопар основного нагреватля
            TerconData dataCh1; // термопара верхнего охр нагреватель
            dataCh1.channel=1;
            dataCh1.deviceNumber=5;
            dataCh1.unit='T';
            dataCh1.value=convertVolt2TemperatureTypeA1( procData[0]*(1e+3) +offsetVoltRoomTemperature);

            emit dataSend(dataCh1);

            TerconData dataCh3; // термопара нижнего охр нагреватель
            dataCh3.channel=3;
            dataCh3.deviceNumber=5;
            dataCh3.unit='T';
            dataCh3.value=convertVolt2TemperatureTypeA1( procData[2]*(1e+3) +offsetVoltRoomTemperature);

            emit dataSend(dataCh3);

            TerconData dataCh4;
            dataCh4.channel=4;
            dataCh4.deviceNumber=5;
            dataCh4.unit='T';
            dataCh4.value=convertVolt2TemperatureTypeA1( procData[3]*(1e+3) +offsetVoltRoomTemperature);

            emit dataSend(dataCh4);

            TerconData dataCh5;
            dataCh5.channel=5;
            dataCh5.deviceNumber=5;
            dataCh5.unit='U';
            dataCh5.value= procData[4]*(1e+3);

            emit dataSend(dataCh5);



        }else{
            emit message(tr("Ошибка (модуль LTR114 (LTR114_Recv)). Код ошибки:%1 (%2).").arg(err)
                         .arg(LTR114_GetErrorString(err)),Shared::warning);
            stop();
            return false;
        }

        if(isStopAck)
            break;
    }

    stop();
    return true;
}

bool Ltr114Worker::stop()
{
    delete [] recvData;
    delete [] procData;

    emit message(tr("Модуль: %1. Опрос закончен.").arg(ltr114->ModuleInfo.Name),Shared::information);
    isWaitTimeout = false;
    if (!isStart)
        return true;

    int err = LTR114_Stop(ltr114);
    if (err){
        emit message(tr("Ошибка (модуль LTR114 (LTR114_Stop)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR114_GetErrorString(err)),Shared::warning);
        emit stopAck();
        return false;
    }

    emit stopAck();
    return true;
}

void Ltr114Worker::stopWorker()
{
    isStopAck = true;
}

long double Ltr114Worker::convertVolt2TemperatureTypeA1(double value){
    long double temperature=0.9643027
            + 79.495086*value
            - 4.9990310*pow(value,2)
            + 0.6341776*pow(value,3)
            - 4.7440967e-2*pow(value,4)
            + 2.1811337e-3*pow(value,5)
            - 5.8324228e-5*pow(value,6)
            + 8.2433725e-7*pow(value,7)
            - 4.5928480e-9*pow(value,8);
    return temperature;
}

long double Ltr114Worker::convertTemperature2VoltTypeA1(double temperature){
    long double volt =7.1564735E-04
            + 1.1951905E-02*temperature
            + 1.6672625E-05*pow(temperature,2)
            - 2.8287807E-08*pow(temperature,3)
            + 2.8397839E-11*pow(temperature,4)
            - 1.8505007E-14*pow(temperature,5)
            + 7.3632123E-18*pow(temperature,6)
            - 1.6148878E-21*pow(temperature,7)
            + 1.4901679E-25*pow(temperature,8);
    return volt;
}

long double Ltr114Worker::convertVolt2TemperatureTypeK(double value){
    long double temperature = -0.12
            + 25.64975235588457*value
            - 0.8055076713568498*pow(value,2)
            + 0.1956119653603405*pow(value,3)
            - 2.2808773974444e-2*pow(value,4)
            + 1.493718627979179e-3*pow(value,5)
            - 5.965771945226433e-5*pow(value,6)
            + 1.489119820403751e-6*pow(value,7)
            - 2.269922703788692e-8*pow(value,8)
            + 1.933261352900763e-10*pow(value,9)
            - 7.049691433910994e-13*pow(value,10);
    return temperature;
}

long double Ltr114Worker::convertTemperature2VoltTypeK(double temperature){
    /*from 0 grad C to 35 grad C*/
    long double volt =0
            + 0.0395*temperature
            + 2E-05*pow(temperature,2);
    return volt;
}
