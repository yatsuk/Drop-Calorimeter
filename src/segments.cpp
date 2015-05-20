#include "segments.h"
#include <QDebug>
#include <QDataStream>
#include <QFile>
#include <QTime>

Segments::Segments(QObject *parent) :
    QObject(parent)
{
    beginTemperature = 20;
}

int Segments::segmentCount(){
    return segments.size();
}

bool Segments::isEmpty(){
    return segments.isEmpty();
}

Segment * Segments::segment(int number){
    return segments.at(number).data();
}

void Segments::setBeginTemperature(int segmentNumber,double temperature){
    beginTemperature = temperature;
    Segment * segment = segments.at(segmentNumber).data();
    segment->setBeginTemperature(temperature);
    segment->calculateDuration();
    calculateTotalDuration();

    emit update();
}

void Segments::addSegment(Segment *segment){
    calculateDurationNewSegment(segment);
    segments.append(QSharedPointer <Segment> (segment));
    calculateTotalDuration();

    emit update();
}

void Segments::deleteSegment(int number){
    segments.remove(number);
    calculateTotalDuration();

    emit update();
}

void Segments::save(const QString &fileName){
    if (fileName.isEmpty())return;
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly))
        return;
    QDataStream out(&file);
    for (int i=0;i<segments.size();++i){
        Segment * segment = segments.at(i).data();
        out << (qint32)segment->type()
            << segment->velocity()
            << segment->endTemperature()
            << segment->duration();
    }
}

void Segments::load(const QString &fileName){
    if (fileName.isEmpty())return;
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
        return;
    QDataStream in(&file);

    segments.clear();

    for(int i =0;!in.atEnd();++i){
        qint32 type;
        in >> type;

        double velocity,endTemperature,duration;
        in >> velocity >> endTemperature >> duration;

        if(type==heating)
            addSegment(new HeatingSegment(0,endTemperature,velocity));
        else if (type==cooling)
            addSegment(new CoolingSegment(0,endTemperature,velocity));
        else if (type==isoterm)
            addSegment(new IsotermalSegment(endTemperature,duration));
    }
}

void Segments::calculateDurationNewSegment(Segment *segment){
    if (segments.size()==0){
        segment->setBeginTemperature(beginTemperature);
    } else{
        Segment * prevSegment = segments[segments.size()-1].data();
        segment->setBeginTemperature(prevSegment->endTemperature());
    }
    segment->calculateDuration();
}

void Segments::calculateTotalDuration(){
    double duration = 0;
    Segment * segment;
    for (int i = 0;i < segments.size();++i){
        segment = segments.at(i).data();
        duration+=segment->duration();
    }
    emit totalDuration(duration);

}

void Segments::calculateRemainingTime(){

}
