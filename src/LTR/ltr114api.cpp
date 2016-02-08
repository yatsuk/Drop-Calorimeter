/*
 * Библиотека для работы с модулем LTR114.
 * Обеспечивается выполнение базовых операций (старт, стоп, тест и т.д.) с модулем LTR114.
 * Для работы с модулем необходимо иметь запущенный сервер и открытый канал связи с модулем.
 * Взаимодействие с модулем осуществляется передачей и приемом команд через сервер.
 * Данные от модуля принимаются с помощью функции из общей библиотеки.
 */
#include <math.h>
#include <algorithm>

#define DLL_VER   0x105


#define LTR114_INTSTATUS_CALIBRATED (0x000001)


#define CBR_MODE_3T       1
#define CBR_MODE_3T_INTER  2
#define CBR_MODE_5T       3

#define CUR_CBR_MODE  CBR_MODE_5T

#define LTR114API_EXPORTS

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "ltr114api.h"
#include "ltrapi.h"
#include "crc.h"

typedef struct 
{
	BYTE status;
} TLTR114_INTERNAL_DATA, *PLTR114_INTERNAL_DATA;


#define LTR114_GAIN_10                (2)
#define LTR114_GAIN_2                 (0)
#define LTR114_GAIN_04                (1)


/*================================================================================================*/
/* Сообщения об ошибках (соответсвуют кодам ошибок, сообщение 0 - код -1000, 1 - -1001 и т.д.) */
const LPCSTR err_str[] =
{
    "Указатель на описатель модуля равен NULL",
    "Недопустимый режим синхронизации модуля",
    "Недопустимое количество логических каналов",
    "Недопустимое значение частоты дискретизации АЦП модуля",
    "Не удалось получить кадр данных с АЦП",
    "Не удалось получить конфигурацию модуля",
    "Ошибка при чтении конфигурационных данных",
    "Неверное значение первого байта конфигурационной записи модуля",
    "Ошибочная контрольная сумма конфигурационной записи",
    "Указатель на массив равен NULL",
    "Неверный номер канала в массиве полученных от АЦП данных",
    "Указатель на строку с серийным номером крейта равен NULL",
    "Недопустимый номер слота в крейте",
    "Нет подтверждения от модуля LTR114",
    "Модуль не является LTR114",
    "Неверное подтверждение от модуля LTR114",
    "Неверный номер слота в массиве полученных от АЦП данных",
    "Неверный счетчик пакетов в массиве полученных от АЦП данных",
    "Неверные параметры модуля: неправильно задан режим логического канала",
    "Неверный режим калибровки данных",
    "Ошибка при чтении версии ПЛИС'а модуля",
    "Сбор данных для модуля LTR114 уже запущен",
	"Связь с модулем закрыта"
};

/* диапазон возвращаемых кодов ошибок для LTR-114 */
static const int min_err_id = 10000;                   /* минимальный номер (модуль) */

/* Константы подтверждений от модуля LTR114 */
/* 
 * При контроле подтверждений также проверяются биты признаков команды (0x8000) и формата
 * подтверждения (6-битный).
 */
static const unsigned long ack_mask = 0x803F;         /* маска для выделения подтверждения */
static const unsigned long modulemode_ack = 0x8012;   /* установка режима модуля */
static const unsigned long start_ack = 0x8011;        /* подтверждение начала сбора данных */
static const unsigned long stop_ack = 0x8010;         /* переход в режим ожидания */
static const unsigned long test_interface_ack = 0x8013; /*подтверждение перехода в тестовый режим */
static const unsigned long ver_PLD_ack = 0x8014; /*подтверждение перехода в тестовый режим */

/* Передаваемые в модуль команды */
static const unsigned dummy_cmd = 0x00;               /* неиспользуемая модулем команда */
static const unsigned getconfig_cmd = 0x0C;           /* запрос конфигурационных данных */
static const unsigned modulemode_cmd = 0x18;          /* установка режимов АЦП */
static const unsigned modulemode_first_cmd = 0x19;     /* установка режимов АЦП  - первое слово*/
static const unsigned start_cmd = 0x0A;               /* запуск сбора данных */
static const unsigned stop_cmd = 0x05;                /* переход в режим ожидания */
static const unsigned test_cmd = 0x06;                /* переход в тестовый режим */

/* Тайм-ауты в миллисекундах */
static const DWORD ack_tm_out = 6000;                 /* на прием подтверждения */
static const DWORD send_tm_out = 2000;                /* на передачу */
static const DWORD cmd_tm_out  = 10;

static const DWORD recv_tm_out  = 1000;

/* Константы, связанные с заданием частоты дискретизации */
static const long max_divider = 8000;                /* наибольшее значение делителя */
static const long min_divider = 2;                    /* наименьшее значение делителя */
/* Возможные значения пределителя частоты модуля, отсортированные по возрастанию */
static const int prescalers[] = {1, 8, 64, 256, 1024};

static double theScalesVal[] = {10, 2, 0.4}; 
static int theRScalesVal[] = {150, 400, 1000, 2500};

static const int cfg_signature = 0x5A;
/*================================================================================================*/

/*================================================================================================*/

static int eval_parity(DWORD cmd);
static DWORD f_elapsed_time(DWORD start_time);
static DWORD fill_command(WORD cmd, WORD param);
//static INT start_ltr11(PTLTR11 hnd, int frame);
/*================================================================================================*/

/*================================================================================================*/

#define T_AVR 0.03 //время данное AVR на обработку преобразования

int powr (int n, int p)
{
    int i;
    int res = 1;
    for (i=0; i < p; i++)
        res *= n;
    return res;
}
/*------------------------------------------------------------------------------------------------*/
BYTE eval_OSR(double fx)
{
    /* Процедура для рассчета значения OSR для заданной частоты
     * Параметры:
     *   fx - значение частоты в Гц
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Значение OSR, соотв. данной частоте.
     */
    double Tadc = 1000/fx;
    double Tconv = Tadc+1;
    BYTE osr = 0x8F;
    Tconv = ((double)40*32768 + 170)/(7500); 

    if ((Tadc - Tconv) >= T_AVR)
        return osr;

    osr = 0xF;
    Tconv = ((double)40*32768 + 170)/(15*1000);

    while (Tadc - Tconv < T_AVR)
    {
        osr = (osr==0xF) ? 9:osr-1;
        Tconv = ((double)40*32*powr(2,osr) + 170)/(15*1000);
    }
    return osr;
}

/*------------------------------------------------------------------------------------------------*/
static int eval_parity
    (
    DWORD cmd
    )
    {

    /*
    * Вычисление четности команды
    * ОПИСАНИЕ
    *   Вычисляет бит четности для команды по битам 22-0 для контроля четности 
    * ПАРАМЕТРЫ
    *   cmd - команда;
    * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
    *   четность (0 или 1)
    */
    unsigned long index;
    int parity;

    parity = 0;
    for (index = 0x01UL; (index <= 0x10UL); index <<= 1)
        {
        parity ^= (cmd & index) != 0;
        }

    for (index = 0x80000000UL; (index >= 0x010000UL); index >>= 1)
        {
        parity ^= (cmd & index) != 0;
        }
    return parity;
    }
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
static DWORD f_elapsed_time
    (
    DWORD start_time
    )
     /*
     * Вычисление времени, прошедшего с момента start_time
     * ПАРАМЕТРЫ
     *   start_time - момент времени, относительно которового считается временной интервал;
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   прошедшее время (в мс)
     */

    {
    DWORD elapsed_time;
    DWORD sys_time;


    sys_time = GetTickCount();
    if (sys_time >= start_time) elapsed_time = sys_time - start_time;
    else                        elapsed_time = (65535 - (start_time - sys_time)) + 1;

    return elapsed_time;
    }
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/

static DWORD fill_command
    (
    WORD cmd,
    WORD param
    )
     /*
     * Заполнение 4-х байтового слова команды в соответствии с форматом команды по коду команды и ее параметрам
     * ПАРАМЕТРЫ
     *   cmd - код команды;
     *   param - 2-х байтовый парметр команды
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   32-разр. слово посылаемой команды
     */

    {
    DWORD ret_cmd;
    ret_cmd = 0x80C0U | (DWORD)param << 16 | (DWORD)cmd & 0x1F;
    ret_cmd |= eval_parity(ret_cmd) << 5;  //подсчет четности команды (бит P)

    return ret_cmd;
    }
/*------------------------------------------------------------------------------------------------*/

#define IS_R_SWMODE(w) ((LTR114_TABLE_SWMODE(w) & 0xF0) == 0x20)

#define IS_MODE_R(mode) ((mode& 0xF0) == 0x20)
#define IS_MODE_CHANNEL(mode) ((IS_MODE_R(mode))||(mode==LTR114_MEASMODE_U))
#define IS_TEST_MODE(mode) ((mode == LTR114_MEASMODE_X0Y0)||(mode==LTR114_MEASMODE_X5Y0)||(mode==LTR114_MEASMODE_X0Y5))

#define INVALID_CH 0xFFFF


/*вспомагательная функция. Позволяет поулчить по параметрам в структуре
  LCHANNEL код описателя канала типа WORD (в формате, принимаемом модулем)
  При неправильных параметрах возвращает INVALID_CH*/
WORD create_table_lch(LTR114_LCHANNEL ch)
{
    WORD out = 0;

    //установка диапазона
    if (IS_MODE_R(ch.MeasMode))
    {
        if (ch.Range >= LTR114_R_RANGEQNT)
            return INVALID_CH;
        else
            out |= (WORD)ch.Range<<10;

        if (ch.Range == 0)
            out |= 0x40;    //1 для диапазона 0-400
    }
    else
    {
        switch (ch.Range)
        {
            case LTR114_URANGE_10: out |= LTR114_GAIN_10<<8; break;
            case LTR114_URANGE_2: out |= LTR114_GAIN_2<<8; break;
            case LTR114_URANGE_04: out |= LTR114_GAIN_04<<8; break;
            default: return INVALID_CH;
        }
    }
    
    //установка номера канала
    if (IS_MODE_R(ch.MeasMode))
    {
        if ((ch.Channel < 0)||(ch.Channel > 7))  //для сопро
            return INVALID_CH;
        else out |= ch.Channel;
    }

    if ((ch.MeasMode == LTR114_MEASMODE_U)||(IS_TEST_MODE(ch.MeasMode))) 
    {
        if ((ch.Channel < 0) || (ch.Channel > 15))
            return INVALID_CH;
        else out |= ch.Channel;
    }

    if ((ch.MeasMode==LTR114_MEASMODE_DAC12_CBR) || (ch.MeasMode==LTR114_MEASMODE_NDAC12_CBR)
        || (ch.MeasMode==LTR114_MEASMODE_DAC12_INTR_CBR) || (ch.MeasMode==LTR114_MEASMODE_NDAC12_INTR_CBR))
    {
        if ((ch.Channel < 8) || (ch.Channel > 15))
            return INVALID_CH;
        out |= (ch.Channel-8);
    }
    out |= ch.MeasMode;
    return out;
}






