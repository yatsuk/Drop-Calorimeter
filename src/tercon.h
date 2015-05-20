#ifndef TERCON_H
#define TERCON_H

#include <QObject>
#include <QSerialPort>
#include "terconData.h"
#include "shared.h"

class Tercon : public QObject
{
    Q_OBJECT
public:
    explicit Tercon(QObject *parent = 0);
    ~Tercon();
    void setPortName(const QString & portName);
    void setDeviceNumber(int number);
    bool startAck();
    bool stopAck();
    QString portName();

    QString description;



signals:
    void dataSend(TerconData data);
    void message(const QString & msg, Shared::MessageLevel msgLevel);

public slots:
private slots:
    void readData();
    void extractData();
    void convertData(QByteArray strData);

private:
    int deviceNumber;
    QSerialPort * port;
    QByteArray recvBytes;
};

#endif // TERCON_H
