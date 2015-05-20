#ifndef SIGNALSVIEW_H
#define SIGNALSVIEW_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QVector>
#include <QVBoxLayout>
#include "terconData.h"

class SignalsView : public QWidget
{
    Q_OBJECT
public:
    explicit SignalsView(QWidget *parent = 0);
    void addSignal(const QString & signalName);

    
signals:
    
public slots:   
    void addValue (TerconData terconData);

private:
    QVBoxLayout * layout;
    QVector <QLabel *> labels;

};

#endif // SIGNALSVIEW_H
