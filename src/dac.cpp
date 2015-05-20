#include "dac.h"
#include <QDebug>

DAC::DAC(QObject *parent) :
    QObject(parent)
{  
    isStarted = false;
    ltr34 = 0;
    ltr = 0;
}

void DAC::initializationLTR34(){
    ltr34 = new TLTR34;
    uint slotNum = 7;
    code = new double[sizeArray];
    arrayToSend = new DWORD[sizeArray];
    for (unsigned int i = 0;i<sizeArray;++i){
        code[i]=0;
        arrayToSend[i]=0;
    }

    ltr34Error = LTR34_Init(ltr34);
    if(ltr34Error){
        emit message(tr("Ошибка (модуль LTR34(LTR34_Init)). Код ошибки:%1 (%2).").arg(ltr34Error)
                     .arg(LTR34_GetErrorString(ltr34Error)),Shared::warning);
    }
    else{
        ltr34Error= LTR34_Open(ltr34,SADDR_DEFAULT,SPORT_DEFAULT,"",slotNum);
        if(ltr34Error){
            emit message(tr("Ошибка (модуль LTR34(LTR34_Open)). Код ошибки:%1 (%2).").arg(ltr34Error)
                         .arg(ltr34Error),Shared::warning);
        }
        else{
            emit message("",Shared::empty);
            emit message(tr("Модуль LTR34:"),Shared::information);
            emit message(tr("Имя модуля:%1").arg(ltr34->ModuleInfo.Name),Shared::information);
            emit message(tr("Серийный номер:%1").arg(ltr34->ModuleInfo.Serial),Shared::information);
            emit message(tr("Максимальное число каналов:%1").arg(ltr34->ModuleInfo.MaxChannelQnt),Shared::information);
            emit message(tr("версия ПЛИС:%1").arg(ltr34->ModuleInfo.FPGA_Version),Shared::information);
            emit message(tr("Версия калибровки:%1").arg(ltr34->ModuleInfo.CalibrVersion),Shared::information);

            ltr34->ChannelQnt = 4;
            ltr34->LChTbl[0]=LTR34_CreateLChannel(1,false);
            ltr34->LChTbl[1]=LTR34_CreateLChannel(2,false);
            ltr34->LChTbl[2]=LTR34_CreateLChannel(3,false);
            ltr34->LChTbl[3]=LTR34_CreateLChannel(4,false);
            ltr34->FrequencyDivisor = 0;
            ltr34->UseClb =true;
            ltr34->AcknowledgeType = false;

            ltr34Error = LTR34_Reset(ltr34);
            if(ltr34Error){
                emit message(tr("Ошибка (модуль LTR34(LTR34_Reset)). Код ошибки:%1 (%2).").arg(ltr34Error)
                             .arg(LTR34_GetErrorString(ltr34Error)),Shared::warning);
            }
            else{
                ltr34Error = LTR34_Config(ltr34);
                if(ltr34Error){
                    emit message(tr("Ошибка (модуль LTR34(LTR34_Config)). Код ошибки:%1 (%2).").arg(ltr34Error)
                                 .arg(LTR34_GetErrorString(ltr34Error)),Shared::warning);
                }
            }
        }
    }
}

void DAC::initializationLTR(){
    ltr = new TLTR;
    const int maxModules = 16;
    ltrError = LTR_Init(ltr);
    if (ltrError){
        emit message(tr("Ошибка (крейт LTR(LTR_Init)). Код ошибки:%1 (%2).").arg(ltrError)
                     .arg(LTR_GetErrorString(ltrError)),Shared::warning);
    }
    else{
        ltrError = LTR_Open(ltr);
        if (ltrError){
            emit message(tr("Ошибка (крейт LTR(LTR_Open)). Код ошибки:%1 (%2).").arg(ltrError)
                         .arg(LTR_GetErrorString(ltrError)),Shared::warning);
        }
        else{
            WORD modules [maxModules];

            ltrError = LTR_GetCrateModules(ltr,modules);
            if (ltrError==LTR_OK){
                emit message(tr("Поиск доступных модулей LTR:"),Shared::information);
                for (int i=0;i < maxModules;++i)
                    if((modules[i] & 0xFF)!=0)
                        emit message(tr("В слоте №%1 обнаружен модуль LTR%2").arg(i+1).arg(modules[i] & 0xFF),Shared::information);
            }
        }
    }
}

void DAC::setValueDAC(double value, int channelNumber){
    code[channelNumber]=value/10.0;
    ltr34Error = LTR34_ProcessData(ltr34,code,arrayToSend,sizeArray,true);
    if(ltr34Error)
        emit message(tr("Ошибка (модуль LTR34(LTR34_ProcessData)). Код ошибки:%1 (%2).").arg(ltr34Error)
                     .arg(LTR34_GetErrorString(ltr34Error)),Shared::warning);
    unsigned int sizeArraySend = LTR34_Send(ltr34,arrayToSend,sizeArray,500);

    if(sizeArraySend!=sizeArray)
        emit message(tr("Ошибка (модуль LTR34(LTR34_Send)). Код ошибки:%1 (%2).").arg(ltr34Error)
                     .arg(LTR34_GetErrorString(ltr34Error)),Shared::warning);
}

void DAC::startDAC(){
    if(!isStarted){
        ltr34Error = LTR34_DACStart(ltr34);
        if(ltr34Error)
            emit message(tr("Ошибка (модуль LTR34(LTR34_DACStart)). Код ошибки:%1 (%2).").arg(ltr34Error)
                         .arg(LTR34_GetErrorString(ltr34Error)),Shared::warning);
        else
            isStarted = true;
    }
}

void DAC::stopDAC(int channelNumber){
    setValueDAC(0,channelNumber);
}


DAC::~DAC()
{
    if (isStarted){
        ltr34Error = LTR34_DACStop(ltr34);
        if(ltr34Error)
            emit message(tr("Ошибка (модуль LTR34(LTR34_DACStop)). Код ошибки:%1 (%2).").arg(ltr34Error)
                         .arg(LTR34_GetErrorString(ltr34Error)),Shared::warning);
    }

    if(ltr && LTR_IsOpened(ltr))
        LTR_Close(ltr);
    if(ltr34 && LTR34_IsOpened(ltr34))
        LTR34_Close(ltr34);

    delete ltr;
    delete ltr34;
    delete [] code;
    delete [] arrayToSend;
}
