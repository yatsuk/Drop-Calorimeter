#include "device.h"
#include <QUuid>
#include <QDebug>

Device::~Device()
{

}

void Device::createDevice()
{
    parameters_["id"]= QUuid::createUuid().toString();
}

