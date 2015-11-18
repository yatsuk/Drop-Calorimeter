#include "furnace.h"
#include <math.h>
#include <QDebug>

Furnace * Furnace::g_furnace = 0;

Furnace::Furnace(QObject *parent) :
    QObject(parent)
{

    g_furnace = this;
    settings =  new QSettings(tr("IT"),tr("furnace"),this);

    dataRecorder = new DataRecorder(this);
    dataRecorder->writeFile("Time\tDeviceNumber\tNChannel\tValue\r\n",Shared::dataFile);

    time = new QElapsedTimer();
    time->start();
    workingTimeCalibrHeater = new QElapsedTimer();

    regulatorOfFurnace = new Regulator(this);
    regulatorOfThermostat = new Regulator(this);
    regulatorUpHeater_ = new Regulator(this);
    regulatorDownHeater_ = new Regulator(this);
    covers = new Covers(this);
    safetyValve = new SafetyValve(this);
    sampleLock = new SampleLock(this);

    dac = new DAC(this);
    adc = new ADC(this);
    ltr43 = new Ltr43(this);
    ltr114 = new Ltr114(this);
    m_diagnostic = new Diagnostic(this);
    arduino = new Arduino(this);

    covers->setLTR43(ltr43);
    safetyValve->setLTR43(ltr43);
    sampleLock->setLTR43(ltr43);

    loadSettings();
}

Furnace * Furnace::instance()
{
    return g_furnace;
}

qint64 Furnace::getElapsedTime()
{
    if(time->isValid()){
        return time->elapsed();
    } else {
        return 0;
    }
}

void Furnace::startTercon(){
    tercon->startAck();
}

void Furnace::stopTercon(){
    tercon->stopAck();
}

void Furnace::beginDataRecord(){
    dataRecorder->beginRecord();
}

void Furnace::endDataRecord(){
    dataRecorder->endRecord();
}

void Furnace::turnOnCalibrationHeater(int duration)
{
    if(duration>0)
        ltr43->turnOnCalibrationHeaterTimer(duration);
    else
        ltr43->turnOnCalibrationHeater();

    workingTimeCalibrHeater->restart();
    message(tr("Калибровочный нагреватель включен."),Shared::information);
}

void Furnace::turnOffCalibrationHeater()
{
    ltr43->turnOffCalibrationHeater();
    message(QString(tr("Калибровочный нагреватель выключен. Время работы %1 мсек")).arg(workingTimeCalibrHeater->elapsed()),Shared::information);
}

void Furnace::writeLogMessageToFile(const QString &message)
{
    dataRecorder->writeFile(message,Shared::logFile);
}

void Furnace::setDacValueFurnaceChannel(double value)
{
    dac->setValueDAC(value,0);
}

void Furnace::setDacValueThermostateChannel(double value)
{
    dac->setValueDAC(value,1);
}

void Furnace::setDacValueUpHeaterChannel(double value)
{
    dac->setValueDAC(value,2);
}

void Furnace::setDacValueDownHeaterChannel(double value)
{
    dac->setValueDAC(value,3);
}

void Furnace::writeFurnaceRegulatorLog(const QString &logString)
{
    dataRecorder->writeFile(logString,Shared::regulatorFurnaceFile);
}

void Furnace::writeThermostatRegulatorLog(const QString &logString)
{
    dataRecorder->writeFile(logString,Shared::regulatorThermostatFile);
}

void Furnace::writeUpHeaterRegulatorLog(const QString &logString)
{
    dataRecorder->writeFile(logString,Shared::regulatorUpHeaterFile);
}

void Furnace::writeDownHeaterRegulatorLog(const QString &logString)
{
    dataRecorder->writeFile(logString,Shared::regulatorDownHeaterFile);
}

void Furnace::stopDacFurnaceChannel()
{
    dac->stopDAC(0);
}

void Furnace::stopDacThermostateChannel()
{
    dac->stopDAC(1);
}

void Furnace::stopDacUpHeaterChannel()
{
    dac->stopDAC(2);
}

void Furnace::stopDacDownHeaterChannel()
{
    dac->stopDAC(3);
}

void Furnace::terconDataReceive(TerconData data){

    data.time = time->elapsed();
    writeFile(data);

    if(data.deviceNumber==1&&data.channel==2){
        data.value = convertU2C(data.value);
    }

    emit AdcTerconDataSend(data);
}

