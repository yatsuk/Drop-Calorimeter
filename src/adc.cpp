#include "adc.h"
#include <math.h>
#include <QDebug>

ADC::ADC(QObject *parent) :
    QObject(parent)
{
    ltr27 = new TLTR27;
    recvTimer = new QTimer(this);

    parameters_.roomTemperature = 17;
    parameters_.averageCount = 10;
    parameters_.filter = false;
    parameters_.thermocoupleType = "A1";

    if (parameters_.thermocoupleType == "A1")
        offsetVoltRoomTemperature = convertTemperature2VoltTypeA1(parameters_.roomTemperature);
    else if (parameters_.thermocoupleType == "K")
        offsetVoltRoomTemperature = convertTemperature2VoltTypeK(parameters_.roomTemperature);

    connect(recvTimer,SIGNAL(timeout()),this,SLOT(recvData()));
}


ADC::~ADC(){
    if(ltr27 && LTR27_IsOpened(ltr27))
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
        emit message(tr("Ошибка (модуль LTR27(LTR27_Init)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR27_GetErrorString(err)),Shared::warning);
        return;
    }

    err = LTR27_Open(ltr27,SADDR_DEFAULT,SPORT_DEFAULT,"",slotNum);
    if(err){
        if(err==LTR_WARNING_MODULE_IN_USE)
            emit message(tr("Предупреждение (модуль LTR27(LTR27_Open)). Код предупреждения:%1 (%2).").arg(err)
                         .arg(LTR27_GetErrorString(err)),Shared::warning);
        else{
            emit message(tr("Ошибка (модуль LTR27(LTR27_Open)). Код ошибки:%1 (%2).").arg(err)
                         .arg(LTR27_GetErrorString(err)),Shared::warning);
            return;
        }
    }

    err = LTR27_GetConfig(ltr27);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27(LTR27_GetConfig)). Код ошибки:%1 (%2).").arg(err)
                     .arg(err),Shared::warning);
        return;
    }

    err = LTR27_GetDescription(ltr27,LTR27_ALL_DESCRIPTION);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27(LTR27_GetDescription)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR27_GetErrorString(err)),Shared::warning);
        return;
    }

    emit message("",Shared::empty);
    emit message(tr("Модуль LTR27:"),Shared::information);

    for (int i =0; i<LTR27_MEZZANINE_NUMBER;++i){
        QString strMsg(ltr27->Mezzanine[i].Name);
        if (strMsg!="EMPTY"){
            emit message(tr("Субмодуль установлен в слот №:%1").arg(i+1),Shared::information);
            emit message(tr("Имя субмодуля: H-27%1").arg(strMsg),Shared::information);
            emit message(tr("Единица измерения: %1").arg(ltr27->Mezzanine[i].Unit),Shared::information);
        }
    }

    ltr27->FrequencyDivisor=255;

    for (int i =0; i<LTR27_MEZZANINE_NUMBER;++i)
        for (int j = 0; j <4;++j){
            ltr27->Mezzanine[i].CalibrCoeff[j]= ltr27->ModuleInfo.Mezzanine[i].Calibration[j];
        }

    err = LTR27_SetConfig(ltr27);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27(LTR27_SetConfig)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR27_GetErrorString(err)),Shared::warning);
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

long double ADC::convertVolt2TemperatureTypeK(double value){
    long double temperature=-0.12;
    temperature+=25.64975235588457*value;
    temperature-=0.8055076713568498*pow(value,2);
    temperature+=0.1956119653603405*pow(value,3);
    temperature-=2.2808773974444e-2*pow(value,4);
    temperature+=1.493718627979179e-3*pow(value,5);
    temperature-=5.965771945226433e-5*pow(value,6);
    temperature+=1.489119820403751e-6*pow(value,7);
    temperature-=2.269922703788692e-8*pow(value,8);
    temperature+=1.933261352900763e-10*pow(value,9);
    temperature-=7.049691433910994e-13*pow(value,10);
    return temperature;
}

