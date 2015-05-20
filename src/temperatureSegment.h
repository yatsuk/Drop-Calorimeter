#ifndef TEMPERATURESEGMENT_H
#define TEMPERATURESEGMENT_H
#include <QString>

enum Type{heating,isoterm,cooling};

class Segment
{
public:
    Segment (double beginTemperature,double endTemperature,double velocity,double duration);
    virtual ~Segment(){}
    double duration();
    double velocity();
    double beginTemperature();
    double endTemperature();
    QString typeTitle();
    Type type();
    virtual void setBeginTemperature(double temperature)=0;
    void setEndTemperature(double temperature);
    virtual double requiredTemperature(double time)=0;
    virtual void calculateDuration()=0;
    virtual bool isEnd(double temperature)=0;

protected:
    QString m_typeTitle;
    Type m_type;
    double m_duration;//in min
    double m_beginTemperature;
    double m_endTemperature;
    double m_velocity;
};

class HeatingSegment: public Segment
{
public:
    HeatingSegment(double beginTemperature,double endTemperature,double velocity);
    double requiredTemperature(double time);
    void calculateDuration();
    void setBeginTemperature(double temperature);
    bool isEnd(double temperature);
};

class CoolingSegment: public Segment
{
public:
    CoolingSegment(double beginTemperature,double endTemperature,double velocity);
    double requiredTemperature(double time);
    void calculateDuration();
    void setBeginTemperature(double temperature);
    bool isEnd(double temperature);
};

class IsotermalSegment: public Segment
{
public:
    IsotermalSegment(double temperature,double duration);
    double requiredTemperature(double time);
    void calculateDuration(){}
    void setBeginTemperature(double temperature);
    bool isEnd(double temperature);
};

#endif // TEMPERATURESEGMENT_H
