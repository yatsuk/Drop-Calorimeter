#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QSharedPointer>

#include "temperatureSegment.h"


class Segments : public QObject
{
    Q_OBJECT
public:
    explicit Segments(QObject *parent = 0);
    int segmentCount();
    bool isEmpty();
    Segment *segment(int number);
    void setBeginTemperature(int segmentNumber, double temperature);
    
signals:
    void update();
    void totalDuration(double);
    void remainingTime(double);
    
public slots:
    void addSegment(Segment * segment);
    void deleteSegment(int number);
    void save(const QString &fileName);
    void load(const QString &fileName);

private:
    void calculateDurationNewSegment(Segment * segment);
    void calculateTotalDuration();
    void calculateRemainingTime();

    QVector <QSharedPointer<Segment> > segments;
    double beginTemperature;


};

#endif // SEGMENTS_H