long double ADC::convertTemperature2VoltTypeK(double temperature){
    /*from 0 grad C to 35 grad C*/
    long double volt =0;
    volt+=0.0395*temperature;
    volt+=2E-05*pow(temperature,2);
    return volt;
}

void ADC::recvData(){
    const int timeout = 10;

    DWORD size  = LTR27_Recv(ltr27,buf,NULL,SamplesCount,timeout);
    if (size>0){
        int err=LTR27_ProcessData(ltr27,buf,dataArray,&size,1,1);
        if(err!=LTR_OK){
            emit message(tr("Ошибка (модуль LTR27(LTR27_ProcessData)). Код ошибки:%1 (%2).").arg(err)
                         .arg(LTR27_GetErrorString(err)),Shared::warning);
            return;
        }
        for(unsigned int i = 0;i < size;++i){

            if (i==2){
                static double prevValue;
                TerconData data;
                data.channel=0;
                data.deviceNumber=4;
                data.unit='T';

                double value = dataArray[i];

                if (!firstValue){
                    firstValue = true;
                    data.value = value;
                }else{
                    if(parameters_.filter)
                        data.value= (qAbs(value-prevValue)>maxDeltaCurrentPrevValue)?prevValue:value;
                    else
                        data.value = value;
                }

                prevValue=data.value;

                sourcesDataCh1.push_back(data.value);
                if(sourcesDataCh1.size()>parameters_.averageCount)
                    sourcesDataCh1.pop_front();

                if(parameters_.averageCount>0){
                    if (parameters_.thermocoupleType == "A1")
                        data.value=convertVolt2TemperatureTypeA1( moveAverage(sourcesDataCh1)+offsetVoltRoomTemperature);
                    else if (parameters_.thermocoupleType == "K")
                        data.value=convertVolt2TemperatureTypeK( moveAverage(sourcesDataCh1)+offsetVoltRoomTemperature);
                }else{
                    if (parameters_.thermocoupleType == "A1")
                        data.value=convertVolt2TemperatureTypeA1( data.value+offsetVoltRoomTemperature);
                    else if (parameters_.thermocoupleType == "K")
                        data.value=convertVolt2TemperatureTypeK( data.value+offsetVoltRoomTemperature);
                }

                dataSend(data);

            }
            else if(i==1){
                TerconData data;
                data.channel=1;
                data.deviceNumber=4;
                data.unit='T';

                sourcesDataCh2.push_back((dataArray[i]+offsetZero)*scale);
                if(sourcesDataCh2.size()>parameters_.averageCount)
                    sourcesDataCh2.pop_front();

                data.value=convertVolt2TemperatureTypeA1( moveAverage(sourcesDataCh2)+convertTemperature2VoltTypeA1(25.0));
                dataSend(data);
            }
        }
    }
}

void ADC::startADC(){
    int err = LTR27_ADCStart(ltr27);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27(LTR27_ADCStart)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR27_GetErrorString(err)),Shared::warning);
        return;
    }

    firstValue = false;
    recvTimer->start(50);
}

void ADC::stopADC(){
    int err = LTR27_ADCStop(ltr27);
    if(err!=LTR_OK){
        emit message(tr("Ошибка (модуль LTR27(LTR27_ADCStop)). Код ошибки:%1 (%2).").arg(err)
                     .arg(LTR27_GetErrorString(err)),Shared::warning);
        return;
    }
    recvTimer->stop();
}

/*void ADC::setParameters(AdcParameters parameters){
    parameters_= parameters;
    if (parameters_.thermocoupleType == "A1")
        offsetVoltRoomTemperature = convertTemperature2VoltTypeA1(parameters_.roomTemperature);
    else if (parameters_.thermocoupleType == "K")
        offsetVoltRoomTemperature = convertTemperature2VoltTypeK(parameters_.roomTemperature);

}

AdcParameters ADC::parameters(){
    return parameters_;
}*/
