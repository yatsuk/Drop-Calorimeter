#ifndef MIT_8_20_H
#define MIT_8_20_H

#include <QObject>
#include <QSerialPort>
#include <include/externals/nlohmann/json/json.hpp>
#include "device.h"
#include "terconData.h"
#include "shared.h"

using json = nlohmann::json;

class Mit_8_20 : public Device
{
    Q_OBJECT
public:
    Mit_8_20();
    ~Mit_8_20();

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

#endif // MIT_8_20_H
