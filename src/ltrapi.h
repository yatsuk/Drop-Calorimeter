//-----------------------------------------------------------------------------
// common part
//-----------------------------------------------------------------------------
#ifndef __ltrapi__
#define __ltrapi__
#include <windows.h>
#include "ltrapidefine.h"
#include "ltrapitypes.h"


// коды ошибок
#define LTR_OK                     ( 0l)     /*Выполнено без ошибок.*/
#define LTR_ERROR_UNKNOWN          (-1l)     /*Неизвестная ошибка.*/
#define LTR_ERROR_PARAMETERS       (-2l)     /*Ошибка входных параметров.*/
#define LTR_ERROR_PARAMETRS        LTR_ERROR_PARAMETERS
#define LTR_ERROR_MEMORY_ALLOC     (-3l)     /*Ошибка динамического выделения памяти.*/
#define LTR_ERROR_OPEN_CHANNEL     (-4l)     /*Ошибка открытия канала обмена с сервером.*/
#define LTR_ERROR_OPEN_SOCKET      (-5l)     /*Ошибка открытия сокета.*/
#define LTR_ERROR_CHANNEL_CLOSED   (-6l)     /*Ошибка. Канал обмена с сервером не создан.*/
#define LTR_ERROR_SEND             (-7l)     /*Ошибка отправления данных.*/
#define LTR_ERROR_RECV             (-8l)     /*Ошибка приема данных.*/
#define LTR_ERROR_EXECUTE          (-9l)     /*Ошибка обмена с крейт-контроллером.*/
#define LTR_WARNING_MODULE_IN_USE  (-10l)    /*Канал обмена с сервером создан в текущей программе
                                                и в какой - то еще*/
#define LTR_ERROR_NOT_CTRL_CHANNEL (-11l)    /* Номер канала для этой операции должен быть CC_CONTROL */

#define LTR_DEFAULT_SEND_RECV_TIMEOUT   10000UL

// Значения для управления ножками процессора, доступными для пользовательского программирования
enum en_LTR_UserIoCfg
    {
    LTR_USERIO_DIGIN1   = 1,    // ножка является входом и подключена к DIGIN1
    LTR_USERIO_DIGIN2   = 2,    // ножка является входом и подключена к DIGIN2
    LTR_USERIO_DIGOUT   = 0,    // ножка является выходом (подключение см. en_LTR_DigOutCfg)
    LTR_USERIO_DEFAULT  = LTR_USERIO_DIGOUT
    };

// Значения для управления выходами DIGOUTx
enum en_LTR_DigOutCfg
    {
    LTR_DIGOUT_CONST0   = 0x00, // постоянный уровень логического "0"
    LTR_DIGOUT_CONST1   = 0x01, // постоянный уровень логической "1"
    LTR_DIGOUT_USERIO0  = 0x02, // выход подключен к ножке userio0 (PF1 в рев. 0, PF1 в рев. 1)
    LTR_DIGOUT_USERIO1  = 0x03, // выход подключен к ножке userio1 (PG13)
    LTR_DIGOUT_DIGIN1   = 0x04, // выход подключен ко входу DIGIN1
    LTR_DIGOUT_DIGIN2   = 0x05, // выход подключен ко входу DIGIN2
    LTR_DIGOUT_START    = 0x06, // на выход подаются метки "СТАРТ"
    LTR_DIGOUT_SECOND   = 0x07, // на выход подаются метки "СЕКУНДА"
    LTR_DIGOUT_DEFAULT  = LTR_DIGOUT_CONST0
    };

// Значения для управления метками "СТАРТ" и "СЕКУНДА"
enum en_LTR_MarkMode
    {
    LTR_MARK_OFF                = 0x00, // метка отключена
    LTR_MARK_EXT_DIGIN1_RISE    = 0x01, // метка по фронту DIGIN1
    LTR_MARK_EXT_DIGIN1_FALL    = 0x02, // метка по спаду DIGIN1
    LTR_MARK_EXT_DIGIN2_RISE    = 0x03, // метка по фронту DIGIN2
    LTR_MARK_EXT_DIGIN2_FALL    = 0x04, // метка по спаду DIGIN2
    LTR_MARK_INTERNAL           = 0x05  // внутренняя генерация метки
    };

