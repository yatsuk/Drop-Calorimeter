#include "diagnostic.h"
#include <QDebug>

void Diagnostic::diagnosticThermocouple(double value){
    if (firstValue){
        oldValue = value;
        firstValue = false;
        if (enableSignals){
            emit controlThermocouple(false);
        }
        return;
    }

    if ((qAbs(value-oldValue)/oldValue)<0.5){
        oldValue = value;

        if (statusThermocouple!=normalThermocouple){
            if (enableSignals){
                emit controlThermocouple(false);
                emit message(tr("Диагностика: Термопара в норме."),Shared::information);
            }
        }
        statusThermocouple = normalThermocouple;
    }
    else{
        /*if (statusThermocouple!=breakage){
            if (enableSignals){
                emit controlThermocouple(true);
                emit message(tr("Диагностика: Обрыв термопары!"),critical);
                emit smoothOffRegulator();
            }
        }*/
        statusThermocouple = breakage;
    }
}

void Diagnostic::upperPressure(){
    if (enableSignals&&m_Pressure!=upper){
        emit alarmUpperPressure();
        emit message(tr("Диагностика: Превышенное давление в системе охлаждения печи калориметра!"),Shared::critical);
        offRegulatorTimer->start();
        m_Pressure=upper;
    }
}

void Diagnostic::normalPressure(){
    if (enableSignals&&m_Pressure!=normal){
        emit alarmNormalPressure();
        emit message(tr("Диагностика: Давление в системе охлаждения печи калориметра в пределах нормы."),Shared::information);
        offRegulatorTimer->stop();
        m_Pressure=normal;
    }
}

void Diagnostic::lowerPressure(){
    if (enableSignals&&m_Pressure!=lower){
        emit alarmLowerPressure();
        emit message(tr("Диагностика: Пониженное давление в системе охлаждения печи калориметра!"),Shared::critical);
        offRegulatorTimer->start();
        m_Pressure=lower;
    }
}

void Diagnostic::startEmitAlarmSignals(){
    enableSignals = true;
    firstValue = true;
    m_Pressure = undefined;
}

void Diagnostic::stopEmitAlarmSignals(){
    enableSignals = false;
}

void Diagnostic::enableEmitAlarmSignals(bool enable){
    enableSignals = enable;
}

void Diagnostic::offRegulatorTimerTimeout(){
    emit smoothOffRegulator();
    offRegulatorTimer->stop();
}

Diagnostic::Diagnostic(QObject *parent):
    QObject(parent)
{
    firstValue = true;
    enableSignals = false;
    statusThermocouple = normalThermocouple;
    offRegulatorTimer = new QTimer(this);
    offRegulatorTimer->setInterval(60000);
    connect(offRegulatorTimer,SIGNAL(timeout()),SLOT(offRegulatorTimerTimeout()));
}


