#include "regulator.h"
#include "temperatureSegment.h"
#include <QPolygonF>
#include <QDebug>


Regulator::Regulator(QObject *parent) :
    QObject(parent)
{
    smoothOffTimer = new QTimer(this);
    connect(smoothOffTimer,SIGNAL(timeout()),this,SLOT(smoothTimerTimeout()));
    progPowerTimer = new QTimer(this);
    connect(progPowerTimer,SIGNAL(timeout()),this,SLOT(progTimerTimeout()));

    mode=Shared::manual;

    temperatureProgramm = new Segments(this);
    regulatorOn = false;
    connect(temperatureProgramm,SIGNAL(update()),this,SLOT(updateTemperatureProgramm()));

    setPoint = 0;
    firstPoint = true;
    prevError = 0;
    derivative = 0;
    integral = 0;
    count = 0;

    velocity_ = 0;
    constVelocitySegment_ = 0;

    state_["out-power"]=0;
    state_["mode"]="manual";
    state_["enable"]=false;
    prepareAndSendState();
}

Regulator::~Regulator(){
    delete constVelocitySegment_;
}

void Regulator::smoothOff(){
    emit manualMode();
    smoothOffTimer->start(1000);
    state_["mode"]="manual";
    prepareAndSendState();
}

void Regulator::smoothTimerTimeout(){
    power -=1;
    if(power<0){
        emit outPower(0);
        emit emergencyStopRegulator();
        regulatorEmergencyStop();
        smoothOffTimer->stop();

        state_["out-power"]=0;
        prepareAndSendState();

        return;
    }
    state_["out-power"]=power;
    prepareAndSendState();
    emit outPower(power);
}

void Regulator::progTimerTimeout(){
    if(regulatorOn){
        power+=incPower_;
        emit outPower(power);
        state_["out-power"]=power;
        prepareAndSendState();
    }
}

double Regulator::computePowerProg(double temperature){
    /*QPolygonF polygon;
    polygon << QPointF(450,10)<< QPointF(790,15)<< QPointF(1060,20)<< QPointF(1240,25)
            << QPointF(1400,30)<< QPointF(1520,35)<< QPointF(1600,37.5);
    QwtSpline spline;
    spline.setPoints(polygon);
    return spline.value(temperature);*/
    return 0;
}

void Regulator::calculatePower(double value){  
    double error = setPoint - value;

    if(!firstPoint)
        derivative = error - prevError;

    prevError = error;

    power = parameters_.offset + (error*parameters_.gP)+(derivative*parameters_.gD)+((integral+error)*parameters_.gI);

    if ((power<parameters_.maxIntegralValue)
            &&  (power <= parameters_.maxPower)
            &&  (power >= parameters_.minPower)){
        integral += error;
    }



    if (power>parameters_.maxPower){
        power = parameters_.maxPower;
    } else if (power < parameters_.minPower){
        power = parameters_.minPower;
    }


    if ((power-prevPower)>parameters_.procentPerSec)
        power=prevPower+parameters_.procentPerSec;
    else if ((power-prevPower)<(-1*parameters_.procentPerSec))
        power=prevPower-parameters_.procentPerSec;

    prevPower = power;
    firstPoint = false;

    emit outPower(power);
    emit regulatorLogData(QString("%1\t%3\t%4\t%5\t%6\t%7\r\n").arg(value).arg(error).arg(power).arg(error*parameters_.gP).arg(integral*parameters_.gI).arg(derivative*parameters_.gD));

    state_["value"]=value;
    state_["delta"]=error;
    state_["P*delta"]=error*parameters_.gP;
    state_["I*delta"]=integral*parameters_.gI;
    state_["D*delta"]=derivative*parameters_.gD;
    state_["set-point"]=setPoint;
    state_["out-power"]=power;
    prepareAndSendState();
}

bool Regulator::isEndSegment(double currentTemperature, int segmentNumber){
    return temperatureProgramm->segment(segmentNumber)->isEnd(currentTemperature);
}

void Regulator::goToSegment(int numberSegment){
    if (numberSegment>currentSegment){
        startAutoRegulatorTime.restart();
        currentSegment=numberSegment;
        temperatureProgramm->setBeginTemperature(currentSegment,currentValueADC);//recalculation temperature segments(new temperature);
        currentTemperatureSegment(currentSegment);
    }
}