//-----------------------------------------------------------------------------


typedef struct 
    {
    DWORD saddr;                       // сетевой адрес сервера
    WORD  sport;                       // сетевой порт сервера
    CHAR  csn[16];                     // серийный номер крейта
    WORD  cc;                          // номер канала крейта
    DWORD flags;                       // флаги состояния канала
    DWORD tmark;                       // последняя принятая метка времени
    //                                 //
    LPVOID internal;                   // указатель на канал
    }
    TLTR;

// Структура конфигурации LTR-EU
typedef struct
    {
    // Настройка ножек процессора
    // [0] PF1 в рев. 0, PF1 в рев. 1+
    // [1] PG13
    // [2] PF3, только рев. 1+, только вход
    // [3] резерв
    WORD        userio[4];          // одно из значений LTR_USERIO_...

    // Настройка выходов DIGOUTx
    WORD        digout[2];          // конфигурация выходов (LTR_DIGOUT_...)
    WORD        digout_en;          // разрешение выходов DIGOUT1, DIGOUT2
    }
    TLTR_CONFIG;


//-----------------------------------------------------------------------------

// -- Функции для внутреннего применения
INT LTR__GenericCtlFunc
    (
    TLTR* ltr,                          // дескриптор LTR
    LPCVOID request_buf,                // буфер с запросом
    DWORD request_size,                 // длина запроса (в байтах)
    LPVOID reply_buf,                   // буфер для ответа (или NULL)
    DWORD reply_size,                   // длина ответа (в байтах)
    INT ack_error_code,                 // какая ошибка, если ack не GOOD (0 = не использовать ack)
    DWORD timeout                       // таймаут, мс
    );

// -- Функции общего назначения

// Инициализация управляющей структуры TLTR
INT    LTR_Init(TLTR *ltr);
// Открытие соединения с крейтом
INT    LTR_Open(TLTR *ltr);
// Закрытие соединения с крейтом
INT    LTR_Close(TLTR *ltr);
// Проверка, установлено ли соединение с крейтом
INT    LTR_IsOpened(TLTR *ltr);
// Получение текстового сообщения об ошибке по ее коду
LPCSTR LTR_GetErrorString(INT error);

// -- Функции низкого уровня

// Прием данных из крейта
INT    LTR_Recv(TLTR *ltr, DWORD *data, DWORD *tmark, DWORD size, DWORD timeout);
// Посылка данных в крейт
INT    LTR_Send(TLTR *ltr, DWORD *data, DWORD size, DWORD timeout);

// -- Функции, работающие с управляющим каналом крейт-контроллера (должен быть открыт CC_CONTROL)

// Получение списка крейтов, подключенных к серверу
INT    LTR_GetCrates(TLTR *ltr, BYTE *csn);
// Получение списка модулей в крейте
INT    LTR_GetCrateModules(TLTR *ltr, WORD *mid);
// Управление приоритетом процесса ltrserver.exe
INT    LTR_SetServerProcessPriority(TLTR *ltr, DWORD Priority);
// Чтение описания крейта (модель, тип интерфейса)
INT    LTR_GetCrateInfo(TLTR *ltr, TCRATE_INFO * CrateInfo);
// Получение сырых данных из крейта (если был установлен CC_RAW_DATA_FLAG)
INT    LTR_GetCrateRawData(TLTR *ltr, DWORD * data, DWORD * tmark, DWORD size, DWORD timeout);
// Задание конфигурации цифровых входов и выходов крейта
INT    LTR_Config(TLTR *ltr, const TLTR_CONFIG *conf);
// Включение внутренней генерации секундных меток
INT    LTR_StartSecondMark(TLTR *ltr, enum en_LTR_MarkMode mode);
// Отключение внутренней генерации секундных меток
INT    LTR_StopSecondMark(TLTR *ltr);
// Подача метки СТАРТ (однократно)
INT    LTR_MakeStartMark(TLTR *ltr, enum en_LTR_MarkMode mode);
// Чтение приоритета процесса ltrserver.exe
INT    LTR_GetServerProcessPriority(TLTR *ltr, DWORD* Priority);

// -- Функции управления сервером (работают по управляющему каналу,
//    в т.ч. без крейта через особый серийный номер CSN_SERVER_CONTROL

