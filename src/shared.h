#ifndef SHARED_H
#define SHARED_H

namespace Shared
{
enum MessageLevel{information, warning, critical, empty, userMessage};
enum RegulatorMode {automatic, manual,stopCurrentTemperature, constVelocity, constValue};
enum FileType {logFile, dataFile, regulatorFurnaceFile, regulatorThermostatFile,
               regulatorUpHeaterFile, regulatorDownHeaterFile};
}
#endif // SHARED_H
