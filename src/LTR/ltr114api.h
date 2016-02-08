#ifndef LTR114API_H_
#define LTR114API_H_


#define LTR114_PLD_VER

#define LTR114_HDR_VER  0x105

#include <windows.h>
#include "ltrapi.h"
#include "ltrapitypes.h"

/*================================================================================================*/
#define LTR114_CLOCK                 (15000) /* тактовая частота модуля в кГц */
#define LTR114_ADC_DIVIDER           (1875)  //делитель частоты для АЦП
#define LTR114_MAX_CHANNEL           (16)    /* Максимальное число физических каналов */
#define LTR114_MAX_R_CHANNEL         (8)     /* Максимальное число физических каналов для измерения сопротивлений */
#define LTR114_MAX_LCHANNEL          (128)   /* Максимальное число логических каналов */

#define LTR114_MID                   (0x7272) //id модуля LTR114


#define LTR114_ADC_RANGEQNT          (3)     //кол-во диапазонов измерения напряжения
#define LTR114_R_RANGEQNT            (3)     //кол-во диапазонов измерения сопротивлений
#define LTR114_AUTOCALIBR_STEPS      (13)     //кол-во шагов для автокалибровки
#define LTR114_MAX_SCALE_VALUE     (8000000)  //код шкалы, соответствующий максимальному значению диапазона измерения

//флаги для функции LTR114_ProcessData
#define LTR114_PROCF_NONE           (0x00)
#define LTR114_PROCF_VALUE          (0x01)   //признак необходимисти перевода кода в физические величины
#define LTR114_PROCF_AVGR           (0x02)   //признак необходимости осреднения двух измерений - +I и -I
/*коды диапазонов напряжений*/
#define LTR114_URANGE_10        0
#define LTR114_URANGE_2         1
#define LTR114_URANGE_04        2
/*коды диапазонов сопротивлений*/
#define LTR114_RRANGE_400        0
#define LTR114_RRANGE_1200       1
#define LTR114_RRANGE_4000       2

/*режимы коррекции данных*/
#define LTR114_CORRECTION_MODE_NONE      0
#define LTR114_CORRECTION_MODE_INIT      1
#define LTR114_CORRECTION_MODE_AUTO      2

/*режимы синхронизации*/
#define LTR114_SYNCMODE_NONE            0
#define LTR114_SYNCMODE_INTERNAL        1
#define LTR114_SYNCMODE_MASTER          2
#define LTR114_SYNCMODE_EXTERNAL        4

/*режимы проверки входов*/
#define LTR114_CHECKMODE_X0Y0         1
#define LTR114_CHECKMODE_X5Y0         2
#define LTR114_CHECKMODE_X0Y5         4
#define LTR114_CHECKMODE_ALL          7

/*коды стандартных режимов измерения*/
#define LTR114_MEASMODE_U      0x00
#define LTR114_MEASMODE_R      0x20
#define LTR114_MEASMODE_NR     0x28
/*коды специальных режимов коммутации*/
#define LTR114_MEASMODE_NULL         0x10     //измерение собственного нуля
#define LTR114_MEASMODE_DAC12        0x11     //измерение DAC1 - DAC2
#define LTR114_MEASMODE_NDAC12       0x12 
#define LTR114_MEASMODE_NDAC12_CBR   0x38
#define LTR114_MEASMODE_DAC12_CBR    0x30

#define LTR114_MEASMODE_DAC12_INTR    0x91     //измерение DAC1 - DAC2 посередине интервала
#define LTR114_MEASMODE_NDAC12_INTR   0x92 
#define LTR114_MEASMODE_DAC12_INTR_CBR    0xB8     //измерение DAC1 - DAC2 посередине интервала
#define LTR114_MEASMODE_NDAC12_INTR_CBR   0xB0 
#define LTR114_MEASMODE_X0Y0              0x40
#define LTR114_MEASMODE_X5Y0              0x50
#define LTR114_MEASMODE_X0Y5              0x70


