#ifndef FILTER_H
#define FILTER_H

#include <QObject>

class Filter : public QObject
{
    Q_OBJECT
public:
    explicit Filter(QObject *parent = 0);
    ~Filter();

signals:

public slots:
};


class TermocoupleConverter: public Filter
{

};

#endif // FILTER_H
