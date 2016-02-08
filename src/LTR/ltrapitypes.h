#ifndef __ltrapitypes__
#define __ltrapitypes__
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
//
#ifndef COMMENT_LENGTH
#define COMMENT_LENGTH 256
#endif
#ifndef ADC_CALIBRATION_NUMBER
#define ADC_CALIBRATION_NUMBER 256
#endif
#ifndef DAC_CALIBRATION_NUMBER
#define DAC_CALIBRATION_NUMBER 256
#endif
//
#pragma pack(4)

typedef struct __ERROR_STRING_DEF__
    {
    INT code;
    LPCSTR message;
    }
    T_ERROR_STRING_DEF;

// Информация о типе крейта
typedef struct _CRATE_INFO_
{
 BYTE CrateType;                                           // Тип крейта
 BYTE CrateInterface;                                      // Тип подключения крейта
} TCRATE_INFO;

// описание модуля
typedef struct _DESCRIPTION_MODULE_                        //
{                                                          //
  BYTE   CompanyName[16];                                  //
  BYTE   DeviceName[16];                                   // название изделия
  BYTE   SerialNumber[16];                                 // серийный номер изделия
  BYTE   Revision;                                         // ревизия изделия
  BYTE   Comment[COMMENT_LENGTH];                          //
} TDESCRIPTION_MODULE;                                     //
// описание процессора и програмного обеспечения
typedef struct _DESCRIPTION_CPU_                           //
{                                                          //
  BYTE   Active;                                           // флаг достоверности остальных полей структуры
  BYTE   Name[16];                                         // название
  double ClockRate;                                        //
  DWORD  FirmwareVersion;                                  //
  BYTE   Comment[COMMENT_LENGTH];                          //
} TDESCRIPTION_CPU;                                        //
// описание плис
typedef struct _DESCRIPTION_FPGA_                          //
{                                                          //
  BYTE   Active;                                           // флаг достоверности остальных полей структуры
  BYTE   Name[16];                                         // название
  double ClockRate;                                        //
  DWORD  FirmwareVersion;                                  //
  BYTE   Comment[COMMENT_LENGTH];                          //
} TDESCRIPTION_FPGA;                                       //
// описание ацп
typedef struct _DESCRIPTION_ADC_                           //
{                                                          //
  BYTE   Active;                                           // флаг достоверности остальных полей структуры
  BYTE   Name[16];                                         // название
  double Calibration[ADC_CALIBRATION_NUMBER];              // корректировочные коэффициенты
  BYTE   Comment[COMMENT_LENGTH];                          //
} TDESCRIPTION_ADC;                                        //
// описание цап
typedef struct _DESCRIPTION_DAC_                           //
{                                                          //
  BYTE   Active;                                           // флаг достоверности остальных полей структуры
  BYTE   Name[16];                                         // название
  double Calibration[DAC_CALIBRATION_NUMBER];              // корректировочные коэффициенты
  BYTE   Comment[COMMENT_LENGTH];                          //
} TDESCRIPTION_DAC;                                        //
// описание h-мезанинов
typedef struct _DESCRIPTION_MEZZANINE_                     //
{                                                          //
  BYTE   Active;                                           // флаг достоверности остальных полей структуры
  BYTE   Name[16];                                         // название изделия
  BYTE   SerialNumber[16];                                 // серийный номер изделия
  BYTE   Revision;                                         // ревизия изделия
  double Calibration[4];                                   // корректировочные коэффициенты
  BYTE   Comment[COMMENT_LENGTH];                          // комментарий
} TDESCRIPTION_MEZZANINE;                                   //
// описание цифрового вв
typedef struct _DESCRIPTION_DIGITAL_IO_                    //
{                                                          //
  BYTE   Active;                                           // флаг достоверности остальных полей структуры
  BYTE   Name[16];                                         // название ???????
  WORD   InChannels;                                       // число каналов
  WORD   OutChannels;                                      // число каналов
  BYTE   Comment[COMMENT_LENGTH];                          //
} TDESCRIPTION_DIGITAL_IO;                                 //
// описание интерфейсных модулей
typedef struct _DESCRIPTION_INTERFACE_                     //
{                                                          //
  BYTE   Active;                                           // флаг достоверности остальных полей структуры
  BYTE   Name[16];                                         // название
  BYTE   Comment[COMMENT_LENGTH];                          //
} TDESCRIPTION_INTERFACE;                                  //

// элемент списка IP-крейтов
typedef struct _IPCRATE_ENTRY_
    {
    DWORD ip_addr;                                          // IP адрес (host-endian)
    DWORD flags;                                            // флаги режимов (CRATE_IP_FLAG_...)
    CHAR serial_number[16];                                 // серийный номер (если крейт подключен)
    BYTE is_dynamic;                                        // 0 = задан пользователем, 1 = найден автоматически
    BYTE status;                                            // состояние (CRATE_IP_STATUS_...)
    }
    TIPCRATE_ENTRY;

#pragma pack()
//
#ifdef __cplusplus
}
#endif
#endif