/*настройки модуля по-умолчанию*/
#define LTR114_DEF_DIVIDER            2
#define LTR114_DEF_INTERVAL           0
#define LTR114_DEF_OSR                0
#define LTR114_DEF_SYNC_MODE          LTR114_SYNCMODE_INTERNAL

/*коды тестов модуля LTR114*/
#define LTR114_TEST_INTERFACE         1   /*проверка интерфейса PC-LTR114*/
#define LTR114_TEST_DAC               2   /*проверка DAC*/
#define LTR114_TEST_DAC1_VALUE        3   /*передача тестового значения для DAC1*/
#define LTR114_TEST_DAC2_VALUE        4  /*передача тестового значения для DAC2*/
#define LTR114_TEST_SELF_CALIBR       5  /*проведение измерения модулем себя для калибровки*/

/*параметры подтверждения проверки интерфейса PC-LTR114*/
#define LTR114_TEST_INTERFACE_DATA_L (0x55)
#define LTR114_TEST_INTERFACE_DATA_H (0xAA)

//коды дополнительных возможностей
#define LTR114_FEATURES_STOPSW       0x01   //использовать режим автокалибровки
#define LTR114_FEATURES_THERM        0x02   //термометр
#define LTR114_FEATURES_CBR_DIS      0x04   //запрет начальной калибровки
#define LTR114_MANUAL_OSR            0x08   //ручная настройка OSR


/* Коды ошибок, возвращаемые функциями библиотеки */
#define LTR114_ERR_INVALID_DESCR        (-10000) /* указатель на описатель модуля равен NULL */
#define LTR114_ERR_INVALID_SYNCMODE     (-10001) /* недопустимый режим синхронизации модуля АЦП */
#define LTR114_ERR_INVALID_ADCLCHQNT    (-10002) /* недопустимое количество логических каналов */
#define LTR114_ERR_INVALID_ADCRATE      (-10003) /* недопустимое значение частоты дискретизации АЦП
                                                * модуля
                                                */
#define LTR114_ERR_GETFRAME             (-10004) /* ошибка получения кадра данных с АЦП */
#define LTR114_ERR_GETCFG               (-10005) /* ошибка чтения конфигурации */
#define LTR114_ERR_CFGDATA              (-10006) /* ошибка при получении конфигурации модуля */
#define LTR114_ERR_CFGSIGNATURE         (-10007) /* неверное значение первого байта конфигурационной
                                                * записи модуля
                                                */
#define LTR114_ERR_CFGCRC               (-10008) /* неверная контрольная сумма конфигурационной
                                                * записи
                                                */
#define LTR114_ERR_INVALID_ARRPOINTER   (-10009) /* указатель на массив равен NULL */
#define LTR114_ERR_ADCDATA_CHNUM        (-10010) /* неверный номер канала в массиве данных от АЦП */
#define LTR114_ERR_INVALID_CRATESN      (-10011) /* указатель на строку с серийным номером крейта
                                                * равен NULL
                                                */
#define LTR114_ERR_INVALID_SLOTNUM      (-10012) /* недопустимый номер слота в крейте */
#define LTR114_ERR_NOACK                (-10013) /* нет подтверждения от модуля */
#define LTR114_ERR_MODULEID             (-10014) /* попытка открытия модуля, отличного от LTR114 */
#define LTR114_ERR_INVALIDACK           (-10015) /* неверное подтверждение от модуля */
#define LTR114_ERR_ADCDATA_SLOTNUM      (-10016) /* неверный номер слота в данных от АЦП */
#define LTR114_ERR_ADCDATA_CNT          (-10017) /* неверный счетчик пакетов в данных от АЦП */
#define LTR114_ERR_INVALID_LCH          (-10018) /*неверный режим лог. канала*/
#define LTR114_ERR_CORRECTION_MODE      (-10019) /*неверный режим коррекции данных*/
#define LTR114_ERR_GET_PLD_VER          (-10020) /*ошибка при чтении версии ПЛИСа*/
#define LTR114_ERR_ALREADY_RUN          (-10021) /*ошибка при попытке запуска сбора данных, если он уже запущен*/
#define LTR114_ERR_MODULE_CLOSED        (-10022) /*ошибка при попытке работы с закрытым модулем*/