double Furnace::convertU2C(double U){
    long double temperature=0;
    U+=0.173;//temperature cold = 30C
    if (U < 1.874){
        temperature+=1.8494946E+02*U;
        temperature-=8.00504062E+01*pow(U,2);
        temperature+=1.0223743E+02*pow(U,3);
        temperature-=1.52248592E+02*pow(U,4);
        temperature+=1.88821343E+02*pow(U,5);
        temperature-=1.59085941E+02*pow(U,6);
        temperature+=8.2302788E+01*pow(U,7);
        temperature-=2.34181944E+01*pow(U,8);
        temperature+=2.7978626E+00*pow(U,9);
    }
    else if ((U >= 1.874) && (U < 10.332)){
        temperature+=1.291507177E+01;
        temperature+=1.466298863E+02*U;
        temperature-=1.534713402E+01*pow(U,2);
        temperature+=3.145945973E+00*pow(U,3);
        temperature-=4.163257839E-01*pow(U,4);
        temperature+=3.187963771E-02*pow(U,5);
        temperature-=1.2916375E-03*pow(U,6);
        temperature+=2.183475087E-05*pow(U,7);
        temperature-=1.447379511E-07*pow(U,8);
        temperature+=8.211272125E-09*pow(U,9);

    } else if( U >= 10.332){
        temperature-=8.087801117E+01;
        temperature+=1.621573104E+02*U;
        temperature-=8.536869453E+00*pow(U,2);
        temperature+=4.719686976E-01*pow(U,3);
        temperature-=1.441693666E-02*pow(U,4);
        temperature+=2.08161889E-04*pow(U,5);
    }
    return temperature;
}

void Furnace::writeFile(TerconData data){
    dataRecorder->writeFile(QString::number(data.time/1000.0,'f',3).toLatin1()+'\t'
                            + QString::number(data.deviceNumber).toLatin1()+'\t'
                            + QString::number(data.channel).toLatin1()+'\t'
                            + QString::number(data.value,'f',4).toLatin1()+"\r\n"
                            ,Shared::dataFile);


    if (data.deviceNumber==1){
        if (data.channel==1)
            dataRecorder->writeFile(QString::number(data.time/1000.0,'f',3).toLatin1()+'\t'
                                    +QString::number(data.value,'f',4).toLatin1()+'\t'
                                    ,Shared::mainSignalsFile);
        else
            dataRecorder->writeFile(QString::number(data.value,'f',4).toLatin1()+"\r\n"
                                    ,Shared::mainSignalsFile);
    }
    if (data.deviceNumber==2){
        if (data.channel==1)
            dataRecorder->writeFile(QString::number(data.time/1000.0,'f',3).toLatin1()+'\t'
                                    +QString::number(data.value,'f',4).toLatin1()+'\t'
                                    ,Shared::thermostatSignalsFile);
        else
            dataRecorder->writeFile(QString::number(data.value,'f',4).toLatin1()+"\r\n"
                                    ,Shared::thermostatSignalsFile);
    }
    if (data.deviceNumber==3){
        if (data.channel==1)
            dataRecorder->writeFile(QString::number(data.time/1000.0,'f',3).toLatin1()+'\t'
                                    +QString::number(data.value,'f',4).toLatin1()+'\t'
                                    ,Shared::calibrationHeaterFile);
        else
            dataRecorder->writeFile(QString::number(data.value,'f',4).toLatin1()+"\r\n"
                                    ,Shared::calibrationHeaterFile);
    }
}