// Чтение номера версии сервера (DWORD, 0x01020304 = 1.2.3.4)
INT    LTR_GetServerVersion(TLTR *ltr, DWORD *version);

// Получение списка известных серверу IP-крейтов (подключенных и не подключенных)
// Соответствует содержимому окна сервера "Управление IP-крейтами".
INT    LTR_GetListOfIPCrates(TLTR *ltr,
    DWORD max_entries, DWORD ip_net, DWORD ip_mask,
    DWORD *entries_found, DWORD *entries_returned,
    TIPCRATE_ENTRY *info_array);

// Добавление IP-адреса крейта в таблицу (с сохранением в файле конфигурации).
// Если адрес уже существует, возвращает ошибку.
INT    LTR_AddIPCrate(TLTR *ltr, DWORD ip_addr, DWORD flags, BOOL permanent);

// Удаление IP-адреса крейта из таблицы (и из файла конфигурации, если он статический).
// Если адреса нет, либо крейт занят, возвращает ошибку.
INT    LTR_DeleteIPCrate(TLTR *ltr, DWORD ip_addr, BOOL permanent);

// Установление соединения с IP-крейтом (адрес должен быть в таблице)
// Если адреса нет, возвращает ошибку. Если крейт уже подключен или идет попытка подключения,
// возвращает LTR_OK.
// Если вернулось LTR_OK, это значит, что ОТПРАВЛЕНА КОМАНДА установить соединение.
// Результат можно отследить позже по LTR_GetCrates() или LTR_GetListOfIPCrates()
INT    LTR_ConnectIPCrate(TLTR *ltr, DWORD ip_addr);

// Завершение соединения с IP-крейтом (адрес должен быть в таблице)
// Если адреса нет, возвращает ошибку. Если крейт уже отключен, возвращает LTR_OK.
// Если вернулось LTR_OK, это значит, что ОТПРАВЛЕНА КОМАНДА завершить соединение.
// Результат можно отследить позже по LTR_GetCrates() или LTR_GetListOfIPCrates()
INT    LTR_DisconnectIPCrate(TLTR *ltr, DWORD ip_addr);

// Установление соединения со всеми IP-крейтами, имеющими флаг "автоподключение"
// Если вернулось LTR_OK, это значит, что ОТПРАВЛЕНА КОМАНДА установить соединение.
// Результат можно отследить позже по LTR_GetCrates() или LTR_GetListOfIPCrates()
INT    LTR_ConnectAllAutoIPCrates(TLTR *ltr);

// Завершение соединения со всеми IP-крейтами
// Если вернулось LTR_OK, это значит, что ОТПРАВЛЕНА КОМАНДА завершить соединение.
// Результат можно отследить позже по LTR_GetCrates() или LTR_GetListOfIPCrates()
INT    LTR_DisconnectAllIPCrates(TLTR *ltr);

// Изменение флагов для IP-крейта
INT    LTR_SetIPCrateFlags(TLTR *ltr, DWORD ip_addr, DWORD new_flags, BOOL permanent);

// Чтение режима поиска IP-крейтов в локальной сети
INT    LTR_GetIPCrateDiscoveryMode(TLTR *ltr, BOOL *enabled, BOOL *autoconnect);

// Установка режима поиска IP-крейтов в локальной сети
INT    LTR_SetIPCrateDiscoveryMode(TLTR *ltr, BOOL enabled, BOOL autoconnect, BOOL permanent);

// Чтение уровня журнализации
INT    LTR_GetLogLevel(TLTR *ltr, INT *level);

// Установка уровня журнализации
INT    LTR_SetLogLevel(TLTR *ltr, INT level, BOOL permanent);

// Рестарт программы ltrserver (с разрывом всех соединений).
// После успешного выполнения команды связь с сервером теряется.
INT    LTR_ServerRestart(TLTR *ltr);

// Рестарт программы ltrserver (с разрывом всех соединений)
// После успешного выполнения команды связь с сервером теряется.
INT    LTR_ServerShutdown(TLTR *ltr);

// Установка таймаута по умолчанию для send/recv
// Используется в LTR_Send и LTR_Recv, если задан таймаут 0,
// а также во всех командах управляющего канала.
INT    LTR_SetTimeout(TLTR *ltr, DWORD ms);

#endif