/*================================================================================================*/


/*================================================================================================*/
/* информация о модуле */
typedef struct
{
    CHAR Name[8];                          /* название модуля (строка) */
    CHAR Serial[16];                        /* серийный номер модуля (строка) */

    WORD VerMCU;                               /* версия ПО модуля (младший байт - минорная,
                                             * старший - мажорная
                                             */
	CHAR Date[14];                          /* дата создания ПО (строка) */
	BYTE VerPLD;                            //версия прошивки ПЛИС
    struct
    {
        float U[LTR114_ADC_RANGEQNT];       /*значения ИОН для диапазонов измерения напряжений*/             
        float I[LTR114_R_RANGEQNT];         /*значения токов для диапазонов измерения сопротивлений*/
        float UIntr[LTR114_ADC_RANGEQNT];   /*значение промежуточных напряжений*/
    } CbrCoef;                             /* заводские калибровочные коэффициенты */
} TINFO_LTR114, *PTINFO_LTR114;           

/*измеренные значения шкалы на этампе автокалибровки*/
typedef struct 
{
    INT Null;        //значение нуля                    
    INT Ref;         //значение +шкала
    INT NRef;       //значение -шкала
    INT Interm;
    INT NInterm;
} TSCALE_LTR114, *PTSCALE_LTR114; 

#define LTR114_SCALE_INTERVALS 3

typedef struct
{
    struct                                  
    {
        double Offset;                      /* смещение нуля */
        double Gain;                        /* масштабный коэффициент */
    } Coef[LTR114_SCALE_INTERVALS];         /*вычисленные на этапе автокалибровки значения Gain и Offset*/
    TSCALE_LTR114* TempScale;       /*массив временных измерений шкалы/нуля */
    TSCALE_LTR114 Index;           /*количество измерений в TempScale*/
    TSCALE_LTR114 LastVals;       /*последнее измерение*/

    INT HVal;
    INT LVal;
    
} TCBRINFO;                  /*информация для автокалибровки модуля по одному диапазону*/

typedef struct
{
    BYTE MeasMode;       /*режим измерения*/
    BYTE Channel;       /*физический канал*/
    BYTE Range;         /*диапазон измерения*/
} LTR114_LCHANNEL;            /*описатель логического канала*/


/* описатель состояния модуля LTR114 и указатель на него */
typedef struct
{
    INT size;                               /* размер структуры в байтах */
    TLTR Channel;                           /* описатель канала связи с модулем */

    TCBRINFO AutoCalibrInfo[LTR114_ADC_RANGEQNT];      /* данные для вычисления калибровочных коэф. для каждого диапазона */

    INT LChQnt;                              // количество активных логических каналов 
    LTR114_LCHANNEL LChTbl[LTR114_MAX_LCHANNEL];        // управляющая таблица с настройками логических каналов

    WORD Interval;                          //длина межкадрового интервала    

    BYTE SpecialFeatures;                   //дополнительные возможности модуля (подключение термометра, блокировка коммутации)
    BYTE AdcOsr;                             //значение передискр. АЦП - вычисляется в соответствии с частотой дискретизации
    BYTE SyncMode;                           /*режим синхронизации 
                                                  000 - нет синхронизации
                                                  001 - внутренняя синхронизация
                                                  010 - внутренняя синхронизация - ведущий
                                                  100 - внешняя синхронизация (ведомый)
                                                  */

    INT FreqDivider;                       // делитель частоты АЦП (2..8000)
                                           // частота дискретизации равна F = LTR114_CLOCK/(LTR114_ADC_DIVIDER*FreqDivider)

    INT FrameLength;                       //размер данных, передаваемых модулем за один кадр 
                                           //устанавливается после вызова LTR114_SetADC
    BOOL Active;                           //находится ли модуль в режиме сбора данных
    void* Reserve;
    TINFO_LTR114 ModuleInfo;                 /* информация о модуле LTR114 */

} TLTR114, *PTLTR114;

