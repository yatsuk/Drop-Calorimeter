#ifndef SHARED_H
#define SHARED_H

namespace Shared
{
enum MessageLevel{information, warning, critical, empty};
enum RegulatorMode {automatic, manual,programPower,stopCurrentTemperature, constVelocity, constValue};
enum FileType {logFile, dataFile, regulatorFurnaceFile, regulatorThermostatFile,
               regulatorUpHeaterFile, regulatorDownHeaterFile,
               mainSignalsFile,thermostatSignalsFile,calibrationHeaterFile};
}
#endif // SHARED_H
