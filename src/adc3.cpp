#include "adc.h"
#include <math.h>
#include <QDebug>

ADC::ADC(QObject *parent) :
    QObject(parent)
{
    ltr27 = new TLTR27;
    recvTimer = new QTimer(this);

    roomTemperature = 17;
    offsetVolt = convertTemperature2VoltTypeA1(roomTemperature);

    connect(recvTimer,SIGNAL(timeout()),this,SLOT(recvData()));
}


ADC::~ADC(){
    LTR27_Close(ltr27);
    delete ltr27;
}

void ADC::setTime(QTime *time){
    m_Time = time;
}

void ADC::initializationLTR27(){
    int err;
    int slotNum = 1;

    err = LTR27_Init(ltr27);
    if(err){
        emit message(tr("Ошибка (модуль LTR27). Код ошибки:%1 (%2).").arg(err)
                     .arg(QString::fromLocal8Bit(LTR27_GetErrorString(err))),Shared::warning);
        return;
    }

    err = LTR27_Open(ltr27,SADDR_DEFAULT,SPORT_DEFAULT,"",slotNum);
    if(err){
        if(err==LTR_WARNING_MODULE_IN_USE)
            emit message(tr("Предупреждение (модуль LTR27). Код предупреждения:%1 (%2).").arg(err)
                         .arg(QString::fromLocal8Bit(LTR27_GetErrorString(err))),Shared::warning);
        else{
            emit message(tr("Ошибка (модуль LTR27). Код ошибки:%1 (%2).").arg(err)
                         .arg(QString::fromLocal8Bit(LTR27_GetErrorString(err))),Shared::warning);
            return;
        }
    }

    err = LTR27_GetConfig(ltr27);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27). Код ошибки:%1 (%2).").arg(err)
                     .arg(QString::fromLocal8Bit(LTR27_GetErrorString(err))),Shared::warning);
        return;
    }

    err = LTR27_GetDescription(ltr27,LTR27_ALL_DESCRIPTION);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27). Код ошибки:%1 (%2).").arg(err)
                     .arg(QString::fromLocal8Bit(LTR27_GetErrorString(err))),Shared::warning);
        return;
    }

    emit message(tr("модуль LTR27:"),Shared::information);

    for (int i =0; i<LTR27_MEZZANINE_NUMBER;++i){
        QString strMsg(ltr27->Mezzanine[i].Name);
        if (strMsg!="EMPTY"){
            emit message(tr("Имя субмодуля: H-27%1").arg(strMsg),Shared::information);
            emit message(tr("Единица измерения: %1").arg(QString::fromLocal8Bit(ltr27->Mezzanine[i].Unit)),Shared::information);
        }
    }

    ltr27->FrequencyDivisor=255;

    for (int i =0; i<LTR27_MEZZANINE_NUMBER;++i)
        for (int j = 0; j <4;++j){
            ltr27->Mezzanine[i].CalibrCoeff[j]= ltr27->ModuleInfo.Mezzanine[i].Calibration[j];
        }

    err = LTR27_SetConfig(ltr27);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27). Код ошибки:%1 (%2).").arg(err)
                     .arg(QString::fromLocal8Bit(LTR27_GetErrorString(err))),Shared::warning);
        return;
    }
}

double ADC::moveAverage(const QVector<double> &valueArray){
    double averageValue=0;
    int averageSize = valueArray.size();
    for(int i =0; i<averageSize;++i)
        averageValue+=valueArray.at(i);
    averageValue/=averageSize;
    return averageValue;
}

long double ADC::convertVolt2TemperatureTypeA1(double value){
    long double temperature=0.9643027;
    temperature+=79.495086*value;
    temperature-=4.9990310*pow(value,2);
    temperature+=0.6341776*pow(value,3);
    temperature-=4.7440967e-2*pow(value,4);
    temperature+=2.1811337e-3*pow(value,5);
    temperature-=5.8324228e-5*pow(value,6);
    temperature+=8.2433725e-7*pow(value,7);
    temperature-=4.5928480e-9*pow(value,8);
    return temperature;
}

long double ADC::convertTemperature2VoltTypeA1(double temperature){
    long double volt =7.1564735E-04;
    volt+=1.1951905E-02*temperature;
    volt+=1.6672625E-05*pow(temperature,2);
    volt-=2.8287807E-08*pow(temperature,3);
    volt+=2.8397839E-11*pow(temperature,4);
    volt-=1.8505007E-14*pow(temperature,5);
    volt+=7.3632123E-18*pow(temperature,6);
    volt-=1.6148878E-21*pow(temperature,7);
    volt+=1.4901679E-25*pow(temperature,8);
    return volt;
}

void ADC::recvData(){
    const int timeout = 50;

    DWORD size  = LTR27_Recv(ltr27,buf,NULL,SamplesCount,timeout);
    if (size>0){
        int err=LTR27_ProcessData(ltr27,buf,dataArray,&size,1,1);
        if(err!=LTR_OK){
            emit message(tr("Ошибка (модуль LTR27). Код ошибки:%1 (%2).").arg(err)
                         .arg(QString::fromLocal8Bit(LTR27_GetErrorString(err))),Shared::warning);
            return;
        }
        for(unsigned int i = 0;i < size;++i){
            if (dataArray[i]!=0.0){
                if (i==0){
                    TerconData data;
                    data.channel=0;
                    data.deviceNumber=4;
                    data.unit='T';

                    sourcesDataCh1.push_back((dataArray[i]+offsetZero)*scale);
                    if(sourcesDataCh1.size()>averageCount)
                        sourcesDataCh1.pop_front();

                    data.value=convertVolt2TemperatureTypeA1( moveAverage(sourcesDataCh1)+offsetVolt);

                    dataSend(data);

                }
                else if(i==1){
                    TerconData data;
                    data.channel=1;
                    data.deviceNumber=4;
                    data.unit='T';

                    sourcesDataCh2.push_back((dataArray[i]+offsetZero)*scale);
                    if(sourcesDataCh2.size()>averageCount)
                        sourcesDataCh2.pop_front();

                    data.value=convertVolt2TemperatureTypeA1( moveAverage(sourcesDataCh2)+convertTemperature2VoltTypeA1(25.0));
                    dataSend(data);
                }
            }
        }
    }
}

void ADC::startADC(){
    int err = LTR27_ADCStart(ltr27);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27). Код ошибки:%1 (%2).").arg(err)
                     .arg(QString::fromLocal8Bit(LTR27_GetErrorString(err))),Shared::warning);
        return;
    }

    recvTimer->start(100);
}

void ADC::stopADC(){
    int err = LTR27_ADCStop(ltr27);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27). Код ошибки:%1 (%2).").arg(err)
                     .arg(QString::fromLocal8Bit(LTR27_GetErrorString(err))),Shared::warning);
        return;
    }
    recvTimer->stop();
}