/*------------------------------------------------------------------------------------------------*/
static INT start_ltr114
    (
    PTLTR114 hnd,
    int frame
    )
    {
    /*
     * Запуск сбора данных модулем LTR114.
     * ОПИСАНИЕ
     *   Выполняется передача команды запуска преобразования в заданном режиме и ожидается (с
     *   тайм-аутом) подтверждение команды.
     * ПАРАМЕТРЫ
     *   hnd   - указатель на описатель модуля;
     *   frame - признак необходимости запуска АЦП на сбор одного кадра.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */

    DWORD ack;
    DWORD cmd;
	INT res;
    INT mode = 0;
    long n;
    INT ret_val = LTR_OK;


    if (hnd == NULL)
        return LTR114_ERR_INVALID_DESCR;
    if (hnd->Active)
        return LTR114_ERR_ALREADY_RUN;
	if (hnd->Reserve == NULL)
		return LTR114_ERR_MODULE_CLOSED;

	//запуск калибровки, если не проводилась и не была запрещена
	if (!(((PLTR114_INTERNAL_DATA)hnd->Reserve)->status & LTR114_INTSTATUS_CALIBRATED) && 
		!(hnd->SpecialFeatures & LTR114_FEATURES_CBR_DIS))
	{
		res = LTR114_Calibrate(hnd);
		if (res!=LTR_OK)
			return res;
	}


    //filter_data_clear(hnd);
    if (frame)
        mode |= 0x8000U;
    cmd = fill_command(start_cmd, mode);


    //посылка команды RUN
    if ((n = LTR_Send(&hnd->Channel, &cmd, 1, send_tm_out)) < 0)
        return n;
    if (n != 1)
        return LTR_ERROR_SEND;
    
    //прием подтверждения
    if ((n = LTR_Recv(&hnd->Channel, &ack, NULL, 1, ack_tm_out)) < 0)
        return n;
    if (n < 1)
        return LTR114_ERR_NOACK;
    //проверка подтверждения
    ack &= ack_mask;
    if (ack != start_ack)
        return LTR114_ERR_INVALIDACK;

    hnd->Active = TRUE;

    return ret_val;
    }

/*------------------------------------------------------------------------------------------------*/
INT LTR114_Close
    (
    PTLTR114 hnd
    )
    {
    /*
     * Завершение работы с модулем LTR114.
     * ОПИСАНИЕ
     *   Закрывается канал связи с модулем.
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель модуля.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. файл заголовка).
     */

    INT ret_val = LTR_OK;


    if (hnd == NULL)
    {
        ret_val = LTR114_ERR_INVALID_DESCR;
    }
    else
    {
        DWORD cmd = LTR010CMD_STOP;  //останов модуля - посылка команды STOP
        int i;

        LTR_Send(&hnd->Channel, &cmd, 1, send_tm_out);    

        for (i=0; i < LTR114_ADC_RANGEQNT; i++)  /*освобождение временной памяти, если была занята*/
        {
            if (hnd->AutoCalibrInfo[i].TempScale!=NULL)
            {
                free(hnd->AutoCalibrInfo[i].TempScale);
                hnd->AutoCalibrInfo[i].TempScale = NULL;
            }
        }

		if (hnd->Reserve != NULL)
			free(hnd->Reserve);
		hnd->Reserve = NULL;

        /*        if (hnd->FilterData!=NULL)
        {
            free(hnd->FilterData);
            hnd->FilterData = NULL;
        }*/
        ret_val = LTR_Close(&hnd->Channel);
    }

    hnd->Active = FALSE;

    return ret_val; 
    }
