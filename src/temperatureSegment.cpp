#include "temperatureSegment.h"
#include <QObject>

Segment::Segment(double beginTemperature, double endTemperature, double velocity, double duration):
    m_duration(duration),
    m_beginTemperature(beginTemperature),
    m_endTemperature(endTemperature),
    m_velocity(velocity)
{

}

double Segment::duration(){
    return m_duration;
}

double Segment::velocity(){
    return m_velocity;
}

double Segment::beginTemperature(){
    return m_beginTemperature;
}

double Segment::endTemperature(){
    return m_endTemperature;
}

QString Segment::typeTitle(){
    return m_typeTitle;
}

Type Segment::type(){
    return m_type;
}

void Segment::setEndTemperature(double temperature){
    m_endTemperature = temperature;
}

HeatingSegment::HeatingSegment(double beginTemperature, double endTemperature, double velocity):
    Segment(beginTemperature,endTemperature,velocity,0)
{
    m_typeTitle= QObject::tr("Нагрев");
    m_type = heating;
}

double HeatingSegment::requiredTemperature(double time){
    return m_beginTemperature+m_velocity*time/60;
}

void HeatingSegment::calculateDuration(){
    m_duration = (m_endTemperature-m_beginTemperature)/m_velocity * 60;
}

void HeatingSegment::setBeginTemperature(double temperature){
    m_beginTemperature = temperature;
}

bool HeatingSegment::isEnd(double temperature){
    return (m_endTemperature<temperature)?true:false;
}


CoolingSegment::CoolingSegment(double beginTemperature, double endTemperature, double velocity):
    Segment(beginTemperature,endTemperature,velocity,0)
{
    m_typeTitle= QObject::tr("Охлаждение");
    m_type = cooling;
}

double CoolingSegment::requiredTemperature(double time){
    return m_beginTemperature-m_velocity*time/60;
}

void CoolingSegment::calculateDuration(){
    m_duration = (m_beginTemperature-m_endTemperature)/m_velocity * 60;
}

void CoolingSegment::setBeginTemperature(double temperature){
    m_beginTemperature = temperature;
}

bool CoolingSegment::isEnd(double temperature){
    return (m_endTemperature>temperature)?true:false;
}


IsotermalSegment::IsotermalSegment(double temperature, double duration):
    Segment(temperature,temperature,0,duration)
{
    m_typeTitle= QObject::tr("Изотерма");
    m_type = isoterm;
}

double IsotermalSegment::requiredTemperature(double time){
    return m_endTemperature;
}

void IsotermalSegment::setBeginTemperature(double temperature){
    m_beginTemperature = temperature;
    m_endTemperature = temperature;
}

bool IsotermalSegment::isEnd(double temperature){
    return (m_endTemperature>temperature)?true:false;
}