/*----------------------------------------------------------------------------------------------*/
/*вспомогательные макросы для определения параметров логического канала по слову в логической таблице*/
#define LTR114_TABLE_SWMODE(Word) (BYTE) (Word)
#define LTR114_TABLE_GAIN(Word)        (BYTE) ((Word >> 8) & 0x3)
#define LTR114_TABLE_SRC(Word)           (BYTE) ((Word >> 10) & 0x7)

//#define LTR114_CREATE_VCHANNEL(SwitchMode, GAIN, Channel, Src) (WORD)(((WORD)Src<<10) | ((WORD)GAIN) << 8 | SwitchMode | Channel)
#define LTR114_CREATE_LCHANNEL(LChannel, MeasMode, Channel, Scale) LChannel.MeasMode = MeasMode; LChannel.Channel = Channel; LChannel.Scale = Scale;//(WORD)(((WORD)Src<<10) | ((WORD)GAIN) << 8 | SwitchMode | Channel)






#define LTR114_FREQ(hltr) ((float)LTR114_CLOCK*1000/(LTR114_ADC_DIVIDER*hltr.FreqDivider))
#define LTR114_FREQ_CHANNEL(hltr) (LTR114_FREQ(hltr)/(hltr.LChQnt + hltr.Interval))


#define LTR114_START_CONFIG_ADDR     (0x80)   
#define LTR114_START_SERIAL_ADDR     (0x99)
#define LTR114_CONFIG_DATA_SIZE      (79)     //размер данных конфигурации




/*размер данных настройки модуля без лог. таблицы каналов в 2-х байтовых словах*/
#define LTR114_MODULE_MODE_BASE_SIZE  6 
/*размер данных настройки модуля в 2-х байтовых словах*/
#define LTR114_MODULE_MODE_SIZE      (LTR114_MODULE_MODE_BASE_SIZE + LTR114_MAX_LCHANNEL)  

/*================================================================================================*/

/*================================================================================================*/
INT LTR114_Init(PTLTR114 hnd);
INT LTR114_Open(PTLTR114 hnd, DWORD net_addr, WORD net_port, CHAR *crate_sn,
    INT slot_num);
INT LTR114_Close(PTLTR114 hnd);

INT LTR114_GetConfig(PTLTR114 hnd);
INT LTR114_Calibrate(PTLTR114 hnd);

INT LTR114_SetADC(PTLTR114 hnd);
INT LTR114_Start(PTLTR114 hnd);
INT LTR114_Stop(PTLTR114 hnd);

LPCSTR LTR114_GetErrorString(INT err);
INT LTR114_GetFrame(PTLTR114 hnd, DWORD *buf);

INT LTR114_Recv(PTLTR114 hnd, DWORD *data, DWORD *tmark, DWORD size, DWORD timeout);
INT LTR114_ProcessData(PTLTR114 hnd, DWORD *src, double *dest, INT *size, INT correction_mode, INT flags);
INT LTR114_ProcessDataTherm(PTLTR114 hnd, DWORD *src, double *dest, double *therm, INT *size, INT *tcnt, INT correction_mode, INT flags);

INT LTR114_CheckInputs(PTLTR114 hnd, INT ChannelsMask, INT CheckMode, double *res_data, INT *size);

INT LTR114_SetRef(PTLTR114 hnd, INT range, BOOL middle);
WORD LTR114_GetDllVer(void);

LTR114_LCHANNEL LTR114_CreateLChannel(INT MeasMode, INT Channel, INT Range);

INT LTR114_ConvertToValue(PTLTR114 hnd, double *data, INT size, INT correction_mode, INT flags);
INT LTR114_ProcessDataCb(PTLTR114 hnd, DWORD *src, double *dest, double *therm, INT *size, INT *tcnt, INT correction_mode, INT flags, INT (* cb_func)(PTLTR114 hnd, double* data));
/*================================================================================================*/

#endif                      /* #ifndef LTR11API_H_ */
