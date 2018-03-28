#include "furnace.h"
#include <math.h>
#include <QDebug>

Furnace * Furnace::g_furnace = 0;

Furnace::Furnace(QObject *parent) :
    QObject(parent)
{
    
    g_furnace = this;
    
    time = new QElapsedTimer();
    time->start();
    
    settings =  new QSettings(tr("IT"),tr("furnace"),this);
    
    dataManager = new DataManager(this);
    deviceManager = new DeviceManager(this);
    
    regulatorOfFurnace = new Regulator(this);
    regulatorOfFurnace->setObjectName("regulator-main-heater");
    regulatorOfThermostat = new Regulator(this);
    regulatorOfThermostat->setObjectName("regulator-thermostat");
    regulatorUpHeater_ = new Regulator(this);
    regulatorUpHeater_->setObjectName("regulator-up-heater");
    regulatorDownHeater_ = new Regulator(this);
    regulatorDownHeater_->setObjectName("regulator-down-heater");
    covers = new Covers(this);
    safetyValve = new SafetyValve(this);
    sampleLock = new SampleLock(this);
    dropDevice = new DropDevice(this);
    arduino = new Arduino(this);
    agilent = new Agilent(this);
    
    dac = new DAC(this);
    adc = new ADC(this);
    ltr43 = new Ltr43(this);
    ltr114 = new Ltr114(this);
    m_diagnostic = new Diagnostic(this);
    
    covers->setLTR43(ltr43);
    safetyValve->setLTR43(ltr43);
    sampleLock->setLTR43(ltr43);
    
    dropDevice->setCovers(covers);
    dropDevice->setSampleLock(sampleLock);
    dropDevice->setSafetyValve(safetyValve);
    dropDevice->setDropSensor(arduino);
    dropDevice->init();
    
    loadSettings();
    loadJSONSettings();
    
    connect(dataManager, SIGNAL(dataSend(TerconData)), this, SIGNAL(AdcTerconDataSend(TerconData)));
    connect(dataManager, SIGNAL(dataSend(TerconData)), this, SLOT(receiveData(TerconData)));
}

Furnace::~Furnace()
{
    delete time;
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

void Furnace::beginDataRecord(){
    dataManager->startRecordExperimentFile();
    message(tr("Запись эксперимента в файл включена."),Shared::information);
}

void Furnace::endDataRecord(){
    dataManager->stopRecordExperimentFile();
    message(tr("Запись эксперимента в файл выключена."),Shared::information);
}

void Furnace::turnOnCalibrationHeater(int duration)
{
    if(duration>0)
        ltr43->turnOnCalibrationHeaterTimer(duration);
    else
        ltr43->turnOnCalibrationHeater();
    
    message(tr("Калибровочный нагреватель включен."),Shared::information);
}

void Furnace::turnOffCalibrationHeater()
{
    ltr43->turnOffCalibrationHeater();
}

void Furnace::writeLogMessageToFile(const QString &message)
{
    dataManager->addLogMessage(message);
}

void Furnace::setDacValueFurnaceChannel(double value)
{
    dac->setValueDAC(value,0);
}

void Furnace::setDacValueThermostatChannel(double value)
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
    
    //dataRecorder->writeFile(logString,Shared::regulatorFurnaceFile);
}

void Furnace::writeThermostatRegulatorLog(const QString &logString)
{
    //dataRecorder->writeFile(logString,Shared::regulatorThermostatFile);
}

void Furnace::writeUpHeaterRegulatorLog(const QString &logString)
{
    //dataRecorder->writeFile(logString,Shared::regulatorUpHeaterFile);
}

void Furnace::writeDownHeaterRegulatorLog(const QString &logString)
{
    //dataRecorder->writeFile(logString,Shared::regulatorDownHeaterFile);
}

void Furnace::stopDacFurnaceChannel()
{
    dac->stopDAC(0);
}

void Furnace::stopDacThermostatChannel()
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
    dataManager->addData(data);
    
    emit AdcTerconDataSend(data);
}

bool Furnace::connectTercon(){
    if(settings_.isEmpty()) return false;
    QJsonArray devicesArray = settings_["Devices"].toArray();
    for (int i = 0; i < devicesArray.size(); ++i) {
        QJsonObject deviceObject = devicesArray[i].toObject();
        if (deviceObject.isEmpty())continue;
        Device * device = deviceManager->createDeviceFromJSON(deviceObject);
        if (device){
            connect(device,SIGNAL(message(QString,Shared::MessageLevel)),
                    this,SIGNAL(message(QString,Shared::MessageLevel)));
            
            if(!device->connectDevice()){
                qDebug() << "connect device fail";
            }
            if (!device->start()){
                qDebug() << "start device fail";
            }
            
            connect(device,SIGNAL(dataSend(TerconData)),
                    this,SLOT(terconDataReceive(TerconData)));
        }
    }
    arduino->setPortName("COM3");
    arduino->startAck();
    
    return true;
}

