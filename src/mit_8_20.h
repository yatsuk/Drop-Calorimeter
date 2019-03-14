#ifndef TERCON_H
#define TERCON_H

#include <QObject>
#include <QSerialPort>
#include <include/externals/nlohmann/json/json.hpp>
#include "device.h"
#include "terconData.h"
#include "shared.h"

using json = nlohmann::json;

class Tercon : public Device
{
    Q_OBJECT
public:
    Tercon();
    ~Tercon();

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

private:
    QSerialPort * port;
    QByteArray recvBytes;
    json channelArray;
};

#endif // TERCON_H
