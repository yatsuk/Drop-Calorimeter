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
    ltr114->FreqDivider=1600;
    ltr114->LChQnt = 5;
    ltr114->LChTbl[0]=LTR114_CreateLChannel(LTR114_MEASMODE_U,0,LTR114_URANGE_04);
    ltr114->LChTbl[1]=LTR114_CreateLChannel(LTR114_MEASMODE_U,1,LTR114_URANGE_04);
    ltr114->LChTbl[2]=LTR114_CreateLChannel(LTR114_MEASMODE_U,2,LTR114_URANGE_04);
    ltr114->LChTbl[3]=LTR114_CreateLChannel(LTR114_MEASMODE_U,3,LTR114_URANGE_04);
    ltr114->LChTbl[4]=LTR114_CreateLChannel(LTR114_MEASMODE_U,6,LTR114_URANGE_04);
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

    qDebug() << LTR114_FREQ((*ltr114));

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

            TerconData dataCh2; // термопара основного нагревателя
            dataCh2.unit=tr("В");
            dataCh2.id = "{ff98f69d-11cd-4553-8261-c338fe0e4a29}";
            dataCh2.value=procData[1];
            emit dataSend(dataCh2);

            //измерения термопар охранных нагреватели должны отправляться  в обработку после измерений термопар основного нагреватля
            TerconData dataCh1; // термопара верхнего охр нагревателя
            dataCh1.unit=tr("В");
            dataCh1.id = "{89349bc0-7eab-49db-b86c-047bac3915ef}";
            dataCh1.value=procData[0];

            emit dataSend(dataCh1);

            TerconData dataCh3; // термопара нижнего охр нагревателя
            dataCh3.unit=tr("В");
            dataCh3.id = "{f41b67da-dc68-4ee9-8d3c-b74f22368a05}";
            dataCh3.value=procData[2];
            emit dataSend(dataCh3);

            TerconData dataCh4;
            dataCh4.unit=tr("В");
            dataCh4.id = "{1bab9ffd-68df-459a-b21b-de569a211232}";
            dataCh4.value=procData[3];
            emit dataSend(dataCh4);

            TerconData dataCh5;
            dataCh5.unit=tr("В");
            dataCh5.id = "{ac8d9b87-39d1-4ab2-90a9-fb88018e3306}";
            dataCh5.value= procData[4];
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