void Regulator::setValueADC(double value){
    if(firstValue&&regulatorOn&&mode==Shared::automatic){
        startAutoRegulatorTime.restart();
        while(isEndSegment(value,currentSegment))
            currentSegment++;
        temperatureProgramm->setBeginTemperature(currentSegment,value);//recalculation temperature segments(new temperature);
        currentTemperatureSegment(currentSegment);
        firstValue = false;
    }
    else if(firstValue&&regulatorOn&&mode==Shared::stopCurrentTemperature){
        setPoint = value;
        firstValue = false;
        return;
    }
    else if(firstValue&&regulatorOn&&mode==Shared::programPower){
        initialTemperature_ = value;
        progPowerTimer->start(1000);
        power = computePowerProg(initialTemperature_);
        incPower_= (computePowerProg(temperatureProg_)-computePowerProg(initialTemperature_))/duration_/60;

        if (temperatureProg_>initialTemperature_)
            progPowerHeat_=true;
        else
            progPowerHeat_ = false;

        firstValue = false;
        return;
    }
    else if (firstValue&&regulatorOn&&mode==Shared::constVelocity){
        startAutoRegulatorTime.restart();
        delete constVelocitySegment_;
        constVelocitySegment_ = 0;
        if (velocity_>0){
            if (setPoint)
                constVelocitySegment_= new HeatingSegment(setPoint,2500,velocity_);
            else
                constVelocitySegment_= new HeatingSegment(value,2500,velocity_);
        }
        else if (velocity_<0){
            if (setPoint)
                constVelocitySegment_= new CoolingSegment(setPoint,0,velocity_*-1);
            else
                constVelocitySegment_= new CoolingSegment(value,0,velocity_*-1);
        }else if (!setPoint){
            setPoint=value;
        }

        firstValue = false;
        return;
    }

    currentValueADC = value;
    if(regulatorOn){
        if(mode==Shared::automatic){
            Segment * segment = temperatureProgramm->segment(currentSegment);
            double timeMinutes = startAutoRegulatorTime.elapsed()/1000.0;

            if (timeMinutes>segment->duration()){
                if (currentSegment==temperatureProgramm->segmentCount()-1){
                    regulatorStop();
                    return;
                } else{
                    startAutoRegulatorTime.restart();
                    currentSegment++;
                    currentTemperatureSegment(currentSegment);
                }
            }

            setPoint = segment->requiredTemperature(timeMinutes);
            calculatePower(value);
            QJsonObject jsonTemperatureSegment;
            jsonTemperatureSegment["duration"]=segment->duration();
            jsonTemperatureSegment["velocity"]=segment->velocity();
            jsonTemperatureSegment["elapsed-time"]=timeMinutes;
            jsonTemperatureSegment["type"]=segment->typeTitle();
            state_["temperature-segment"]=jsonTemperatureSegment;
            prepareAndSendState();
        }

        else if (mode==Shared::stopCurrentTemperature){
            calculatePower(value);
        }
        else if (mode==Shared::programPower){
            if (value>temperatureProg_&&progPowerHeat_)
                progPowerTimer->stop();
            else if (value<temperatureProg_&&!progPowerHeat_)
                progPowerTimer->stop();
        }
        else if (mode==Shared::constVelocity){
            if (velocity_!=0){
                double timeMinutes = startAutoRegulatorTime.elapsed()/1000.0;
                setPoint = constVelocitySegment_->requiredTemperature(timeMinutes);
            }
            calculatePower(value);
        }
        else if (mode == Shared::constValue){
            calculatePower(value);
        }
    }
}

void Regulator::regulatorEmergencyStop(){
    regulatorStop();
    state_["emergency-stop"]=true;
    prepareAndSendState();
}

void Regulator::timerTestSlot(){
    static double value = 30;
    setValueADC(value);
    value+=20;
}

void Regulator::regulatorStart(){
    regulatorOn = true;
    firstValue = true;
    currentSegment = 0;
    prevPower =0;
    setPoint = 0;

    emit startRegulator();

    state_["emergency-stop"]=false;
    state_["enable"]=true;
    prepareAndSendState();
}

void Regulator::regulatorStop(){
    regulatorOn = false;
    clearRegulator();

    progPowerTimer->stop();

    emit stopRegulator();
    emit outPower(0);

    state_["enable"]=false;
    state_["out-power"]=0;
    prepareAndSendState();
}

void Regulator::clearRegulator(){
    firstPoint = true;
    integral = 0;
    derivative = 0;
}

RegulatorParameters  Regulator::parameters()
{
    return parameters_;
}

Segments * Regulator::getTemperatureProgramm(){
    return temperatureProgramm;
}

void Regulator::updateTemperatureProgramm(){

}

void Regulator::setTemperatureProgramm(Segments *tProgramm){
    temperatureProgramm->disconnect();
    temperatureProgramm = tProgramm;
    connect(temperatureProgramm,SIGNAL(update()),this,SLOT(updateTemperatureProgramm()));
}

void Regulator::setMode(Shared::RegulatorMode regulatorMode){
    progPowerTimer->stop();

    if (regulatorMode==Shared::constVelocity){
        setPoint = 0;
        if (power>0)
            integral = power/parameters_.gI;
    }

    mode = regulatorMode;
    firstValue = true;

    state_["mode"]=convertModeToString(mode);
    prepareAndSendState();
}

void Regulator::setParameters(RegulatorParameters parameters)
{
    if(integral!=0){
        integral = integral*parameters_.gI/parameters.gI;
    }
    parameters_ = parameters;
    emit updateParameters();
}

void Regulator::setValueManual(double value)
{
    if (regulatorOn){
        power = value;
        prevPower = power;
        emit outPower(value);
        state_["out-power"]=value;
        prepareAndSendState();
    }
}

void Regulator::setTargetValue(double value)
{
    setPoint = value;
}

void Regulator::setTemperatureProgMode(double temperature){
    temperatureProg_ = temperature;
    firstValue = true;
}

void Regulator::setDurationProgMode(double duration){
    duration_ =duration;
    firstValue = true;
}

void Regulator::setVelocity(double velocity)
{
    velocity_ = velocity;
    firstValue = true;
}

double Regulator::getSetPoint()
{
    return setPoint;
}

QString Regulator::convertModeToString(Shared::RegulatorMode regulatorMode)
{
    if (regulatorMode==Shared::automatic) {
        return "automatic";
    } else if (regulatorMode==Shared::manual) {
        return "manual";
    } else if (regulatorMode==Shared::programPower) {
        return "programPower";
    } else if (regulatorMode==Shared::stopCurrentTemperature) {
        return "stopCurrentTemperature";
    } else if (regulatorMode==Shared::constVelocity) {
        return "constVelocity";
    } else if (regulatorMode==Shared::constValue) {
        return "constValue";
    }

    return "";
}

void Regulator::prepareAndSendState()
{
    state_["sender"]=objectName();
    emit state(state_);
}