void Furnace::receiveData(TerconData data){
    if(data.id == "{802cdd57-b870-4cdb-a1c2-2e430c70981c}"){ //thermostat
        regulatorOfThermostat->setValueADC(data.value);
    }
    else if(data.id == "{a5f14434-bbc4-435e-be15-a7ad91de2701}"){//main heater
        regulatorOfFurnace->setValueADC(data.value);
    }
    else if(data.id == "{33bc6e29-4e26-41a6-85f1-cfa0bc5525b9}"){ //up heater
        double setPointMainHeater = regulatorOfFurnace->getSetPoint();
        if (setPointMainHeater!=0){
            regulatorUpHeater_->setValueADC(data.value - setPointMainHeater);
        }
    }
    else if(data.id == "{7cb4d773-5b1d-4060-b316-24045308317f}"){//down heater
        double setPointMainHeater = regulatorOfFurnace->getSetPoint();
        if (setPointMainHeater!=0){
            regulatorDownHeater_->setValueADC(data.value - setPointMainHeater);
        }
    }
    
}

Diagnostic * Furnace::diagnostic(){
    return m_diagnostic;
}

Regulator * Furnace::regulatorFurnace(){
    return regulatorOfFurnace;
}

Regulator * Furnace::regulatorThermostat(){
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
    
    connect (ltr43,SIGNAL(calibrationHeaterOff(qint64)),
             this,SIGNAL(calibrationHeaterOff()));
    
    connect(regulatorOfFurnace,SIGNAL(stopRegulator()),
            this,SLOT(stopDacFurnaceChannel()));
    connect(regulatorOfFurnace,SIGNAL(stopRegulator()),
            regulatorUpHeater_,SLOT(regulatorStop()));
    connect(regulatorOfFurnace,SIGNAL(stopRegulator()),
            regulatorDownHeater_,SLOT(regulatorStop()));
    connect(regulatorOfFurnace,SIGNAL(updateParameters()),
            this,SLOT(saveSettings()));
    connect(regulatorOfFurnace,SIGNAL(outPower(double)),
            this,SLOT(setDacValueFurnaceChannel(double)));
    connect(regulatorOfFurnace,SIGNAL(regulatorLogData(QString)),
            this,SLOT(writeFurnaceRegulatorLog(QString)));
    
    connect(regulatorOfThermostat,SIGNAL(stopRegulator()),
            this,SLOT(stopDacThermostatChannel()));
    connect(regulatorOfThermostat,SIGNAL(updateParameters()),
            this,SLOT(saveSettings()));
    connect(regulatorOfThermostat,SIGNAL(outPower(double)),
            this,SLOT(setDacValueThermostatChannel(double)));
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
    connect(sampleLock,SIGNAL(dropEnableSignal(bool)),
            arduino,SLOT(enableLed(bool)));
    
    connect(safetyValve,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect(covers,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect(sampleLock,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    connect(dropDevice,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
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
    
    
    connect(ltr114,SIGNAL(dataSend(TerconData)),
            this,SLOT(terconDataReceive(TerconData)));
    connect(ltr114,SIGNAL(dataSend(TerconData)),
            this,SLOT(receiveData(TerconData)));
    connect(ltr114,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    
    
    dataManager->setSettings(settings_);
    connectTercon();
    
    dac->initializationLTR();
    dac->initializationLTR34();
    dac->startDAC();
    
    ltr43->initializationLTR43();
    ltr114->initialization();
    ltr114->start();

    connect(agilent,SIGNAL(dataSend(TerconData)),
            this,SLOT(terconDataReceive(TerconData)));
    connect(agilent,SIGNAL(message(QString,Shared::MessageLevel)),
            this,SIGNAL(message(QString,Shared::MessageLevel)));
    agilent->startAck();
}

bool Furnace::closeAppRequest()
{
    saveJSONSettings();
    arduino->stopAck();
    agilent->stopAck();
    deviceManager->destroyDevices();
    
    if (ltr114->stop()){
        //dataRecorder->convertDataFile();
        return true;
    }
    return false;
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

void Furnace::saveJSONSettings()
{
    /*QFile saveFile(QStringLiteral("settings.json"));
      
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
    }
    
    QJsonObject jsonObject = devices.at(0)->getSetting();
    
    QJsonDocument saveDoc(jsonObject);
    saveFile.write(saveDoc.toJson());*/
}

void Furnace::loadJSONSettings()
{
    QFile loadFile("settings/settings.json");
    
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
    }
    
    QByteArray saveData = loadFile.readAll();
    
    QJsonDocument loadDoc( QJsonDocument::fromJson(saveData));
    
    settings_ = loadDoc.object();
}

QJsonObject Furnace::getSettings()
{
    return settings_;
}