/*------------------------------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------------------------*/
INT LTR114_GetConfig
    (
    PTLTR114 hnd
    )
{
    /*
     * Получение данных о конфигурации модуля LTR114.
     * ОПИСАНИЕ
     *   В модуль передается команда выдачи данных конфигурационных данных и ожидается прием
     *   этих данных и заполнение соответствующих полей в структуре, описывающей модуль.
     *   Формат принимаемого массива (после преобразования из 2-битного вида в байтный):
     *     Байт                   Содержимое
     *      0            сигнатура конфигурационной записи
     *      1-2          версия
     *      3-16         дата создания ПО
     *      17-24        наименование модуля
     *      25-40        серийный номер модуля
     *      41-44        Точное значение ИОН
     *      45-48        Точное значение ЦАП1 – ЦАП2  в точке 2 В 
     *      49-52        Точное значение ЦАП1 – ЦАП2  в точке 0.4 В 
     *      53-56        Опорное значение тока для первого диапазона измерения сопротивлений
     *      57-60        --//-- для второго диапазона
     *      61-64        --//-- для третьего диапазона
     *      65-66        контрольная сумма
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель состояния модуля.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */

    BYTE cfg_arr[LTR114_CONFIG_DATA_SIZE];
    int i;
    DWORD rd_buf[LTR114_CONFIG_DATA_SIZE * 4];          /* один байт передается 4-мя командами */
    INT ret_val = LTR_OK;
    double ver;

    DWORD cmd;
    long n;

    int j;
    DWORD *pbuf = rd_buf;
    float *pcfg;

    if (hnd == NULL)
        return LTR114_ERR_INVALID_DESCR;
    
    if (hnd->Active)
        return LTR114_ERR_ALREADY_RUN;

    cmd = fill_command(getconfig_cmd, 0);
    if ((n = LTR_Send(&hnd->Channel, &cmd, 1, send_tm_out)) < 0)  //посылка модулю команды GET_CONFIG
        return n;
            
    if (n != 1)
        return LTR_ERROR_SEND;
           
    if ((n = LTR_Recv(&hnd->Channel, rd_buf, NULL, LTR114_CONFIG_DATA_SIZE * 4,
        LTR114_CONFIG_DATA_SIZE * 4 * cmd_tm_out + ack_tm_out)) < 0)  //прием данных конфигурации
            return n;
           
    if (n != LTR114_CONFIG_DATA_SIZE * 4)           /* массив данных принят не полностью */
        return LTR114_ERR_GETCFG;
            
    /* преобразование принятых данных в последовательность байт с проверкой */
    if ((rd_buf[0] & 0x80FCUL) != 0x80DCUL)   /* проверка первого байта */
        return LTR114_ERR_CFGDATA;
        
    rd_buf[0] &= ~0x10UL;                 /* сброс признака первого слова */
    for (i = 0; ((i < LTR114_CONFIG_DATA_SIZE) && (ret_val == LTR_OK)); i++)
    {
        cfg_arr[i] = 0;
        for (j = 6; ((j >= 0) && (ret_val == LTR_OK)); j-= 2)
        {
            if ((*pbuf & 0x803CUL) != 0x800CUL)
            {
                ret_val = LTR114_ERR_CFGDATA;
            }
            else
            {
                cfg_arr[i] |= (*pbuf & 0x03UL) << j;
            }
            pbuf++;
        }
    }

    /* занесение принятых данных в структуру с конфигурацией */
    if (ret_val != LTR_OK)
        return ret_val;
       
    if (cfg_arr[0] != cfg_signature)      //проверка сигнатуры
       return LTR114_ERR_CFGSIGNATURE;
       
    if (eval_crc16(0, cfg_arr, LTR114_CONFIG_DATA_SIZE - 2) !=
        (((unsigned short)cfg_arr[LTR114_CONFIG_DATA_SIZE - 2] << 8 |
        (unsigned short)cfg_arr[LTR114_CONFIG_DATA_SIZE - 1]) & 0xFFFFU)  //проверка контрольной суммы
        )
        return LTR114_ERR_CFGCRC;
       
    hnd->ModuleInfo.VerMCU = (unsigned)cfg_arr[1] << 8 | (unsigned)cfg_arr[2];
    (void)memcpy((char *)hnd->ModuleInfo.Date, (char *)(cfg_arr + 3), 14);
    (void)memcpy((char *)hnd->ModuleInfo.Name, (char *)(cfg_arr + 17), 8);
    (void)memcpy((char *)hnd->ModuleInfo.Serial, (char *)(cfg_arr + 25), 16);
       
    pcfg = (float *)(cfg_arr + 41);

    for (i = 0; i < LTR114_ADC_RANGEQNT; i++)
       hnd->ModuleInfo.CbrCoef.U[i] = *pcfg++;
    for (i = 0; (i < LTR114_R_RANGEQNT); i++)
    {
     hnd->ModuleInfo.CbrCoef.I[i] = *pcfg++;
    }
    for (i = 0; i < LTR114_ADC_RANGEQNT; i++)
       hnd->ModuleInfo.CbrCoef.UIntr[i] = *pcfg++;

    //ver = conver_ver(hnd->ModuleInfo.VerMCU);
    if (hnd->ModuleInfo.VerMCU >= 0x103)
    {
        cmd = fill_command(getconfig_cmd, 1);
        if ((n = LTR_Send(&hnd->Channel, &cmd, 1, send_tm_out)) < 0)  //посылка модулю команды GET_CONFIG
            return n;


        if (n != 1)
            return LTR_ERROR_SEND;
               
        if ((n = LTR_Recv(&hnd->Channel, rd_buf, NULL, 1,
             cmd_tm_out + ack_tm_out)) < 0)  //прием данных конфигурации
                return n;
               
        if (n != 1)           /* массив данных принят не полностью */
            return LTR114_ERR_GET_PLD_VER;
        if ((rd_buf[0] & ack_mask)!=ver_PLD_ack)
            return LTR114_ERR_GET_PLD_VER;
        hnd->ModuleInfo.VerPLD = (rd_buf[0] & 0xFF0000)>>16;
    }

    return ret_val;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
LPCSTR LTR114_GetErrorString
    (
    INT err
    )
{
    /*
     * Определение строки с сообшением об ошибке, соответсвующей заданному коду ошибки.
     * ОПИСАНИЕ
     * ПАРАМЕТРЫ
     *   err - код ошибки (должен быть отрицительным), возвращается функциями работы с LTR114.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Указатель на строку с сообщением об ошибке, соответсвующему заданному коду.
     *   NULL - неизвестный код ошибки.
     */
     
    const int err_qnt = sizeof err_str / sizeof err_str[0];
    LPCSTR ret_val = NULL;

    
    if ((err > -min_err_id) || (err <= -(min_err_id + err_qnt)))
        {
        /* неизвестный код ошибки (за пределами диапазона кодов обрабатываемых ошибок) */
        /* строка с ошибкой формируется в библиотеке нижнего уровня */
        ret_val = LTR_GetErrorString(err);
        }
    else
        {
        /* получение указателя на строку из массива сообщений об ошибках LTR-114 */
        ret_val = err_str[-err - min_err_id];
        }

    return ret_val;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
INT LTR114_GetFrame
    (
    PTLTR114 hnd,
    DWORD *buf
    )
{
    /*
     * Сбор одного кадра с АЦП модуля LTR11.
     * ОПИСАНИЕ
     *   Модуль LTR11 переводится в режим сбора одного кадра данных.
     *   Принятый поток данных от модуля заносится в буфер. Окончание сбора данных производится
     *   по приему подтверждения перехода в режим ожидания или тайм-ауту (длительность тайм-аута
     *   зависит от частоты дискретизации).
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель состояния модуля;
     *   buf - указатель на буфер, в который будут записываться принимаемые от модуля данные.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Положительное число или 0 - количество байт в принятом кадре.
     *   Отрицательное число       - код ошибки (см. заголовочный файл).
     */
    DWORD cmd;
    DWORD rd_buf[LTR114_MAX_LCHANNEL*2 + 2];
    INT ret_val = 0, res;
    double adcrate;

    if (hnd == NULL)
        return LTR114_ERR_INVALID_DESCR;
    if (hnd->Active)
        return LTR114_ERR_ALREADY_RUN;
	if (hnd->Reserve==NULL)
		return LTR114_ERR_MODULE_CLOSED;
        
    if ((hnd->LChQnt <= 0) || (LTR114_MAX_LCHANNEL < hnd->LChQnt))
        return LTR114_ERR_INVALID_ADCLCHQNT;
    
    if ((adcrate = LTR114_FREQ((*hnd))) <= 0.0)
       return LTR114_ERR_INVALID_ADCRATE;
    
	//запуск калибровки, если не была произведена
	if (!(((PLTR114_INTERNAL_DATA)hnd->Reserve)->status & LTR114_INTSTATUS_CALIBRATED) && 
		!(hnd->SpecialFeatures & LTR114_FEATURES_CBR_DIS))
	{
		res = LTR114_Calibrate(hnd);
		if (res!=LTR_OK)
			return res;
	}


   // hnd->Active = TRUE;
    if ((ret_val = start_ltr114(hnd, 1)) == LTR_OK)
    {
        long n;
        DWORD *pbuf;
        DWORD tick;
        DWORD tm_out;

        tick = GetTickCount();
        /* вычисление значения тайм-аута сбора данных для данной частоты дискретизации */
        tm_out = (DWORD)((double)1 * hnd->FrameLength / adcrate + ack_tm_out + 0.5);
        pbuf = rd_buf;
        do
        {
            if ((n = LTR_Recv(&hnd->Channel, pbuf, NULL, 2*hnd->LChQnt + 1, tm_out)) < 0)
            {
                ret_val = n;
            }
            else if (n > 0)
            {
                unsigned a;

                /* проверка окончания сбора кадра (по приходу подтверждения) */
               pbuf += n;
                a = pbuf[-1] & ack_mask;
                if (a == stop_ack)                   /* получен один кадр данных */
                {
                    ret_val = pbuf - rd_buf;
                    if (--ret_val <= 0)
                    {
                        ret_val = LTR114_ERR_GETFRAME;
                    }
                    else
                    {
                        (void)memcpy(buf, rd_buf, ret_val * sizeof(DWORD));
                    }
                }
                else if (n < hnd->LChQnt + 1)        /* нет подтверждения окончания сбора данных */
                {
                    ret_val = LTR114_ERR_GETFRAME;
                }
             }
        } while ((f_elapsed_time(tick) <= tm_out) && (ret_val == LTR_OK) && (n > 0));
    }

    cmd = LTR010CMD_STOP;  //останов модуля - посылка команды STOP
        
    LTR_Send(&hnd->Channel, &cmd, 1, send_tm_out); 

    hnd->Active = FALSE;

    return ret_val;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
INT LTR114_Init
    (
    PTLTR114 hnd
    )
{
    /*
     * Инициализация описателя модуля LTR114.
     * ОПИСАНИЕ
     *   Полям структуры описателя модуля присваиваются значения "по-умолчанию".
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель модуля.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */

    INT ret_val = LTR_OK;

    if (hnd == NULL)
     return LTR114_ERR_INVALID_DESCR;
    hnd->size = sizeof(TLTR114);

    if ((ret_val = LTR_Init(&hnd->Channel)) == LTR_OK)
    {
        const char date_def[14]="";
        int i,j;
        const char name_def[8]  = "LTR114";
        const char serial_def[16] = "";

        hnd->LChQnt = 1;               //используется один канал
        //все записи в таблице логических каналов обнуляются
        for (i = 0; (i < LTR114_MAX_LCHANNEL); i++)
        {
            hnd->LChTbl[i].Channel = 0; hnd->LChTbl[i].Range =0; hnd->LChTbl[i].MeasMode=0;
        }

        //присвоение параметров по-умолчанию
        hnd->FreqDivider = LTR114_DEF_DIVIDER;         //частота 4000 Гц      
        hnd->Interval = LTR114_DEF_INTERVAL;           //нет межкадровой задержки
        hnd->SpecialFeatures = 0;                      //все флаги отключены
        hnd->AdcOsr = LTR114_DEF_OSR;                  //OSR определяется по умолчанию
        hnd->SyncMode = LTR114_DEF_SYNC_MODE;          //внутренняя синхронизация
        hnd->Active = FALSE;


        hnd->ModuleInfo.VerMCU = 0;
        hnd->ModuleInfo.VerPLD = 0;
        (void)strcpy((char *)hnd->ModuleInfo.Date, date_def);
        (void)strcpy((char *)hnd->ModuleInfo.Name, name_def);
        (void)strcpy((char *)hnd->ModuleInfo.Serial, serial_def);

        for (i=0; i < LTR114_ADC_RANGEQNT; i++)
        {
            hnd->AutoCalibrInfo[i].TempScale=NULL;
            for (j=0;j<LTR114_SCALE_INTERVALS; j++)
            {
                hnd->AutoCalibrInfo[i].Coef[j].Gain = 1;
                hnd->AutoCalibrInfo[i].Coef[j].Offset = 0;
            }
        }

		hnd->Reserve = NULL;

//        hnd->FilterData = NULL;
    }  //if ((ret_val = LTR_Init(&hnd->Channel)) == LTR_OK)

    return ret_val;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
INT LTR114_Open
(
 PTLTR114 hnd,
 DWORD net_addr,
 WORD net_port,
 CHAR *crate_sn,
 INT slot_num
 )
{
    /*
    * Начало работы с модулем LTR114.
    * ОПИСАНИЕ
    *   Открывается канал связи с модулем и выполняется аппаратный сброс модуля.
    * ПАРАМЕТРЫ
    *   hnd      - указатель на описатель модуля;
    *   net_addr - сетевой адрес клиента (вызывающего функцию приложения) в формате (hex):
    *              MsbAABBCCDDLsb - AA.BB.CC.DD;
    *   net_port - сетевой порт сервера;
    *   crate_sn - серийный номер крейта, в котором установлен модуль;
    *   slot_num - номер слота крейта, в который установлен модуль.
    * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
    *   Код ошибки (см. заголовочный файл).
    */

    INT ret_val = LTR_OK;
    int WarningRes = LTR_OK;


    if (hnd == NULL)
    {
        ret_val = LTR114_ERR_INVALID_DESCR;
    }
    else if (crate_sn == NULL)
    {
        ret_val = LTR114_ERR_INVALID_CRATESN;
    }
    else if ((slot_num < 1) || (MODULE_MAX < slot_num))
    {
        ret_val = LTR114_ERR_INVALID_SLOTNUM;
    }
    else
    {
        //if ((WarningRes = LTR_IsOpened(&hnd->Channel)) == LTR_OK)
        //    LTR_Close(&hnd->Channel);

		if (hnd->Reserve != NULL)
            free(hnd->Reserve);
        hnd->Reserve = malloc (sizeof(TLTR114_INTERNAL_DATA));
		((PLTR114_INTERNAL_DATA)hnd->Reserve)->status = 0;
		
        hnd->Channel.saddr = net_addr;
        hnd->Channel.sport = net_port;
        (void)strncpy((char *)hnd->Channel.csn,(char *)crate_sn, 16);
        hnd->Channel.cc = slot_num;                   /* тип канала - связь с модулем */

        WarningRes = LTR_Open(&hnd->Channel);
        if(WarningRes==LTR_OK||WarningRes==LTR_WARNING_MODULE_IN_USE) 
            ret_val=LTR_OK;
        else ret_val = WarningRes;

        if (ret_val == LTR_OK)
        {
            DWORD cmd[3];
            long n;
            /*Перед запуском модуля необходимо послать последовательность команд STOP - RESET - STOP*/
            cmd[0] = LTR010CMD_STOP;
            cmd[1] = LTR010CMD_RESET;
            cmd[2] = LTR010CMD_STOP;
            if ((n = LTR_Send(&hnd->Channel, cmd, 3, send_tm_out)) < 0)
            {
                ret_val = n;
            }
            else if (n != 3)
           {
               ret_val = LTR_ERROR_SEND;
           }
           else
           {
               DWORD rd_buf;  //прием поддверждения от модуля на команду RESET
               if ((n = LTR_Recv(&hnd->Channel, &rd_buf, NULL, 1, ack_tm_out)) < 0)
               {
                   ret_val = n;
               }
               else if (n == 0)
               {
                   ret_val = LTR114_ERR_NOACK;          
               }
               else if ((rd_buf & 0xF0FF) == LTR010CMD_RESET)  //проверка правильности подтверждения
               {
                   if (((rd_buf >> 16) & 0xFFFFUL) != LTR114_MID)
                   {
                       ret_val = LTR114_ERR_MODULEID;
                   }
                   else
                   {
                        ret_val = LTR_OK;
                   }
                }
            } /*else @else if (n != 3)@*/
            Sleep(200); // ждем, пока модуль не выйдет на рабочий режим

        } /*if ((ret_val = LTR_Open(&hnd->Channel)) == LTR_OK)*/
    } /*else @else if ((slot_num < min_slotnum) || (max_slotnum < slot_num))@*/    

    if(WarningRes==LTR_WARNING_MODULE_IN_USE&&ret_val==LTR_OK) ret_val=WarningRes;
    return ret_val;
}
/*------------------------------------------------------------------------------------------------*/

/*макросы, для разбора принятых данных (см. формат передаваемых данных)*/
#define RECV_IS_FIRST_WORD(RecWord) !(RecWord&0x8)                   //является ли это слово первым?
#define RECV0_SWITCH_MODE(RecWord)    (BYTE)((RecWord>>24)&0xFF)     //получение режима коммутации из первого слова
#define RECV0_GAIN(RecWord)           (BYTE)((RecWord&0x3))          //получение значения диапазона из первого слова
#define RECV_COUNT(RecWord)          (BYTE)((RecWord>>4)&0xF)        //получение значения счетчика (из любого слова)
#define RECV1_N(RecWord)              (BYTE)((RecWord&0x7))          //получение номера источника из второго слова
#define RECV0_DATA2(RecWord)          (BYTE)((RecWord>>16)&0xFF)     //получение старшего байта данных из первого слова
#define RECV1_DATA10(RecWord)          (WORD)((RecWord>>16)&0xFFFF)  //получение двух младших байт данных из второго слова

//является ли режим коммутации этого слова одним из режимов для автокалибровки?
//#define IS_AUTO_CALIBR_MODE(RecWord)   \
//   ((RECV0_SWITCH_MODE(RecWord) >= LTR114_SWMODE_NULL) && ((RECV0_SWITCH_MODE(RecWord)) <= LTR114_SWMODE_REF)) 
#define IS_AUTO_CALIBR_MODE(RecWord)   \
   ((RECV0_SWITCH_MODE(RecWord) == LTR114_MEASMODE_DAC12) || \
    (RECV0_SWITCH_MODE(RecWord) == LTR114_MEASMODE_NDAC12) || \
    (RECV0_SWITCH_MODE(RecWord) == LTR114_MEASMODE_NDAC12_CBR) || \
    (RECV0_SWITCH_MODE(RecWord) == LTR114_MEASMODE_DAC12_CBR)) || \
    (RECV0_SWITCH_MODE(RecWord) == LTR114_MEASMODE_DAC12_INTR) || \
    (RECV0_SWITCH_MODE(RecWord) == LTR114_MEASMODE_NDAC12_INTR) || \
    (RECV0_SWITCH_MODE(RecWord) == LTR114_MEASMODE_DAC12_INTR_CBR) || \
    (RECV0_SWITCH_MODE(RecWord) == LTR114_MEASMODE_NDAC12_INTR_CBR) || \
    (RECV0_SWITCH_MODE(RecWord) == LTR114_MEASMODE_NULL)



#define IS_THERM(RecWord) ((RecWord&0xE) == 4)  //является ли это слово словом данных от термодатчика?

#define IS_THERM_VALID(RecWord) ((RecWord&0xF) == 5)  //является ли измерение термодатчика действительным?

#define LTR114_CYCLE_CNT_MODE 0xD //модуль циклического счетчика

//макрос для перехода к след. значению циклического счетчика
#define CYCLE_COUNTER_INC(cnt) \
  if (cnt == (LTR114_CYCLE_CNT_MODE-1)) \
    cnt = 0;\
  else\
    (cnt)++

//перевод значения шкалы из 3-х байтового числа в формате DWORD в 4-х байтовое INT со знаком
#define SCALE_TO_INT(scale) (scale & 0x800000)? (INT)(scale | 0xFF000000):(INT)scale

int coef_calc(PTLTR114 hnd, INT gain)
{
    /* Вспомогательная процедура рассчета калибровочных коэффициентов (GAIN, OFFSET)
     * по последним результатом измерения модуля собственного нуля, +шкалы, - шкалы
     * (значения берутся из hnd->AutoCalibrInfo[gain].TempScale[0])
     * путем построения прямой с минимальным среднеквадратичным отклонением
     * в режиме автокалибровки
     *
     * ПАРАМЕТРЫ
     *   hnd      - указатель на описатель модуля;
     *   gain     - измеряемый диапазон
    * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
    *   Код ошибки (см. заголовочный файл).
    */
    if (gain != LTR114_URANGE_10)
    {
        double x1 = hnd->AutoCalibrInfo[gain].TempScale[0].NRef;
        double x2 = hnd->AutoCalibrInfo[gain].TempScale[0].NInterm;
        double x3 = hnd->AutoCalibrInfo[gain].TempScale[0].Null;
        double x4 = hnd->AutoCalibrInfo[gain].TempScale[0].Interm;
        double x5 = hnd->AutoCalibrInfo[gain].TempScale[0].Ref;

        double u =  (double)hnd->ModuleInfo.CbrCoef.U[gain]*LTR114_MAX_SCALE_VALUE/theScalesVal[gain];
        double u_int =  (double)hnd->ModuleInfo.CbrCoef.UIntr[gain]*LTR114_MAX_SCALE_VALUE/theScalesVal[gain];
        double o1 = (x4+x2)/2;


        if (CUR_CBR_MODE == CBR_MODE_5T)
        {

            hnd->AutoCalibrInfo[gain].Coef[1].Gain = 2*u_int/(x4-x2);
            hnd->AutoCalibrInfo[gain].Coef[1].Offset = x3;

            hnd->AutoCalibrInfo[gain].Coef[2].Gain = (u-u_int)/(x5-x4);
            hnd->AutoCalibrInfo[gain].Coef[2].Offset = x4 - (1/hnd->AutoCalibrInfo[gain].Coef[2].Gain) * u_int + (x3-o1);

            hnd->AutoCalibrInfo[gain].Coef[0].Gain = (u-u_int)/(x2-x1);
            hnd->AutoCalibrInfo[gain].Coef[0].Offset = x2 - (1/hnd->AutoCalibrInfo[gain].Coef[0].Gain) * (-u_int) + (x3-o1);
        }

        if (CUR_CBR_MODE == CBR_MODE_3T)
        {
            hnd->AutoCalibrInfo[gain].Coef[0].Gain = hnd->AutoCalibrInfo[gain].Coef[1].Gain =  hnd->AutoCalibrInfo[gain].Coef[2].Gain = 2*u/(x5-x1);
            hnd->AutoCalibrInfo[gain].Coef[0].Offset = hnd->AutoCalibrInfo[gain].Coef[1].Offset = hnd->AutoCalibrInfo[gain].Coef[2].Offset = x3;
        }

        if (CUR_CBR_MODE == CBR_MODE_3T_INTER)
        {
            hnd->AutoCalibrInfo[gain].Coef[0].Gain = hnd->AutoCalibrInfo[gain].Coef[1].Gain =  hnd->AutoCalibrInfo[gain].Coef[2].Gain = 2*u_int/(x4-x2);
            hnd->AutoCalibrInfo[gain].Coef[0].Offset = hnd->AutoCalibrInfo[gain].Coef[1].Offset = hnd->AutoCalibrInfo[gain].Coef[2].Offset = x3;
        }


        hnd->AutoCalibrInfo[gain].HVal = x4;
        hnd->AutoCalibrInfo[gain].LVal = x2;
        

    }
    else
    {

        double x4 = hnd->AutoCalibrInfo[gain].TempScale[0].Ref;
        double x3 = hnd->AutoCalibrInfo[gain].TempScale[0].Null;
        double x2 = hnd->AutoCalibrInfo[gain].TempScale[0].NRef;
        double o1 = (x4+x2)/2;

        if (CUR_CBR_MODE == CBR_MODE_5T)
        {
            double u_int =  (double)hnd->ModuleInfo.CbrCoef.U[gain]*LTR114_MAX_SCALE_VALUE/theScalesVal[gain];
            double u_int2;
            double u_int1;

            hnd->AutoCalibrInfo[gain].Coef[1].Gain =  2*u_int/(x4-x2);
            hnd->AutoCalibrInfo[gain].Coef[1].Offset = x3;

            x2 = hnd->AutoCalibrInfo[LTR114_URANGE_2].TempScale[0].NInterm;
            x4 = hnd->AutoCalibrInfo[LTR114_URANGE_2].TempScale[0].Interm;
            u_int1 = u_int * x2/hnd->AutoCalibrInfo[gain].TempScale[0].NRef; 
            u_int2 = u_int * x4/hnd->AutoCalibrInfo[gain].TempScale[0].Ref; 

            hnd->AutoCalibrInfo[gain].Coef[0].Gain = hnd->AutoCalibrInfo[gain].Coef[1].Gain * hnd->AutoCalibrInfo[LTR114_URANGE_2].Coef[0].Gain/hnd->AutoCalibrInfo[LTR114_URANGE_2].Coef[1].Gain;
            hnd->AutoCalibrInfo[gain].Coef[0].Offset = x2 - (1/hnd->AutoCalibrInfo[gain].Coef[0].Gain) * (-u_int1) + (x3-o1);

            hnd->AutoCalibrInfo[gain].Coef[2].Gain = hnd->AutoCalibrInfo[gain].Coef[1].Gain * hnd->AutoCalibrInfo[LTR114_URANGE_2].Coef[2].Gain/hnd->AutoCalibrInfo[LTR114_URANGE_2].Coef[1].Gain;
            hnd->AutoCalibrInfo[gain].Coef[2].Offset = x4 - (1/hnd->AutoCalibrInfo[gain].Coef[2].Gain) * u_int2 + (x3-o1);        
        }
        else
        {
            double u =  (double)hnd->ModuleInfo.CbrCoef.U[gain]*LTR114_MAX_SCALE_VALUE/theScalesVal[gain];

     //   hnd->AutoCalibrInfo[gain].Coef.Gain = u * (x1 - x3)/((x1*x1 + x2*x2 + x3*x3) - pow(x1 + x2 + x3,2)/3);
    //    hnd->AutoCalibrInfo[gain].Coef.Offset = - hnd->AutoCalibrInfo[gain].Coef.Gain * (x1+x2+x3)/3;

            hnd->AutoCalibrInfo[gain].Coef[0].Gain = hnd->AutoCalibrInfo[gain].Coef[1].Gain =  hnd->AutoCalibrInfo[gain].Coef[2].Gain = 2*u/(x4-x2);
      // hnd->AutoCalibrInfo[gain].Coef.Offset = (x1+x2+x3)/3;
            hnd->AutoCalibrInfo[gain].Coef[0].Offset = hnd->AutoCalibrInfo[gain].Coef[1].Offset = hnd->AutoCalibrInfo[gain].Coef[2].Offset = x3;

            
        }
        hnd->AutoCalibrInfo[gain].HVal = x4;
        hnd->AutoCalibrInfo[gain].LVal = x2;
    }
    hnd->AutoCalibrInfo[gain].LastVals = hnd->AutoCalibrInfo[gain].TempScale[0];

    return LTR_OK;

}

int get_autocbr_cnt(PTLTR114 hnd)
{
    /* Вспомогательная процедура подсчета необходимого количество измеренных
     * значений при автокалибровке для осреднения (зависит от частоты модуля)
     *
     * ПАРАМЕТРЫ
     *   hnd      - указатель на описатель модуля;
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   кол-во отсчетов для осреднения.
     */
    int res = (int)LTR114_FREQ((*hnd))/10;
    if (res < 2)
        res = 2;
    return res;
}

int proc_autocalibr(PTLTR114 hnd, DWORD** psrc1, BYTE *cnt, WORD cur_int)
{
     /*Вспомогательная процедура для обработки данных о измерении
     * модулем собственного нуля и значений на границах шкалы во время межкадрового интервала
     * в режиме автокалибровки
     * ПАРАМЕТРЫ
     *   hnd       - указатель на описатель модуля;
     *   psrc1     - указатель на массив с принятыми данными
     *   cnt       - значение циклического счетчика
     *   cur_int   - оставшееся количичество отсчетов данного межкадрового интервала
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */

    DWORD* psrc = *psrc1;
    BYTE sw_mode = RECV0_SWITCH_MODE(*psrc);
    BYTE gain = RECV0_GAIN(*psrc);
     INT data, index,i;

     switch (gain)
     {
         case LTR114_GAIN_10: gain = LTR114_URANGE_10; break;
         case LTR114_GAIN_2: gain = LTR114_URANGE_2; break;
         case LTR114_GAIN_04: gain = LTR114_URANGE_04; break;
         default: return LTR114_ERR_INVALID_LCH;
     }
    
   

    data = (INT)RECV0_DATA2(*psrc)<< 16; //сохранение старшего байта данных

    //проверка второго слова
    CYCLE_COUNTER_INC(*cnt);
    psrc++;

    if (((*psrc >> 8) & 0x8F) != (hnd->Channel.cc - CC_MODULE1))
        return LTR114_ERR_ADCDATA_SLOTNUM;
    if (*cnt != RECV_COUNT(*psrc))
        return LTR114_ERR_ADCDATA_CNT;

    data |= RECV1_DATA10(*psrc); 

    data = SCALE_TO_INT(data);
    //сохранение измеренного значение в соответствующий массив в зависимости от режима коммутации и диапазона
    switch(sw_mode)
    {
        case LTR114_MEASMODE_NULL:
            if ( hnd->AutoCalibrInfo[gain].Index.Null<get_autocbr_cnt(hnd))
            {
                hnd->AutoCalibrInfo[gain].TempScale[hnd->AutoCalibrInfo[gain].Index.Null].Null = data;
                hnd->AutoCalibrInfo[gain].Index.Null++;
            }
            else
            {
                for (i=0; i< get_autocbr_cnt(hnd)-1; i++)
                    hnd->AutoCalibrInfo[gain].TempScale[i].Null = hnd->AutoCalibrInfo[gain].TempScale[i+1].Null ; 
                hnd->AutoCalibrInfo[gain].TempScale[get_autocbr_cnt(hnd)-1].Null = data;
            }
            break;
        case LTR114_MEASMODE_NDAC12_INTR:
            if ( hnd->AutoCalibrInfo[gain].Index.NInterm<get_autocbr_cnt(hnd))
            {
                hnd->AutoCalibrInfo[gain].TempScale[hnd->AutoCalibrInfo[gain].Index.NInterm].NInterm = data;
                hnd->AutoCalibrInfo[gain].Index.NInterm++;
            }
            else
            {
                for (i=0; i< get_autocbr_cnt(hnd)-1; i++)
                    hnd->AutoCalibrInfo[gain].TempScale[i].NInterm = hnd->AutoCalibrInfo[gain].TempScale[i+1].NInterm ; 
                hnd->AutoCalibrInfo[gain].TempScale[get_autocbr_cnt(hnd)-1].NInterm = data;
            }
            break;
        case LTR114_MEASMODE_DAC12_INTR:
            if ( hnd->AutoCalibrInfo[gain].Index.Interm<get_autocbr_cnt(hnd))
            {
                hnd->AutoCalibrInfo[gain].TempScale[hnd->AutoCalibrInfo[gain].Index.Interm].Interm = data;
                hnd->AutoCalibrInfo[gain].Index.Interm++;
            }
            else
            {
                for (i=0; i< get_autocbr_cnt(hnd)-1; i++)
                    hnd->AutoCalibrInfo[gain].TempScale[i].Interm = hnd->AutoCalibrInfo[gain].TempScale[i+1].Interm ; 
                hnd->AutoCalibrInfo[gain].TempScale[get_autocbr_cnt(hnd)-1].Interm = data;
            }

            break;
        case LTR114_MEASMODE_NDAC12:
            if ( hnd->AutoCalibrInfo[gain].Index.NRef<get_autocbr_cnt(hnd))
            {
                hnd->AutoCalibrInfo[gain].TempScale[hnd->AutoCalibrInfo[gain].Index.NRef].NRef = data;
                hnd->AutoCalibrInfo[gain].Index.NRef++;
            }
            else
            {
                for (i=0; i< get_autocbr_cnt(hnd)-1; i++)
                    hnd->AutoCalibrInfo[gain].TempScale[i].NRef = hnd->AutoCalibrInfo[gain].TempScale[i+1].NRef;
                hnd->AutoCalibrInfo[gain].TempScale[get_autocbr_cnt(hnd)-1].NRef = data;
            }
            break;
        case LTR114_MEASMODE_DAC12:
            if ( hnd->AutoCalibrInfo[gain].Index.Ref<get_autocbr_cnt(hnd))
            {
                hnd->AutoCalibrInfo[gain].TempScale[hnd->AutoCalibrInfo[gain].Index.Ref].Ref = data;
                hnd->AutoCalibrInfo[gain].Index.Ref++;
            }
            else
            {
                for (i=0; i< get_autocbr_cnt(hnd)-1; i++)
                    hnd->AutoCalibrInfo[gain].TempScale[i].Ref = hnd->AutoCalibrInfo[gain].TempScale[i+1].Ref ; 
                hnd->AutoCalibrInfo[gain].TempScale[get_autocbr_cnt(hnd)-1].Ref = data;
            }

            /*если это последнее измерение NRef на текущем интервале для данного диапазона
              => рассчет калибровачных коэффициентов*/
      /*      if (cur_int <= LTR114_ADC_RANGEQNT*LTR114_AUTOCALIBR_STEPS)
            {
                //кол-во сделанных полных измерений на интервале - минимальноe кол-во каждого из 3-х измерений
                index = min(min(hnd->AutoCalibrInfo[gain].Index.Null, hnd->AutoCalibrInfo[gain].Index.Ref), 
                    hnd->AutoCalibrInfo[gain].Index.NRef);
                if (index > 1) //если больше одного измерения - вычислениее среднего
                {
                    long SNull = 0, SRef = 0, SNRef = 0; 
                    INT i;
                    for (i = 0; i < index; i++) 
                    {
                        SNull += hnd->AutoCalibrInfo[gain].TempScale[i].Null;
                        SRef += hnd->AutoCalibrInfo[gain].TempScale[i].Ref;
                        SNRef += hnd->AutoCalibrInfo[gain].TempScale[i].NRef;
                    }
                    hnd->AutoCalibrInfo[gain].TempScale[0].Null = SNull/index;
                    hnd->AutoCalibrInfo[gain].TempScale[0].Ref = SRef/index; 
                    hnd->AutoCalibrInfo[gain].TempScale[0].NRef = SNRef/index; 
                }
                coef_calc(hnd, gain);  //рассчет коэфициентов
                //сброс временных значених
                hnd->AutoCalibrInfo[gain].Index.Null = hnd->AutoCalibrInfo[gain].Index.Ref = hnd->AutoCalibrInfo[gain].Index.NRef = 0;
            }*/
            if (gain == LTR114_URANGE_10)
                index = std::min(std::min(std::min(hnd->AutoCalibrInfo[gain].Index.Null, hnd->AutoCalibrInfo[gain].Index.Ref),
                        std::min(hnd->AutoCalibrInfo[LTR114_URANGE_2].Index.Interm, hnd->AutoCalibrInfo[LTR114_URANGE_2].Index.NInterm)),
                        hnd->AutoCalibrInfo[gain].Index.NRef);
            else
                index = std::min(std::min(std::min(hnd->AutoCalibrInfo[gain].Index.Null, hnd->AutoCalibrInfo[gain].Index.Ref),
                        std::min(hnd->AutoCalibrInfo[gain].Index.Interm, hnd->AutoCalibrInfo[gain].Index.NInterm)),
                        hnd->AutoCalibrInfo[gain].Index.NRef); 

            //кол-во сделанных полных измерений на интервале - минимальноe кол-во каждого из 3-х измерений


           // index = min(min(hnd->AutoCalibrInfo[gain].Index.Null, hnd->AutoCalibrInfo[gain].Index.Ref), 
            //    hnd->AutoCalibrInfo[gain].Index.NRef);
                if (index >= get_autocbr_cnt(hnd))
           // if (((gain!=LTR114_URANGE_10) && (index >= get_autocbr_cnt(hnd)))
            //    || ((gain == LTR114_URANGE_10) && (index >= (get_autocbr_cnt(hnd) - 1))))//если кол-во измерений = необходимому - вычисление среднего и подсчет коэф.
            {
                __int64 SNull = 0, SRef = 0, SNRef = 0, SInter = 0, SNInter = 0; 
                INT i;
                for (i = 0; i < index; i++) 
                {
                    SNull += hnd->AutoCalibrInfo[gain].TempScale[i].Null;
                    SRef += hnd->AutoCalibrInfo[gain].TempScale[i].Ref;
                    SNRef += hnd->AutoCalibrInfo[gain].TempScale[i].NRef;

                    if (gain != LTR114_URANGE_10)
                    {
                        SInter += hnd->AutoCalibrInfo[gain].TempScale[i].Interm;
                        SNInter += hnd->AutoCalibrInfo[gain].TempScale[i].NInterm;
                    }
                    
                }
                hnd->AutoCalibrInfo[gain].TempScale[0].Null = SNull/index;
                hnd->AutoCalibrInfo[gain].TempScale[0].Ref = SRef/index; 
                hnd->AutoCalibrInfo[gain].TempScale[0].NRef = SNRef/index; 

                hnd->AutoCalibrInfo[gain].TempScale[0].Interm = SInter/index;
                hnd->AutoCalibrInfo[gain].TempScale[0].NInterm = SNInter/index;
            
                coef_calc(hnd, gain);  //рассчет коэфициентов
                

                //сдвиг окна
                /*for (i=0; i< index-1; i++)
                {
                    hnd->AutoCalibrInfo[gain].TempScale[i].Null = hnd->AutoCalibrInfo[gain].TempScale[i+1].Null;
                    hnd->AutoCalibrInfo[gain].TempScale[i].Ref  = hnd->AutoCalibrInfo[gain].TempScale[i+1].Ref;
                    hnd->AutoCalibrInfo[gain].TempScale[i].NRef = hnd->AutoCalibrInfo[gain].TempScale[i+1].NRef;
                    if (gain != LTR114_URANGE_10)
                    {
                        hnd->AutoCalibrInfo[gain].TempScale[i].Interm = hnd->AutoCalibrInfo[gain].TempScale[i+1].Interm;
                        hnd->AutoCalibrInfo[gain].TempScale[i].NInterm = hnd->AutoCalibrInfo[gain].TempScale[i+1].NInterm;
                    }
                }

                hnd->AutoCalibrInfo[gain].Index.Null = hnd->AutoCalibrInfo[gain].Index.Ref = hnd->AutoCalibrInfo[gain].Index.NRef = index-1;
                if (gain != LTR114_URANGE_10)
                {
                    hnd->AutoCalibrInfo[gain].Index.Interm = hnd->AutoCalibrInfo[gain].Index.NInterm = index-1;
                }*/
            }
            break;
    }
    *psrc1 = psrc;
    return LTR_OK;    
}




void make_ch_table(PTLTR114 hnd, int flags, int* ch_table, int *ch_cnt)
{
    int skipavgr = 0;
    int cur_ch, ch_num=1;


    *ch_cnt = hnd->LChQnt;

    ch_table[0] = 0;
    

    for (cur_ch = 1; (cur_ch < hnd->LChQnt); cur_ch++)
    {
      //если включено осреднение соседних измерений R с +I и -I
        if ((IS_MODE_R(hnd->LChTbl[cur_ch].MeasMode))&&(flags & LTR114_PROCF_AVGR)&&(!skipavgr))
        {
            DWORD mask =~(LTR114_MEASMODE_R ^ LTR114_MEASMODE_NR);
             //если режимы полностью совпадают и один для +I другой для -I
            if ((((hnd->LChTbl[cur_ch].MeasMode == LTR114_MEASMODE_R) && (hnd->LChTbl[cur_ch-1].MeasMode == LTR114_MEASMODE_NR))
                    ||((hnd->LChTbl[cur_ch].MeasMode == LTR114_MEASMODE_NR) && (hnd->LChTbl[cur_ch-1].MeasMode == LTR114_MEASMODE_R)))
                    &&(hnd->LChTbl[cur_ch].Channel == hnd->LChTbl[cur_ch-1].Channel)
                    &&(hnd->LChTbl[cur_ch].Range == hnd->LChTbl[cur_ch-1].Channel)
                    )
            {
            
                skipavgr = 1; //след. не осреднять - для случая трех подряд записей для R
                *ch_cnt = *ch_cnt-1;
                continue;  //переход к след. отсчету
            }
        }
        ch_table[ch_num++] = cur_ch;
        skipavgr = 0;
    }
}

/*LTR114API_DllExport(INT) LTR114_ConvertToValue(PTLTR114 hnd, double *data, INT size, INT correction_mode, INT flags)
{
    int i, cur_ch;
    INT ch_table[LTR114_MAX_LCHANNEL];
    INT ch_cnt;

    make_ch_table(hnd, flags, ch_table, &ch_cnt);
    for (i=0; i <size; i+=ch_cnt)
    {
        for (cur_ch=0; cur_ch < ch_cnt; cur_ch++)
        {
            double dt = data[i+cur_ch];
            //применение калибровочных коэффициентов
            if (correction_mode != LTR114_CORRECTION_MODE_NONE)
                if (IS_MODE_R(hnd->LChTbl[ch_table[cur_ch]].MeasMode))
                    dt = (dt - hnd->AutoCalibrInfo[LTR114_URANGE_04].Coef.Offset) * hnd->AutoCalibrInfo[LTR114_URANGE_04].Coef.Gain;
                else
                    dt = (dt - hnd->AutoCalibrInfo[hnd->LChTbl[ch_table[cur_ch]].Range].Coef.Offset) * hnd->AutoCalibrInfo[hnd->LChTbl[ch_table[cur_ch]].Range].Coef.Gain; 
                      
             if (flags & LTR114_PROCF_VALUE) //перевод в физ единицы
             {
                if (IS_MODE_R(hnd->LChTbl[ch_table[cur_ch]].MeasMode))
                {
                    dt = (theScalesVal[LTR114_URANGE_04]*dt/LTR114_MAX_SCALE_VALUE)*1000/hnd->ModuleInfo.CbrCoef.I[hnd->LChTbl[ch_table[cur_ch]].Range];
                    if (dt < 0)
                        dt = -dt;
                    //dt = theRScalesVal[LTR114_TABLE_N(hnd->LChTbl[cur_ch])]*dt/LTR114_MAX_SCALE_VALUE;
                }
                else
                    dt = theScalesVal[hnd->LChTbl[ch_table[cur_ch]].Range]*dt/LTR114_MAX_SCALE_VALUE;
             }
             data[i+cur_ch] = dt;

             
        }
    }
    return 0;
}*/


INT LTR114_ProcessDataCb
    (
    PTLTR114 hnd,
    DWORD *src,
    double *dest,
    double *therm,
    INT *size,
    INT *tcnt,
    INT correction_mode,
    INT flags,
    INT (* cb_func)(PTLTR114 hnd, double* data)
    )
{
    /*
     * Обработка полученных от модуля данных с возможностью установить cb-функцию, вызываемою после дешифровки
        кажого фрейма до его обработки.
     * ОПИСАНИЕ
     *   Производится проверка полученных данных, преобразование слов данных в дробные значения результатов измерения,
     *   Полученные данные калибруются (зависит от режима коррекции данных)
     *   и, если требуется, приводятся к диапазону измерений.
     *   При ненулевом межкадровом интервале производится обработка измерений модулем своих параметров 
     *   во время межкадрового интервала с пересчетом калибровочных коэффициентов в режиме автокалибровки.
     *   Данные от термодатчика выделяются из общего потока и сохраняются в одтельный массив
     *   При необходимости два измерения сопротивления с разным током осредняются
     * ПАРАМЕТРЫ
     *   hnd    - указатель на описатель модуля;
     *   src    - указатель на полученный от модуля массив данных;
     *   dest   - указатель на массив, в который будут записаны обработанные данные;
     *   therm  - указатель на массив, в который будут записаны значения температуры
     *   size   - размер полученных данных в отсчетах АЦП (т.е. словах от модуля), на выходе из
     *            функции равен количеству обработанных и помещенных в выходной массив отсчетов
     *            АЦП;
     *   tcnt   - размер массива therm
     *   correction_mode - режим коррекции полученных значений с пом. калибр. коэффициентов;
     *   flags   - флаги:
                    LTR114_PROCF_VALUE - признак необходимости перевода кодов АЦП в ф
                                       изические единицы (Вольты, Омы) (в соответствии с диапазоном).
                   LTR114_PROCF_AVGR - признак включения осреднения измериний сопротивлений с разным направлением тока 
                                       на соседних логических каналах
     *   cb_func - cb-функция, вызывается после дешифрации кадра данных. 
                   Принимает указатель на описатель модуля и указатель на кадр данных,
                   который функция может изменить.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */

    INT ret_val = LTR_OK;
    INT res = LTR_OK;
    
    if (tcnt!=NULL)    *tcnt = 0;

    if (hnd == NULL)
        return LTR114_ERR_INVALID_DESCR;
        
    if ((hnd->LChQnt + hnd->Interval <= 0) || (LTR114_MAX_LCHANNEL < hnd->LChQnt))
        return LTR114_ERR_INVALID_ADCLCHQNT;

    if ((correction_mode!=LTR114_CORRECTION_MODE_NONE) 
        &&(correction_mode!=LTR114_CORRECTION_MODE_INIT)
        &&(correction_mode!=LTR114_CORRECTION_MODE_AUTO))
        return LTR114_ERR_CORRECTION_MODE;


    if ((src == NULL) || (dest == NULL))
        ret_val = LTR114_ERR_INVALID_ARRPOINTER;
    else
    {
        /* один кадр данных */
        DWORD frame[LTR114_MAX_LCHANNEL];
        double d_frame[LTR114_MAX_LCHANNEL];
        int slot_num = hnd->Channel.cc - CC_MODULE1;
        double *pdest = dest;
        DWORD *psrc = src;
        BYTE cnt;           //счетчик пакетов принятых данных по модулю LTR114_CYCLE_CNT_MODE
        BYTE cur_ch = 0;    //номер текущего лог. канала в лог. таблице
        WORD cur_int = 0;   //во время межкадрового интервала - кол-во ост. отсчетов
        DWORD buff;

        cnt = RECV_COUNT(*psrc); //установка начального значения счетчика
        
        if (hnd->LChQnt == 0)   //если 0 лог. каналов => сразу переход к приему интервала
            cur_int = hnd->Interval;
        
        if (!RECV_IS_FIRST_WORD(*psrc)) //проверка - является ли первое прин. слова первым словом измерения
        {
            psrc++;
            CYCLE_COUNTER_INC(cnt);
        }

        for (; (psrc < src + *size); psrc++)
        {
            /* проверки:
             *  1) слово не является командой (маска 0x80) и номер слота (0x0F)
             *  2) двухбитный счетчик слов
             */

            //проверка первого слова
            if (((*psrc >> 8) & 0x8F) != slot_num)
                ret_val = LTR114_ERR_ADCDATA_SLOTNUM;
            if (cnt != RECV_COUNT(*psrc))
                ret_val = LTR114_ERR_ADCDATA_CNT;

            //проверка - пришедшее слово является ли данными от термометра
            if (IS_THERM(*psrc) && (hnd->SpecialFeatures & LTR114_FEATURES_THERM)) 
            {
                if (IS_THERM_VALID(*psrc))  //действительно ли значение
                {
                    if (therm!=NULL)     //сохраняем принятое значение, переведенное в градусы С
                        *therm++ = (double)((INT)(*psrc & 0xFFFF0000)>>16)/16;
                    if (tcnt!=NULL) (*tcnt)++;
                }
                CYCLE_COUNTER_INC(cnt);
                continue;  //переход к след. измерению
            }

            if (!cur_int && !hnd->LChQnt) //для 0 лог. каналов по окончанию одного интервала - в начало следующего
                cur_int = hnd->Interval;
            
            //если идет интервал в режиме без калибровки - пропуск отсчета
            if ((cur_int) && (hnd->Interval > 0) && (hnd->SpecialFeatures & LTR114_FEATURES_STOPSW))
            {
                psrc++;
                CYCLE_COUNTER_INC(cnt);
                cur_int--;
                CYCLE_COUNTER_INC(cnt);
                continue;
            }
            //if (cur_int)                                                            //если межкадровая задержка
                if ((hnd->Interval > 0) && (IS_AUTO_CALIBR_MODE(*psrc)))            //если правильный режим коммутации
                {
                    if (correction_mode == LTR114_CORRECTION_MODE_AUTO)           //если вкл. автокалибровка 
                    {
                        res = proc_autocalibr(hnd, &psrc, &cnt, cur_int);     //обработка слова данных ражима автокалибровки
                    }
                    else
                    {
                        psrc++;
                        CYCLE_COUNTER_INC(cnt);
                    }

                    cur_int--;
                    CYCLE_COUNTER_INC(cnt);
                    if (res!=LTR_OK)
                        ret_val = res;
                    continue;                  //переход к след. записи
                }
             /*   else                  //если режим не верный - ошибка, переход к нач.  кадра
                {
                    ret_val = LTR114_ERR_ADCDATA_CHNUM;
                    cur_ch = 0; 
                    cur_int = 0;
                    continue;
                }*/

            //проверка - соответствует ли режим измерения заданному в таблице
            if ((RECV0_SWITCH_MODE(*psrc) != LTR114_TABLE_SWMODE(create_table_lch(hnd->LChTbl[cur_ch])))
                ||(RECV0_GAIN(*psrc) != LTR114_TABLE_GAIN(create_table_lch(hnd->LChTbl[cur_ch])))
                )
            {

                ret_val = LTR114_ERR_ADCDATA_CHNUM; 
                if (IS_AUTO_CALIBR_MODE(*psrc))  //если режим коммутации автокалибровочный - переход к межкадр. интервалу
                {
                    cur_int = hnd->Interval;
                    if (correction_mode == LTR114_CORRECTION_MODE_AUTO)
                    {
                        res = proc_autocalibr(hnd, &psrc, &cnt, cur_int);         //обработка слова данных ражима автокалибровки
                    }
                    else
                    {
                        psrc++;
                        CYCLE_COUNTER_INC(cnt);
                    }
                    cur_int--;
                }
                else
                {
                    cur_ch = 0;
                }
            }
            else        //обработка логич. канала
            {
                buff = (DWORD)RECV0_DATA2(*psrc)<< 16; //сохранение старшего байта данных
                //проверка второго слова
                CYCLE_COUNTER_INC(cnt);
                psrc++;

                if (((*psrc >> 8) & 0x8F) != slot_num)
                     ret_val = LTR114_ERR_ADCDATA_SLOTNUM;
                if (cnt != RECV_COUNT(*psrc))
                    ret_val = LTR114_ERR_ADCDATA_CNT;

                if (RECV1_N(*psrc) != LTR114_TABLE_SRC(create_table_lch(hnd->LChTbl[cur_ch])))
                {
                    ret_val = LTR114_ERR_ADCDATA_CHNUM;
                    cur_ch = 0;
                }
                else
                {
                    buff |= RECV1_DATA10(*psrc); 
                    frame[cur_ch] = buff;
                    if (++cur_ch >= hnd->LChQnt)                /* обработан кадр отсчетов АЦП */
                    {
                        int skipavgr = 0;
                        
                        for (cur_ch = 0; (cur_ch < hnd->LChQnt); cur_ch++)
                        {
                            d_frame[cur_ch] = SCALE_TO_INT(frame[cur_ch]);
                        }
                        if (cb_func!=NULL)
                            cb_func(hnd, d_frame);
                                                /* обработка и сохранение данных в выходном массиве */
                        for (cur_ch = 0; (cur_ch < hnd->LChQnt); cur_ch++)
                        {
                            double dt;
                            INT gain = (IS_MODE_R(hnd->LChTbl[cur_ch].MeasMode)) ? LTR114_URANGE_04 : hnd->LChTbl[cur_ch].Range;
                            dt = d_frame[cur_ch];
                            //применение калибровочных коэффициентов
                            if (correction_mode != LTR114_CORRECTION_MODE_NONE)
                            {
                                double k, b;
                                if (dt < hnd->AutoCalibrInfo[gain].LVal)
                                {
                                    k = hnd->AutoCalibrInfo[gain].Coef[0].Gain;
                                    b = hnd->AutoCalibrInfo[gain].Coef[0].Offset;
                                }
                                else if (dt > hnd->AutoCalibrInfo[gain].HVal)
                                {
                                    k = hnd->AutoCalibrInfo[gain].Coef[2].Gain;
                                    b = hnd->AutoCalibrInfo[gain].Coef[2].Offset;
                                }
                                else
                                {
                                    k = hnd->AutoCalibrInfo[gain].Coef[1].Gain;
                                    b = hnd->AutoCalibrInfo[gain].Coef[1].Offset;
                                }
                                dt = (dt - b) * k; 
                            }
                            if (flags & LTR114_PROCF_VALUE) //перевод в физ единицы
                            {
                                if (IS_R_SWMODE(hnd->LChTbl[cur_ch].MeasMode))
                                {
                                    dt = (theScalesVal[LTR114_URANGE_04]*dt/LTR114_MAX_SCALE_VALUE)*1000/hnd->ModuleInfo.CbrCoef.I[hnd->LChTbl[cur_ch].Range];
                                    if (dt < 0)
                                        dt = -dt;
                                    //dt = theRScalesVal[LTR114_TABLE_N(hnd->LChTbl[cur_ch])]*dt/LTR114_MAX_SCALE_VALUE;
                                }                                    
                                else
                                    dt = theScalesVal[hnd->LChTbl[cur_ch].Range]*dt/LTR114_MAX_SCALE_VALUE;
                            }

                            //если включено осреднение соседних измерений R с +I и -I
                            if ((IS_MODE_R(hnd->LChTbl[cur_ch].MeasMode))&&(flags & LTR114_PROCF_AVGR)&&(cur_ch > 0)&&(!skipavgr))
                            {
                                DWORD mask =~(LTR114_MEASMODE_R ^ LTR114_MEASMODE_NR);
                                 //если режимы полностью совпадают и один для +I другой для -I
                                if ((((hnd->LChTbl[cur_ch].MeasMode == LTR114_MEASMODE_R) && (hnd->LChTbl[cur_ch-1].MeasMode == LTR114_MEASMODE_NR))
                                    ||((hnd->LChTbl[cur_ch].MeasMode == LTR114_MEASMODE_NR) && (hnd->LChTbl[cur_ch-1].MeasMode == LTR114_MEASMODE_R)))
                                    &&(hnd->LChTbl[cur_ch].Range == hnd->LChTbl[cur_ch-1].Range)
                                    &&(hnd->LChTbl[cur_ch].Channel == hnd->LChTbl[cur_ch-1].Channel))
                                {
                                    *(pdest-1) = ((fabs(*(pdest-1))) + fabs(dt))/2; //осреднение
                                    skipavgr = 1; //след. не осреднять - для случая трех подряд записей для R
                                    
                                    continue;  //переход к след. отсчету
                                }
                            }
                            skipavgr = 0;
                         
                            *pdest++ = dt;
                        }


//                        first = 0;
                        cur_ch = 0;
                        cur_int = hnd->Interval;
                    }  //for (cur_ch = 0; (cur_ch < hnd->LChQnt); cur_ch++)
                }
            }
            CYCLE_COUNTER_INC(cnt);
        }
        *size = pdest - dest;
     } /*else @else if ((src == NULL) || (dest == NULL))@*/
     return ret_val;

}


INT LTR114_ProcessData
    (
    PTLTR114 hnd,
    DWORD *src,
    double *dest,
    INT *size,
    INT correction_mode,
    INT flags
    )
{
    /*
     * Обработка полученных от модуля данных без данных о температуре
     * Описание и параметры аналогичны LTR114_ProcessDataCb, за исключением того, 
     * что данные по темперетуре отбрасываются и нет возможности назначения cb-функции
     */
    return LTR114_ProcessDataCb(hnd, src, dest, NULL, size, NULL, correction_mode, flags, NULL);
}





INT LTR114_ProcessDataTherm
    (
    PTLTR114 hnd,
    DWORD *src,
    double *dest,
    double *therm,
    INT *size,
    INT *tcnt,
    INT correction_mode,
    INT flags
    )
{
    /*
     * Обработка полученных от модуля данных 
     * Описание и параметры аналогичны LTR114_ProcessDataCb, за исключением того, 
     * что нет возможности назначения cb-функции
     */

    return LTR114_ProcessDataCb(hnd, src, dest, therm, size, tcnt, correction_mode, flags, NULL);
}



/*------------------------------------------------------------------------------------------------*/
INT LTR114_SetADC
    (
    PTLTR114 hnd
    )
{
    /*
     * Установка режимов работы модуля LTR114.
     * ОПИСАНИЕ
     *   Выполняется формирование и передача управляющей таблицы, значений делителя и пределителя в
     *   частоты для АЦП и количество логических каналов модуль.
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель состояния модуля.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */

    INT ret_val = LTR_OK;

    if (hnd == NULL)
        return LTR114_ERR_INVALID_DESCR;
    if (hnd->Active)
        return LTR114_ERR_ALREADY_RUN;
    if ((hnd->LChQnt + hnd->Interval <= 0) || (LTR114_MAX_LCHANNEL < hnd->LChQnt))
        return LTR114_ERR_INVALID_ADCLCHQNT;
    else
    {
        /* подготовка передаваемой в модуль команды
         * 1 - определение размера передаваемых данных в байтах;
         * 2 - формирование управляющей таблицы;
         * 3 - получение значений делителя и пределителя тактовой частоты модуля для заданной
         *     частоты дискретизации.
         */

        WORD size =  (LTR114_MODULE_MODE_BASE_SIZE + hnd->LChQnt);
        DWORD cmdbuf[LTR114_MODULE_MODE_SIZE];            /* команда для передачи в модуль */
        long divider;
        int i;
        DWORD *pcmdbuf;
        int presc;

        
        pcmdbuf = cmdbuf;
                                       //первое слово - размер структуры
        *pcmdbuf++ = fill_command(modulemode_first_cmd, size*2);
        *pcmdbuf++ = fill_command(modulemode_cmd, hnd->LChQnt);

        
        for (i = 0; (i < hnd->LChQnt); i++)                                         /*2*/
        {
            WORD data = create_table_lch(hnd->LChTbl[i]);
            if (data != INVALID_CH)
                *pcmdbuf++ = fill_command(modulemode_cmd, data);
            else return LTR114_ERR_INVALID_LCH;
        }

        *pcmdbuf++ = fill_command(modulemode_cmd, hnd->Interval);

        
        //рассчет делителя частоты
        presc = 0; 

        divider = hnd->FreqDivider;

        if ((divider < min_divider) || (max_divider < divider))
            //((hnd->ChRate = eval_adcrate(hnd->ADCRate.prescaler, divider)) > 400.0)
            return LTR114_ERR_INVALID_ADCRATE;
		if (!(hnd->SpecialFeatures & LTR114_MANUAL_OSR)) 
            hnd->AdcOsr = eval_OSR(LTR114_FREQ((*hnd)));
        *pcmdbuf++ = fill_command(modulemode_cmd, hnd->FreqDivider);
        *pcmdbuf++ = fill_command(modulemode_cmd, (((WORD)hnd->SpecialFeatures << 8)&0xFF00) + ++presc);

        if ((hnd->SyncMode != LTR114_SYNCMODE_NONE)&&(hnd->SyncMode!=LTR114_SYNCMODE_INTERNAL)
            &&(hnd->SyncMode != LTR114_SYNCMODE_MASTER) && (hnd->SyncMode !=LTR114_SYNCMODE_EXTERNAL))
            return LTR114_ERR_INVALID_SYNCMODE;
        *pcmdbuf++ = fill_command(modulemode_cmd, (((WORD)hnd->SyncMode<<8)&0xFF00) + hnd->AdcOsr);

        if (ret_val == LTR_OK)
        {
            DWORD ack;
            long n;

            if ((n = LTR_Send(&hnd->Channel, cmdbuf, size, size*send_tm_out)) < 0)
                return n;
                
            if (n != size)
                return LTR_ERROR_SEND; 
                
            if ((n = LTR_Recv(&hnd->Channel, &ack, NULL, 1, ack_tm_out)) < 0)
                return n;
                
            if (n < 1)
                return LTR114_ERR_NOACK;
                
            ack &= ack_mask;
            if (ack != modulemode_ack)
               return LTR114_ERR_INVALIDACK;
        }

        /*если есть межкадровый интервал - выделение памяти под отсчеты измерения модулем собственных параметров*/
        
            ///int cnt = hnd->Interval/(LTR114_AUTOCALIBR_STEPS * LTR114_ADC_RANGEQNT) + 2;

       /* for (i=0; i < LTR114_ADC_RANGEQNT; i++)
        {
            if (hnd->AutoCalibrInfo[i].TempScale!=NULL)
            {
                free(hnd->AutoCalibrInfo[i].TempScale);
                hnd->AutoCalibrInfo[i].TempScale = NULL;
                hnd->AutoCalibrInfo[i].Index.Null = hnd->AutoCalibrInfo[i].Index.Ref = hnd->AutoCalibrInfo[i].Index.NRef = 
                    hnd->AutoCalibrInfo[i].Index.Interm = hnd->AutoCalibrInfo[i].Index.NInterm =0;
            }
            if (hnd->Interval > 0)
                hnd->AutoCalibrInfo[i].TempScale = (TSCALE_LTR114*)malloc((get_autocbr_cnt(hnd)+10)*sizeof(TSCALE_LTR114));//new TSCALE_LTR114[cnt];//
            
        }*/


        for (i=0; i < LTR114_ADC_RANGEQNT; i++)
        {
            if ((hnd->AutoCalibrInfo[i].TempScale!=NULL)&&(hnd->Interval > 0))
            {
                int scale_size;
                int index,j,k;

                TSCALE_LTR114* buff_scale = hnd->AutoCalibrInfo[i].TempScale;
                scale_size = get_autocbr_cnt(hnd);

                hnd->AutoCalibrInfo[i].TempScale = (TSCALE_LTR114*)malloc((scale_size+10)*sizeof(TSCALE_LTR114));
                
                if (i == LTR114_URANGE_10)
                    index = std::min(std::min(hnd->AutoCalibrInfo[i].Index.Null, hnd->AutoCalibrInfo[i].Index.Ref),
                        std::min(hnd->AutoCalibrInfo[i].Index.NRef,
                        std::min(hnd->AutoCalibrInfo[LTR114_URANGE_2].Index.NInterm, hnd->AutoCalibrInfo[LTR114_URANGE_2].Index.Interm)));

                else
                    index = std::min(std::min(hnd->AutoCalibrInfo[i].Index.Null, hnd->AutoCalibrInfo[i].Index.Ref),
                        std::min(hnd->AutoCalibrInfo[i].Index.NRef, std::min(hnd->AutoCalibrInfo[i].Index.NInterm,hnd->AutoCalibrInfo[i].Index.Interm)));
                
                
                for (j = index - 1, k=std::min(index-1,scale_size-1); (j>=0) && (k>= 0); j--,k--)
                    hnd->AutoCalibrInfo[i].TempScale[k] = buff_scale[j];
                
                hnd->AutoCalibrInfo[i].Index.Null = hnd->AutoCalibrInfo[i].Index.Ref = hnd->AutoCalibrInfo[i].Index.NRef = 
                    hnd->AutoCalibrInfo[i].Index.Interm = hnd->AutoCalibrInfo[i].Index.NInterm = std::min(index, scale_size);
                
                free(buff_scale);
            }
            else
            {
                if (hnd->AutoCalibrInfo[i].TempScale!=NULL)
                {
                    free(hnd->AutoCalibrInfo[i].TempScale);
                }
                hnd->AutoCalibrInfo[i].TempScale = NULL;
                hnd->AutoCalibrInfo[i].Index.Null = hnd->AutoCalibrInfo[i].Index.Ref = hnd->AutoCalibrInfo[i].Index.NRef = 
                    hnd->AutoCalibrInfo[i].Index.Interm = hnd->AutoCalibrInfo[i].Index.NInterm =0;
                
                if (hnd->Interval > 0)
                    hnd->AutoCalibrInfo[i].TempScale = (TSCALE_LTR114*)malloc((get_autocbr_cnt(hnd)+10)*sizeof(TSCALE_LTR114));//new TSCALE_LTR114[cnt];//
            }
        }



        //filter_data_create(hnd);

        //рассчет кол-во байт данных, передоваемых модулем от нач. одного кадра до начала другого - 
        hnd->FrameLength = 2*(hnd->LChQnt + hnd->Interval);
        if (hnd->SpecialFeatures & LTR114_FEATURES_THERM)
            hnd->FrameLength++;



    } /*else @else if ((hnd->LChQnt <= 0) || (LTR11_MAX_LCHANNEL < hnd->LChQnt)@*/

    return ret_val;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
INT LTR114_Start
    (
    PTLTR114 hnd
    )
    {
    /*
     * Запуск сбора данных модулем LTR114.
     * ОПИСАНИЕ
     *   Выполняется передача команды запуска преобразования на бесконечный сбор данных.
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель модуля.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */
    return start_ltr114(hnd, 0);
    }
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
INT LTR114_Stop
    (
    PTLTR114 hnd
    )
    {
    /*
     * Перевод модуля LTR114 в режим ожидания.
     * ОПИСАНИЕ
     *   Выполняется передача команды останова модуля и ожидается подтверждение выполнения команды
     *   от модуля.
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель состояния модуля.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */

#define RD_BUFSZ (100)
    DWORD ack;
    DWORD cmd;
    DWORD rd_buf[RD_BUFSZ];
    long rd_cnt;
    INT ret_val = LTR_OK;
    DWORD tick;




    if (hnd == NULL)
        {
        ret_val = LTR114_ERR_INVALID_DESCR;
        }
    else
        {
        long n;
        hnd->Active = FALSE;
        cmd = fill_command(stop_cmd, 0);
        if ((n = LTR_Send(&hnd->Channel, &cmd, 1, send_tm_out)) < 0)
            {
            ret_val = n;
            }
        if (n != 1)
            {
            ret_val = LTR_ERROR_SEND;
            }
        else
            {
            int data_received = 0;
            DWORD elapsed_time;
            const DWORD rx_time_out = 100;

            /* Проверка прихода подтверждения перехода в режим ожидания */
            tick = GetTickCount();
            ack = ~stop_ack;
            do
                {
                rd_cnt = (long)LTR_Recv(&hnd->Channel, rd_buf, NULL, RD_BUFSZ, rx_time_out);
                if (rd_cnt > 0)                     /* есть принятые данные */
                    {
                    /* выделение подтверждения */
                    ack = rd_buf[rd_cnt - 1] & ack_mask;
                    data_received = 1;
                    }
                elapsed_time = f_elapsed_time(tick);
                } while ((elapsed_time <= ack_tm_out) && (rd_cnt >= 0) && (ack != stop_ack));
            if (!data_received)                     /* от модуля ничего не пришло */
                {
                ret_val = LTR114_ERR_NOACK;
                }
            else if (ack != stop_ack)               /* неверное подтверждение */
                {
                ret_val = LTR114_ERR_INVALIDACK;
                }
            }
        } /*else @if (hnd->Channel == NULL)@*/

    return ret_val;
#undef RD_BUFSZ
    }

/*------------------------------------------------------------------------------------------------*/

INT LTR114_Calibrate
    (
        PTLTR114 hnd
    )
    /*выполнение одного цикла измерения модулем своих параметров и рассчет калибровочных коэффициентов
     *вызывается перед началом сбора данных, для того, чтобы данные могли сразу быть откалиброваны
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель состояния модуля.
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */
{
    DWORD cmd[3];
    INT res,i;
    DWORD *buf;
    double double_buf[9];
    int interv, qnt, features;
    int size = 2*LTR114_AUTOCALIBR_STEPS + 1;
    
    if (hnd == NULL)
        return LTR114_ERR_INVALID_DESCR;
    if (hnd->Active)
        return LTR114_ERR_ALREADY_RUN;
	if (hnd->Reserve == NULL)
		return LTR114_ERR_MODULE_CLOSED;

    /*если не выделена памать под измерения модулем своих параметров - выделение под одно измерение*/
    for (i=0; i < LTR114_ADC_RANGEQNT; i++)
    {
        if (hnd->AutoCalibrInfo[i].TempScale==NULL)
            hnd->AutoCalibrInfo[i].TempScale = (TSCALE_LTR114*)malloc((get_autocbr_cnt(hnd)+10) * sizeof(TSCALE_LTR114));
        hnd->AutoCalibrInfo[i].Index.NRef = hnd->AutoCalibrInfo[i].Index.Null = hnd->AutoCalibrInfo[i].Index.Ref =
            hnd->AutoCalibrInfo[i].Index.Interm = hnd->AutoCalibrInfo[i].Index.NInterm = 0;
    }


    //передача команды - провести измерение своих параметров
    cmd[0] = fill_command(test_cmd, LTR114_TEST_SELF_CALIBR);
    res = LTR_Send(&hnd->Channel, cmd, 1, send_tm_out);

    if (res < 0)
        return res;
    if (res != 1)
        return LTR_ERROR_SEND ;

    //прием данных - подтверждение + по 2 байта на измерение
    buf = (DWORD*)malloc(get_autocbr_cnt(hnd) * 2*LTR114_AUTOCALIBR_STEPS* sizeof(DWORD));
    size = get_autocbr_cnt(hnd) * 2*LTR114_AUTOCALIBR_STEPS;
    res = LTR_Recv(&hnd->Channel, buf, NULL, size, size*recv_tm_out);
    if (res < 0)
    {
        free(buf);
        return res;
    }
    if (res < size)
    {
        free(buf);
        return LTR_ERROR_RECV;
    }
  //  if ((buf[size - 1] & ack_mask)!=stop_ack)
 //       return LTR114_ERR_NOACK;

    res = LTR114_Stop(hnd);
    if (res!=LTR_OK)
    {
        free(buf);
        return res;
    }


    size--;
    interv = hnd->Interval;
    qnt = hnd->LChQnt;
    features = hnd->SpecialFeatures;

    /*обработка принятых данных с временной заменой параметров для правильной обработки*/
    hnd->Interval = LTR114_ADC_RANGEQNT*LTR114_AUTOCALIBR_STEPS;
    hnd->LChQnt = 0;
    hnd->SpecialFeatures  = 0;//|=LTR114_FEATURES_AUTOCBR;

    res = LTR114_ProcessData(hnd, buf, double_buf, &size, LTR114_CORRECTION_MODE_AUTO, 0);
    free(buf);
    if (res!=LTR_OK)
        return res;
    hnd->Interval = interv;
    hnd->LChQnt = qnt;
    hnd->SpecialFeatures = features;

	if (hnd->Reserve!=NULL)
		((PLTR114_INTERNAL_DATA)hnd->Reserve)->status |=LTR114_INTSTATUS_CALIBRATED;
    
    return LTR_OK;
    
}




INT LTR114_Recv(PTLTR114 hnd, DWORD *data, DWORD *tmark, DWORD size, DWORD timeout)
{
    return LTR_Recv(&hnd->Channel, data,tmark, size, timeout);
}

/*создание описателя логического канала по заданным параметрам*/
LTR114_LCHANNEL LTR114_CreateLChannel(INT MeasMode, INT Channel, INT Range)
{
    LTR114_LCHANNEL ch;
    ch.Channel = Channel;
    ch.MeasMode = MeasMode;
    ch.Range = Range;
    return ch;
}


/*------------------------------------------------------------------------------------------------*/

INT LTR114_CheckInputs
    (
        PTLTR114 hnd,
        INT channel_mask,
        INT check_mode,
        double *res_data,
        INT *size
    )
    /*выполнение проверки входных линий на обрыв и КЗ
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель состояния модуля
     *   ChannelsMask - маска каналов (биты 0 до 15 - 1 => канал используется)
     *   CheckMode - проврочный режим (варианты из LTR114_CHECKINPUTS_XXX)
     *   res_data - возвращаемый массив значений
     *   size - реальный размер возвращенных данных
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */
{
    TLTR114 hltr114;
    DWORD buf[LTR114_MAX_CHANNEL*3*2];
    INT i=0, cnt =0, res;
    
    if (hnd == NULL)
        return LTR114_ERR_INVALID_DESCR;
    if (hnd->Active)
        return LTR114_ERR_ALREADY_RUN;

    hltr114 = *hnd;
    hltr114.FreqDivider = ((LTR114_CLOCK*1000)/(LTR114_ADC_DIVIDER*200));
    hltr114.AdcOsr = 0;
    hltr114.SyncMode = LTR114_SYNCMODE_INTERNAL;
    hltr114.SpecialFeatures = 0;
    hltr114.Interval = 0;
    
    
    for (i=0; i < LTR114_MAX_CHANNEL; i++)
    {
        if (channel_mask & (1<<i))
        {
            if (check_mode & LTR114_CHECKMODE_X0Y0)
                hltr114.LChTbl[cnt++] = LTR114_CreateLChannel(LTR114_MEASMODE_X0Y0, i, LTR114_URANGE_10);
            if (check_mode & LTR114_CHECKMODE_X5Y0)
                hltr114.LChTbl[cnt++] = LTR114_CreateLChannel(LTR114_MEASMODE_X5Y0, i, LTR114_URANGE_10);
            if (check_mode & LTR114_CHECKMODE_X0Y5)
                hltr114.LChTbl[cnt++] = LTR114_CreateLChannel(LTR114_MEASMODE_X0Y5, i, LTR114_URANGE_10);
        }
    }
    if (cnt == 0)
        return LTR114_ERR_INVALID_ADCLCHQNT;
    hltr114.LChQnt = cnt;
    
    res = LTR114_SetADC(&hltr114);
    if (res!= LTR_OK)
        return res;

    res = LTR114_Calibrate(&hltr114);
    if (res!=LTR_OK)
        return res;

    
    res = LTR114_GetFrame(&hltr114, buf);
    if (res!= 2*cnt)
        return LTR114_ERR_GETFRAME;
    *size = 2*hltr114.LChQnt;
    res = LTR114_ProcessData(&hltr114, buf, res_data, size, LTR114_CORRECTION_MODE_INIT, LTR114_PROCF_VALUE);
    
    return res;
}


INT LTR114_SetRef
    (
        PTLTR114 hnd,
        INT range,
        BOOL middle
    )
    /* выдача опорного напряжения на внешние выводы
     * ПАРАМЕТРЫ
     *   hnd - указатель на описатель состояния модуля
     *   range - диапазон
     *   middle - если true - то точка середины шкалы, если false - то всей
     * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
     *   Код ошибки (см. заголовочный файл).
     */
{
    int res;
    TLTR114 hltr114;

    if (hnd == NULL)
        return LTR114_ERR_INVALID_DESCR;
    if (hnd->Active)
        return LTR114_ERR_ALREADY_RUN;

    
    hltr114 = *hnd;
    hltr114.FreqDivider = ((LTR114_CLOCK*1000)/(LTR114_ADC_DIVIDER*200));
    hltr114.AdcOsr = 0;
    hltr114.SyncMode = LTR114_SYNCMODE_INTERNAL;
    hltr114.SpecialFeatures = 0;
    hltr114.Interval = 0;
    hltr114.LChQnt = 1;
    if (middle)
        hltr114.LChTbl[0] = LTR114_CreateLChannel(LTR114_MEASMODE_DAC12_INTR_CBR, 13, range);
    else
        hltr114.LChTbl[0] = LTR114_CreateLChannel(LTR114_MEASMODE_DAC12_CBR, 13, range);
    res = LTR114_SetADC(&hltr114);
    if (res!= LTR_OK)
        return res;
    res = LTR114_Start(&hltr114);
    return res;
}


WORD LTR114_GetDllVer()
{
    return DLL_VER;
}
