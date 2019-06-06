#ifndef LT300_H
#define LT300_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <include/externals/nlohmann/json/json.hpp>
#include "device.h"
#include "terconData.h"
#include "shared.h"

using json = nlohmann::json;

class LT300 : public Device
{
    Q_OBJECT
public:
    LT300();
    ~LT300();

public slots:
    bool initialization();
    bool start();
    bool stop();
    bool connectDevice();
    bool disconnectDevice();
    bool setSetting(const json &parameters);

private slots:
    void readData();
    void extractData();
    void convertData(QByteArray strData);
    void askTimerTimeout();

private:
    bool verifyRecvData(const QByteArray & data);
    QTimer * askTimer;
    QSerialPort * port;
    QByteArray recvBytes;
    json channelArray;
};

#endif // LT300_H