bool Furnace::connectTercon(){
    tercon = new Tercon(this);
    tercon->description = tr("Температура ампулы и сопротивление калориметра");
    tercon->setPortName("COM6");
    tercon->setDeviceNumber(1);

    connect(tercon,SIGNAL(dataSend(TerconData)),
            this,SLOT(terconDataReceive(TerconData)));
    connect(tercon,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    tercon->startAck();

    terconThermostat = new Tercon(this);
    terconThermostat->setPortName("COM8");
    terconThermostat->description = tr("Температура термостата диф. термопара термостата");
    terconThermostat->setDeviceNumber(2);

    connect(terconThermostat,SIGNAL(dataSend(TerconData)),
            this,SLOT(terconDataReceive(TerconData)));
    connect(terconThermostat,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect(terconThermostat,SIGNAL(dataSend(TerconData)),
            this,SLOT(receiveData(TerconData)));
    terconThermostat->startAck();

    terconCalibrationHeater = new Tercon(this);
    terconCalibrationHeater->description = tr("Напряжение калибровочного нагревателя");
    terconCalibrationHeater->setPortName("COM9");
    terconCalibrationHeater->setDeviceNumber(3);

    connect(terconCalibrationHeater,SIGNAL(dataSend(TerconData)),
            this,SLOT(terconDataReceive(TerconData)));
    connect(terconCalibrationHeater,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    terconCalibrationHeater->startAck();

    arduino->setPortName("COM3");
    arduino->startAck();

    return true;
}

void Furnace::receiveData(TerconData data){
    static double furnaceTemperature;
    static bool furnaceTemperatureDataReady;

    if(data.channel==2&&data.deviceNumber==2){ //thermostat
        regulatorOfThermostat->setValueADC(data.value);
        //m_diagnostic->diagnosticThermocouple(data.value);
    }
    else if(data.channel==2&&data.deviceNumber==5){//main heater
        regulatorOfFurnace->setValueADC(data.value);
        furnaceTemperature = data.value;
        furnaceTemperatureDataReady = true;
    }
    else if(data.channel==1&&data.deviceNumber==5){ //up heater
        if (furnaceTemperatureDataReady){
            regulatorUpHeater_->setValueADC(data.value - furnaceTemperature);
        }
    }
    else if(data.channel==3&&data.deviceNumber==5){//down heater
        if (furnaceTemperatureDataReady)
            regulatorDownHeater_->setValueADC(data.value - furnaceTemperature);
    }

}

Diagnostic * Furnace::diagnostic(){
    return m_diagnostic;
}

Regulator * Furnace::regulatorFurnace(){
    return regulatorOfFurnace;
}

Regulator * Furnace::regulatorTermostat(){
    return regulatorOfThermostat;
}

Regulator * Furnace::regulatorUpHeater(){
    return regulatorUpHeater_;
}

Regulator * Furnace::regulatorDownHeater(){
    return regulatorDownHeater_;
}

Covers * Furnace::getCovers()
{
    return covers;
}

SafetyValve * Furnace::getSafetyValve()
{
    return safetyValve;
}

SampleLock * Furnace::getSampleLock()
{
    return sampleLock;
}

void Furnace::run(){

    connect (ltr43,SIGNAL(calibrationHeaterOff()),
             this,SIGNAL(calibrationHeaterOff()));

    connect(regulatorOfFurnace,SIGNAL(stopRegulator()),
            this,SLOT(stopDacFurnaceChannel()));
    connect(regulatorOfFurnace,SIGNAL(updateParameters()),
            this,SLOT(saveSettings()));
    connect(regulatorOfFurnace,SIGNAL(outPower(double)),
            this,SLOT(setDacValueFurnaceChannel(double)));
    connect(regulatorOfFurnace,SIGNAL(regulatorLogData(QString)),
            this,SLOT(writeFurnaceRegulatorLog(QString)));

    connect(regulatorOfThermostat,SIGNAL(stopRegulator()),
            this,SLOT(stopDacThermostateChannel()));
    connect(regulatorOfThermostat,SIGNAL(updateParameters()),
            this,SLOT(saveSettings()));
    connect(regulatorOfThermostat,SIGNAL(outPower(double)),
            this,SLOT(setDacValueThermostateChannel(double)));
    connect(regulatorOfThermostat,SIGNAL(regulatorLogData(QString)),
            this,SLOT(writeThermostatRegulatorLog(QString)));

    connect(regulatorUpHeater_,SIGNAL(stopRegulator()),
            this,SLOT(stopDacUpHeaterChannel()));
    connect(regulatorUpHeater_,SIGNAL(updateParameters()),
            this,SLOT(saveSettings()));
    connect(regulatorUpHeater_,SIGNAL(outPower(double)),
            this,SLOT(setDacValueUpHeaterChannel(double)));
    connect(regulatorUpHeater_,SIGNAL(regulatorLogData(QString)),
            this,SLOT(writeUpHeaterRegulatorLog(QString)));

    connect(regulatorDownHeater_,SIGNAL(stopRegulator()),
            this,SLOT(stopDacDownHeaterChannel()));
    connect(regulatorDownHeater_,SIGNAL(updateParameters()),
            this,SLOT(saveSettings()));
    connect(regulatorDownHeater_,SIGNAL(outPower(double)),
            this,SLOT(setDacValueDownHeaterChannel(double)));
    connect(regulatorDownHeater_,SIGNAL(regulatorLogData(QString)),
            this,SLOT(writeDownHeaterRegulatorLog(QString)));

    connect(dac,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect(adc,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect(ltr43,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect(ltr43,SIGNAL(readPortsSignal(DWORD)),
            covers,SLOT(statusPortLtr43(DWORD)));
    connect(ltr43,SIGNAL(readPortsSignal(DWORD)),
            safetyValve,SLOT(statusPortLtr43(DWORD)));
    connect(ltr43,SIGNAL(readPortsSignal(DWORD)),
            sampleLock,SLOT(statusPortLtr43(DWORD)));

    connect(sampleLock,SIGNAL(dropEnableSignal(bool)),
            safetyValve,SLOT(setRemoteDropEnable(bool)));
    connect(safetyValve,SIGNAL(remoteDropSignal()),
            covers,SLOT(openBottomCover()));
    connect(safetyValve,SIGNAL(remoteDropSignal()),
            covers,SLOT(openTopCover()));
    connect(safetyValve,SIGNAL(remoteDropCompletedSignal()),
            covers,SLOT(closeBottomCover()));
    connect(safetyValve,SIGNAL(remoteDropCompletedSignal()),
            covers,SLOT(closeTopCover()));

    connect(safetyValve,SIGNAL(remoteDropSignal()),
            sampleLock,SLOT(drop()));
    connect(covers,SIGNAL(openBottomCoverSignal()),
            sampleLock,SLOT(drop()));
    connect(covers,SIGNAL(openTopCoverSignal()),
            sampleLock,SLOT(drop()));

    connect(safetyValve,SIGNAL(remoteDropSignal()),
            arduino,SLOT(setWaitDropEnable()));
    connect(safetyValve,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect(covers,SIGNAL(openTopCoverSignal()),
            arduino,SLOT(waitDrop()));
    connect(covers,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect(sampleLock,SIGNAL(dropEnableSignal(bool)),
            arduino,SLOT(enableLed(bool)));
    connect(sampleLock,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));


    connect(arduino,SIGNAL(droped()),
            covers,SLOT(closeTopCover()));
    connect(arduino,SIGNAL(droped()),
            covers,SLOT(closeBottomCover()));
    connect(arduino,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));


    // ////////////////
    /*connect(ltr43,SIGNAL(lowerPressure()),
            m_diagnostic,SLOT(lowerPressure()));
    connect(ltr43,SIGNAL(normalPressure()),
            m_diagnostic,SLOT(normalPressure()));
    connect(ltr43,SIGNAL(upperPressure()),
            m_diagnostic,SLOT(upperPressure()));
    connect(regulatorOfFurnace,SIGNAL(startRegulator()),
            m_diagnostic,SLOT(startEmitAlarmSignals()));
    connect(regulatorOfFurnace,SIGNAL(stopRegulator()),
            m_diagnostic,SLOT(stopEmitAlarmSignals()));
    connect(m_diagnostic,SIGNAL(smoothOffRegulator()),
            regulatorOfFurnace,SLOT(smoothOff()));

    connect(m_diagnostic,SIGNAL(smoothOffRegulator()),
            regulatorOfThermostat,SLOT(smoothOff()));
    connect(m_diagnostic,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));*/
    //  ////////////////

    connect(adc,SIGNAL(dataSend(TerconData)),
            this,SLOT(terconDataReceive(TerconData)));
    connect(adc,SIGNAL(dataSend(TerconData)),
            this,SLOT(receiveData(TerconData)));


    connect(ltr114,SIGNAL(dataSend(TerconData)),
            this,SLOT(terconDataReceive(TerconData)));
    connect(ltr114,SIGNAL(dataSend(TerconData)),
            this,SLOT(receiveData(TerconData)));

    connect(ltr114,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));

    connectTercon();

    dac->initializationLTR();
    dac->initializationLTR34();
    dac->startDAC();

    adc->setTime(time);
    adc->initializationLTR27();
    adc->startADC();

    ltr43->initializationLTR43();

    ltr114->initialization();
    ltr114->start();

}

bool Furnace::closeAppRequest()
{
    arduino->stopAck();
    return ltr114->stop();
}

void Furnace::saveSettings()
{
    settings->beginGroup("Regulator of main heater");
    settings->setValue("ProcentPerSecond", regulatorOfFurnace->parameters().procentPerSec);
    settings->setValue("MinPower", regulatorOfFurnace->parameters().minPower);
    settings->setValue("MaxPower", regulatorOfFurnace->parameters().maxPower);
    settings->setValue("OffsetPower", regulatorOfFurnace->parameters().offset);
    settings->setValue("P", regulatorOfFurnace->parameters().gP);
    settings->setValue("I", regulatorOfFurnace->parameters().gI);
    settings->setValue("D", regulatorOfFurnace->parameters().gD);
    settings->setValue("MaxIntegralValue", regulatorOfFurnace->parameters().maxIntegralValue);
    settings->setValue("MaxProportionalValue", regulatorOfFurnace->parameters().maxProportionalValue);
    settings->setValue("AverageAdc", regulatorOfFurnace->parameters().averageCount);
    settings->setValue("AveragePower", regulatorOfFurnace->parameters().averagePowerCount);
    settings->endGroup();

    settings->beginGroup("Regulator of thermostat");
    settings->setValue("ProcentPerSecond", regulatorOfThermostat->parameters().procentPerSec);
    settings->setValue("MinPower", regulatorOfThermostat->parameters().minPower);
    settings->setValue("MaxPower", regulatorOfThermostat->parameters().maxPower);
    settings->setValue("OffsetPower", regulatorOfThermostat->parameters().offset);
    settings->setValue("P", regulatorOfThermostat->parameters().gP);
    settings->setValue("I", regulatorOfThermostat->parameters().gI);
    settings->setValue("D", regulatorOfThermostat->parameters().gD);
    settings->setValue("MaxIntegralValue", regulatorOfThermostat->parameters().maxIntegralValue);
    settings->setValue("MaxProportionalValue", regulatorOfThermostat->parameters().maxProportionalValue);
    settings->setValue("AverageAdc", regulatorOfThermostat->parameters().averageCount);
    settings->setValue("AveragePower", regulatorOfThermostat->parameters().averagePowerCount);
    settings->endGroup();

    settings->beginGroup("Regulator of up heater");
    settings->setValue("ProcentPerSecond", regulatorUpHeater_->parameters().procentPerSec);
    settings->setValue("MinPower", regulatorUpHeater_->parameters().minPower);
    settings->setValue("MaxPower", regulatorUpHeater_->parameters().maxPower);
    settings->setValue("OffsetPower", regulatorUpHeater_->parameters().offset);
    settings->setValue("P", regulatorUpHeater_->parameters().gP);
    settings->setValue("I", regulatorUpHeater_->parameters().gI);
    settings->setValue("D", regulatorUpHeater_->parameters().gD);
    settings->setValue("MaxIntegralValue", regulatorUpHeater_->parameters().maxIntegralValue);
    settings->setValue("MaxProportionalValue", regulatorUpHeater_->parameters().maxProportionalValue);
    settings->setValue("AverageAdc", regulatorUpHeater_->parameters().averageCount);
    settings->setValue("AveragePower", regulatorUpHeater_->parameters().averagePowerCount);
    settings->endGroup();

    settings->beginGroup("Regulator of down heater");
    settings->setValue("ProcentPerSecond", regulatorDownHeater_->parameters().procentPerSec);
    settings->setValue("MinPower", regulatorDownHeater_->parameters().minPower);
    settings->setValue("MaxPower", regulatorDownHeater_->parameters().maxPower);
    settings->setValue("OffsetPower", regulatorDownHeater_->parameters().offset);
    settings->setValue("P", regulatorDownHeater_->parameters().gP);
    settings->setValue("I", regulatorDownHeater_->parameters().gI);
    settings->setValue("D", regulatorDownHeater_->parameters().gD);
    settings->setValue("MaxIntegralValue", regulatorDownHeater_->parameters().maxIntegralValue);
    settings->setValue("MaxProportionalValue", regulatorDownHeater_->parameters().maxProportionalValue);
    settings->setValue("AverageAdc", regulatorDownHeater_->parameters().averageCount);
    settings->setValue("AveragePower", regulatorDownHeater_->parameters().averagePowerCount);
    settings->endGroup();
}

void Furnace::loadSettings()
{
    RegulatorParameters mainHeaterParameters;
    mainHeaterParameters.procentPerSec = settings->value("Regulator of main heater/ProcentPerSecond").toDouble();
    mainHeaterParameters.minPower = settings->value("Regulator of main heater/MinPower").toDouble();
    mainHeaterParameters.maxPower = settings->value("Regulator of main heater/MaxPower").toDouble();
    mainHeaterParameters.offset = settings->value("Regulator of main heater/OffsetPower").toDouble();
    mainHeaterParameters.gP = settings->value("Regulator of main heater/P").toDouble();
    mainHeaterParameters.gD = settings->value("Regulator of main heater/D").toDouble();
    mainHeaterParameters.gI = settings->value("Regulator of main heater/I").toDouble();
    mainHeaterParameters.maxIntegralValue = settings->value("Regulator of main heater/MaxIntegralValue").toDouble();
    mainHeaterParameters.maxProportionalValue = settings->value("Regulator of main heater/MaxProportionalValue").toDouble();
    mainHeaterParameters.averageCount = settings->value("Regulator of main heater/AverageAdc").toInt();
    mainHeaterParameters.averagePowerCount = settings->value("Regulator of main heater/AveragePower").toInt();

    regulatorOfFurnace->setParameters(mainHeaterParameters);

    RegulatorParameters thermostatParameters;
    thermostatParameters.procentPerSec = settings->value("Regulator of thermostat/ProcentPerSecond").toDouble();
    thermostatParameters.minPower = settings->value("Regulator of thermostat/MinPower").toDouble();
    thermostatParameters.maxPower = settings->value("Regulator of thermostat/MaxPower").toDouble();
    thermostatParameters.offset = settings->value("Regulator of thermostat/OffsetPower").toDouble();
    thermostatParameters.gP = settings->value("Regulator of thermostat/P").toDouble();
    thermostatParameters.gD = settings->value("Regulator of thermostat/D").toDouble();
    thermostatParameters.gI = settings->value("Regulator of thermostat/I").toDouble();
    thermostatParameters.maxIntegralValue = settings->value("Regulator of thermostat/MaxIntegralValue").toDouble();
    thermostatParameters.maxProportionalValue = settings->value("Regulator of thermostat/MaxProportionalValue").toDouble();
    thermostatParameters.averageCount = settings->value("Regulator of thermostat/AverageAdc").toInt();
    thermostatParameters.averagePowerCount = settings->value("Regulator of thermostat/AveragePower").toInt();

    regulatorOfThermostat->setParameters(thermostatParameters);

    RegulatorParameters upHeaterParameters;
    upHeaterParameters.procentPerSec = settings->value("Regulator of up heater/ProcentPerSecond").toDouble();
    upHeaterParameters.minPower = settings->value("Regulator of up heater/MinPower").toDouble();
    upHeaterParameters.maxPower = settings->value("Regulator of up heater/MaxPower").toDouble();
    upHeaterParameters.offset = settings->value("Regulator of up heater/OffsetPower").toDouble();
    upHeaterParameters.gP = settings->value("Regulator of up heater/P").toDouble();
    upHeaterParameters.gD = settings->value("Regulator of up heater/D").toDouble();
    upHeaterParameters.gI = settings->value("Regulator of up heater/I").toDouble();
    upHeaterParameters.maxIntegralValue = settings->value("Regulator of up heater/MaxIntegralValue").toDouble();
    upHeaterParameters.maxProportionalValue = settings->value("Regulator of up heater/MaxProportionalValue").toDouble();
    upHeaterParameters.averageCount = settings->value("Regulator of up heater/AverageAdc").toInt();
    upHeaterParameters.averagePowerCount = settings->value("Regulator of up heater/AveragePower").toInt();

    regulatorUpHeater_->setParameters(upHeaterParameters);

    RegulatorParameters downHeaterParameters;
    downHeaterParameters.procentPerSec = settings->value("Regulator of down heater/ProcentPerSecond").toDouble();
    downHeaterParameters.minPower = settings->value("Regulator of down heater/MinPower").toDouble();
    downHeaterParameters.maxPower = settings->value("Regulator of down heater/MaxPower").toDouble();
    downHeaterParameters.offset = settings->value("Regulator of down heater/OffsetPower").toDouble();
    downHeaterParameters.gP = settings->value("Regulator of down heater/P").toDouble();
    downHeaterParameters.gD = settings->value("Regulator of down heater/D").toDouble();
    downHeaterParameters.gI = settings->value("Regulator of down heater/I").toDouble();
    downHeaterParameters.maxIntegralValue = settings->value("Regulator of down heater/MaxIntegralValue").toDouble();
    downHeaterParameters.maxProportionalValue = settings->value("Regulator of down heater/MaxProportionalValue").toDouble();
    downHeaterParameters.averageCount = settings->value("Regulator of down heater/AverageAdc").toInt();
    downHeaterParameters.averagePowerCount = settings->value("Regulator of down heater/AveragePower").toInt();

    regulatorDownHeater_->setParameters(downHeaterParameters);
}
