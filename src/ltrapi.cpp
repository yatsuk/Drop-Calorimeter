//---------------------------------------------------------------------------
#include <windows.h>
#include "channel.h"
#include "ltrapi.h"

//#include <QDebug>

static const LPCSTR UnknownErrorString = "Неизвестная ошибка.";
static const T_ERROR_STRING_DEF ErrorStrings[] = {
    { LTR_OK,                       "Выполнено без ошибок." },
    { LTR_ERROR_PARAMETERS,         "Ошибка входных параметров." },
    { LTR_ERROR_MEMORY_ALLOC,       "Ошибка динамического выделения памяти." },
    { LTR_ERROR_OPEN_CHANNEL,       "Ошибка открытия канала обмена с сервером." },
    { LTR_ERROR_OPEN_SOCKET,        "Ошибка открытия канала." } ,
    { LTR_ERROR_CHANNEL_CLOSED,     "Ошибка: канал обмена с сервером не создан." },
    { LTR_ERROR_SEND,               "Ошибка передачи данных." },
    { LTR_ERROR_RECV,               "Ошибка приема данных." },
    { LTR_ERROR_EXECUTE,            "Ошибка обмена с крейт-контроллером." },
    { LTR_WARNING_MODULE_IN_USE,    "Предупреждение: канал с сервером для этого модуля уже создан." },
    { LTR_ERROR_NOT_CTRL_CHANNEL,   "Номер канала для этой операции должен быть равен CC_CONTROL." }
};

#define OPEN_RETRY_TIMEOUT      3000

//-----------------------------------------------------------------------------
// Общий код для выполнения управляющей функции.
// Посылается запрос, принимается слово ACK и затем ответ.
static INT f_StdCtlFunc
(
        TLTR* ltr,                          // дескриптор LTR
        LPCVOID request_buf,                // буфер с запросом
        DWORD request_size,                 // длина запроса (в байтах)
        LPVOID reply_buf,                   // буфер для ответа (или NULL)
        DWORD reply_size,                   // длина ответа (в байтах)
        BOOL timer_started = FALSE,         // если TRUE, не запускать таймер
        INT ack_error_code                  // какая ошибка, если ack не GOOD (0 = не использовать ack)
        = LTR_ERROR_EXECUTE
        )
{
    if (!ltr || !request_buf || !request_size) return LTR_ERROR_PARAMETERS;
    if ((reply_size > 0) && !reply_buf) return LTR_ERROR_PARAMETERS;
    TChannel* channel = (TChannel*)ltr->internal;
    if (!channel) return LTR_ERROR_CHANNEL_CLOSED;

    if (ltr->cc != CC_CONTROL)
        return LTR_ERROR_NOT_CTRL_CHANNEL;

    INT res;
    if (!timer_started)
        channel->SetTimerDefault();

    res = channel->Send((LPBYTE)request_buf, request_size);
    if (res < 0) return res;
    if (res != INT(request_size)) return LTR_ERROR_SEND;

    if (ack_error_code != 0)
    {
        DWORD ack = 0;
        res = channel->Recv((LPBYTE)&ack, sizeof(ack));
        if (res < 0) return res;
        if (res != INT(sizeof(ack))) return LTR_ERROR_RECV;
        if (ack != CONTROL_ACKNOWLEDGE_GOOD)
            return ack_error_code;
    }

    if (reply_size > 0)
    {
        res = channel->Recv((LPBYTE)reply_buf, reply_size);
        if (res < 0) return res;
        if (res != INT(reply_size)) return LTR_ERROR_RECV;
    }
    return LTR_OK;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
static INT f_SetFPGAConfigRegisters(TLTR* ltr, WORD first_reg, UINT size, LPCVOID values)
{
    if (!values) return LTR_ERROR_PARAMETERS;
    DWORD sbuf[32] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_PUT_ARRAY,
        (SEL_FPGA_DATA | first_reg), size
    };
    size_t send_size = 4 * sizeof(DWORD) + size;
    if (send_size > sizeof(sbuf)) return LTR_ERROR_PARAMETERS;
    memcpy(&sbuf[4], values, size);
    return f_StdCtlFunc(ltr, sbuf, send_size, NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         основные функции для работы с ltr-сервером
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_Init(TLTR* ltr)
{
    if (!ltr) return LTR_ERROR_PARAMETERS;

    for(int i = 0; i < 16;++i)
        ltr->csn[i]=0x00;


    ltr->saddr = SADDR_DEFAULT;
    ltr->sport = SPORT_DEFAULT;
    ltr->cc = CC_CONTROL;
    ltr->flags = 0;
    ltr->tmark = -1;
    ltr->internal = NULL;
    return LTR_OK;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_Open(TLTR* ltr)
{
    INT res = LTR_Close(ltr);
    if (res < 0) return res;

    TChannel* channel = new TChannel;
    if (!channel) return LTR_ERROR_MEMORY_ALLOC;

    // Открываем соединение с сервером
    if (!channel->Open(ltr->saddr, ltr->sport, OPEN_RETRY_TIMEOUT))
    {
        delete channel;
        return LTR_ERROR_OPEN_SOCKET;
    }

    DWORD ack = 0;
    BYTE buf[8 + 16 + 2 + 4];
    ZeroMemory(buf, sizeof(buf));
    *(DWORD*)(buf + 0) = CONTROL_COMMAND_START;
    *(DWORD*)(buf + 4) = CONTROL_COMMAND_INIT_CONNECTION;
    strncpy(reinterpret_cast <char *>(buf) + 8, ltr->csn, 16);
    *(WORD*)(buf + 8 + 16) = ltr->cc;

    channel->SetTimerDefault();

    // Посылаем команду инициализации соединения
    if ((channel->Send(buf, sizeof(buf)) != INT(sizeof(buf)))
            || (channel->Recv((LPBYTE)&ack, sizeof(ack)) != INT(sizeof(ack)))
            || ((ack != CONTROL_ACKNOWLEDGE_GOOD) && (ack != CONTROL_ACKNOWLEDGE_MODULE_IN_USE))
            || (channel->Recv(buf, sizeof(buf)) != INT(sizeof(buf))))
    {
        delete channel;
        return LTR_ERROR_OPEN_SOCKET;
    }

    // В полученном отклике сервер заполняет секундные метки
    // и серийный номер открытого крейта
    ltr->tmark = *(DWORD*)(buf + 8 + 16 + 2);
    if (!ltr->csn[0])
    { // Серийный номер не был задан, годится любой
        memcpy(ltr->csn, buf + 8, 16);
    }
    else if (strncmp(ltr->csn, reinterpret_cast<char *>(buf) + 8, 16))
    { // Серийный номер почему-то не совпадает
        delete channel;
        return LTR_ERROR_OPEN_CHANNEL;
    }

    // Ошибки (фатальной) нет, возвращаем созданный объект в структуру
    // (после этого она станет LTR_IsOpened())
    ltr->internal = (LPVOID)channel;

    return (ack == CONTROL_ACKNOWLEDGE_MODULE_IN_USE)
            ? LTR_WARNING_MODULE_IN_USE
            : LTR_OK;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_Close(TLTR* ltr)
{
    if (!ltr) return LTR_ERROR_PARAMETERS;
    TChannel* channel = (TChannel*)ltr->internal;
    if (channel)
    {
        delete channel;
        ltr->internal = NULL;
    }
    return LTR_OK;

}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_IsOpened(TLTR* ltr)
{
    if (!ltr) return LTR_ERROR_PARAMETERS;
    return (ltr->internal) ? LTR_OK : LTR_ERROR_CHANNEL_CLOSED;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_Recv(TLTR* ltr, DWORD* data, DWORD* tmark, DWORD size, DWORD timeout)
{
    if (!ltr || !data || !size) return LTR_ERROR_PARAMETERS;
    TChannel* channel = (TChannel*)ltr->internal;
    if (!channel) return LTR_ERROR_CHANNEL_CLOSED;

    ltr->flags &= ~FLAG_RBUF_OVF;
    DWORD num_recv = 0;
    INT xfer_size;

    if (timeout) { channel->SetTimer(timeout); }
    else { channel->SetTimerDefault(); }

    if  ((ltr->cc & (~CC_DEBUG_FLAG)) == CC_CONTROL)
    { // Управляющий канал. Чтение простое, без преобразований.
        xfer_size = channel->Recv((LPBYTE)data, size * sizeof(DWORD));
        if (xfer_size < 0) return xfer_size; // код ошибки
        num_recv = xfer_size / sizeof(DWORD);
    }
    else
    { // Канал данных. Нужно учитывать и пропускать метки времени.
        DWORD align_ofs = 0;
        while ((num_recv < size) && !channel->TimerExpired())
        {
            xfer_size = channel->Recv((LPBYTE)data + align_ofs,
                                      (size - num_recv) * sizeof(DWORD) - align_ofs);
            if (xfer_size < 0) return xfer_size; // код ошибки
            xfer_size += align_ofs; // всего байт в буфере, начиная с data
            DWORD words_in = xfer_size / sizeof(DWORD);
            DWORD words_out = 0;
            for (DWORD i = 0; i < words_in; i++)
            {
                DWORD data_word = data[i];
                if (!(ltr->flags & FLAG_SIGNAL_TSTAMP))
                {
                    switch (data_word)
                    {
                    case SIGNAL_RBUF_OVF:
                        ltr->flags |= FLAG_RBUF_OVF;
                        break;
                    case SIGNAL_TSTAMP:
                        ltr->flags |= FLAG_SIGNAL_TSTAMP;
                        break;
                    default:
                        if (tmark)
                            tmark[words_out] = ltr->tmark;
                        data[words_out++] = data_word;
                        break;
                    }
                }
                else /* (ltr->flags & FLAG_SIGNAL_TSTAMP) */
                {
                    ltr->tmark = data_word;
                    ltr->flags &= ~FLAG_SIGNAL_TSTAMP;
                }
            } /* for i */

            // Если размер не кратен двойному слову, то сохранить хвост
            // от 0 до 3 байт по адресу data[words_in]
            align_ofs = xfer_size % sizeof(DWORD);
            if ((align_ofs > 0) && (words_out != words_in))
            { // Копируем двойное слово, содержащее этот хвост
                data[words_out] = data[words_in];
            }

            num_recv += words_out;
            data += words_out;
            if (tmark) tmark += words_out;
        }
    }

    return num_recv;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_Send(TLTR* ltr, DWORD* data, DWORD size, DWORD timeout)
{
    if (!ltr || !data || !size) return LTR_ERROR_PARAMETERS;
    TChannel* channel = (TChannel*)ltr->internal;
    if (!channel) return LTR_ERROR_CHANNEL_CLOSED;

    INT xfer_size;

    if (timeout) { channel->SetTimer(timeout); }
    else { channel->SetTimerDefault(); }

    xfer_size = channel->Send((LPBYTE)data, size * sizeof(DWORD));
    if (xfer_size < 0) return xfer_size; // код ошибки

    return xfer_size / sizeof(DWORD);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// КОМАНДЫ УПРАВЛЯЮЩЕГО КАНАЛА
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_GetCrates(TLTR* ltr, BYTE* csn)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_GET_CRATES
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), csn, CRATE_MAX * 16);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_SetServerProcessPriority(TLTR* ltr, DWORD Priority)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_SET_SERVER_PROCESS_PRIORITY
    };
    // Функция исторически реализована не так, как остальные, с двумя посылками и двумя ACK.
    INT res = f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
    if (res != LTR_OK) return res;
    return f_StdCtlFunc(ltr, &Priority, sizeof(Priority), NULL, 0, TRUE);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_GetCrateModules(TLTR* ltr, WORD* mid)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_GET_MODULES
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), mid, MODULE_MAX * 2);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
LPCSTR LTR_GetErrorString(INT error)
{
    for (size_t i = 0; i < sizeof(ErrorStrings) / sizeof(ErrorStrings[0]); i++)
    {
        if (ErrorStrings[i].code == error)
            return ErrorStrings[i].message;
    }
    return UnknownErrorString;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_GetCrateRawData(TLTR*ltr, DWORD* data, DWORD* tmark, DWORD size, DWORD timeout)
{
    if (size)
    {
        DWORD sbuf[] =
        {
            CONTROL_COMMAND_START, CONTROL_COMMAND_GET_CRATE_RAW_DATA,
            size, timeout, ltr->flags & FLAG_RFULL_DATA
        };
        INT res = f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0, FALSE, 0);
        if (res != LTR_OK) return res;
        return LTR_Recv(ltr, data, tmark, size, timeout);
    }
    else
    {
        DWORD sbuf[] =
        {
            CONTROL_COMMAND_START, CONTROL_COMMAND_GET_CRATE_RAW_DATA_SIZE
        };
        DWORD SizeRecv;
        INT res = f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), &SizeRecv, sizeof(SizeRecv), FALSE, 0);
        if (res != LTR_OK) return res;
        return SizeRecv;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
INT LTR_GetCrateInfo(TLTR* ltr, TCRATE_INFO* CrateInfo)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_GET_CRATE_TYPE
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), CrateInfo, sizeof(TCRATE_INFO));
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Задание конфигурации цифровых входов и выходов крейта
INT LTR_Config(TLTR* ltr, const TLTR_CONFIG* conf)
{
    WORD reg_val[5];
    if (!conf)
        return LTR_ERROR_PARAMETERS;
    // регистр 0x09: бит 0 = digout_oe, бит 1 = userio0_oe, бит 2 = userio1_oe
    reg_val[0] = ((conf->digout_en != 0) ? 0x01 : 0x00)
            | ((conf->userio[0] != LTR_USERIO_DIGOUT) ? 0x02 : 0x00)
            | ((conf->userio[1] != LTR_USERIO_DIGOUT) ? 0x04 : 0x00);
    // регистр 0x0A: селектор DIGOUT1
    reg_val[1] = conf->digout[0];
    // регистр 0x0B: селектор DIGOUT2
    reg_val[2] = conf->digout[1];
    // регистр 0x0C: бит 0 = 0: userio0 = DIGIN1, 1: userio0 = DIGIN2
    //               бит 4 = 0: userio2 = DIGIN1, 1: userio2 = DIGIN2
    reg_val[3] = ((conf->userio[0] == LTR_USERIO_DIGIN2) ? 0x01 : 0x00)
            | ((conf->userio[2] == LTR_USERIO_DIGIN2) ? 0x10 : 0x00);
    // регистр 0x0D: бит 0 = 0: userio1 = DIGIN1, 1: userio1 = DIGIN2
    reg_val[4] = ((conf->userio[1] == LTR_USERIO_DIGIN2) ? 0x01 : 0x00);

    return f_SetFPGAConfigRegisters(ltr, 0x09, sizeof(reg_val), reg_val);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Включение внутренней генерации секундных меток
INT LTR_StartSecondMark(TLTR* ltr, enum en_LTR_MarkMode mode)
{
    WORD reg_val = mode;
    if (mode == LTR_MARK_INTERNAL)
        reg_val++;
    return f_SetFPGAConfigRegisters(ltr, 0x0F, sizeof(reg_val), &reg_val);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Отключение внутренней генерации секундных меток
INT LTR_StopSecondMark(TLTR* ltr)
{
    WORD reg_val = 0x00;
    return f_SetFPGAConfigRegisters(ltr, 0x0F, sizeof(reg_val), &reg_val);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Подача метки СТАРТ (однократно)
INT LTR_MakeStartMark(TLTR* ltr, enum en_LTR_MarkMode mode)
{
    WORD reg_val = mode;
    return f_SetFPGAConfigRegisters(ltr, 0x0E, sizeof(reg_val), &reg_val);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Чтение номера версии сервера (DWORD, 0x01020304 = 1.2.3.4)
INT LTR_GetServerVersion(TLTR* ltr, DWORD* version)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_GET_SERVER_VERSION
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), version, sizeof(DWORD));
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Получение списка известных серверу IP-крейтов (подключенных и не подключенных)
// Соответствует содержимому окна сервера "Управление IP-крейтами".
INT LTR_GetListOfIPCrates(TLTR* ltr, DWORD max_entries, DWORD ip_net, DWORD ip_mask,
                          DWORD* entries_found, DWORD* entries_returned, TIPCRATE_ENTRY* info_array)
{
    if (!ltr) return LTR_ERROR_PARAMETERS;
    TChannel* channel = (TChannel*)ltr->internal;
    if (!channel) return LTR_ERROR_CHANNEL_CLOSED;

    if (entries_found) *entries_found = 0;
    if (entries_returned) *entries_returned = 0;
    if (!info_array)
        max_entries = 0;
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_GET_LIST,
        max_entries, ip_net, ip_mask
    };
    DWORD rbuf[2];

    channel->SetTimerDefault();

    INT res = f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), rbuf, sizeof(rbuf), TRUE);
    if (res != LTR_OK) return res;

    if (rbuf[1] > max_entries)
    {
        res = LTR_ERROR_RECV;
        rbuf[1] = max_entries;
    }
    DWORD read_bytes = rbuf[1] * sizeof(TIPCRATE_ENTRY);
    if (entries_found) *entries_found = rbuf[0];
    if (entries_returned) *entries_returned = rbuf[1];
    if (read_bytes > 0)
    {
        if (channel->Recv((LPBYTE)info_array, read_bytes) != INT(read_bytes))
            res = LTR_ERROR_RECV;
    }
    return res;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Добавление IP-адреса крейта в таблицу (с сохранением в файле конфигурации).
// Если адрес уже существует, возвращает ошибку.
INT LTR_AddIPCrate(TLTR* ltr, DWORD ip_addr, DWORD flags, BOOL permanent)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_ADD_LIST_ENTRY,
        ip_addr, flags, permanent
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Удаление IP-адреса крейта из таблицы (и из файла конфигурации, если он статический).
// Если адреса нет, либо крейт занят, возвращает ошибку.
INT LTR_DeleteIPCrate(TLTR* ltr, DWORD ip_addr, BOOL permanent)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_DELETE_LIST_ENTRY,
        ip_addr, permanent
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Установление соединения с IP-крейтом (адрес должен быть в таблице)
// Если адреса нет, возвращает ошибку. Если крейт уже подключен или идет попытка подключения,
// возвращает LTR_OK.
// Если вернулось LTR_OK, это значит, что ОТПРАВЛЕНА КОМАНДА установить соединение.
// Результат можно отследить позже по LTR_GetCrates() или LTR_GetListOfIPCrates()
INT LTR_ConnectIPCrate(TLTR* ltr, DWORD ip_addr)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_CONNECT,
        ip_addr
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Завершение соединения с IP-крейтом (адрес должен быть в таблице)
// Если адреса нет, возвращает ошибку. Если крейт уже отключен, возвращает LTR_OK.
// Если вернулось LTR_OK, это значит, что ОТПРАВЛЕНА КОМАНДА завершить соединение.
// Результат можно отследить позже по LTR_GetCrates() или LTR_GetListOfIPCrates()
INT LTR_DisconnectIPCrate(TLTR* ltr, DWORD ip_addr)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_DISCONNECT,
        ip_addr
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Установление соединения со всеми IP-крейтами, имеющими флаг "автоподключение"
// Если вернулось LTR_OK, это значит, что ОТПРАВЛЕНА КОМАНДА установить соединение.
// Результат можно отследить позже по LTR_GetCrates() или LTR_GetListOfIPCrates()
INT LTR_ConnectAllAutoIPCrates(TLTR* ltr)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_CONNECT_ALL_AUTO
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Завершение соединения со всеми IP-крейтами
// Если вернулось LTR_OK, это значит, что ОТПРАВЛЕНА КОМАНДА завершить соединение.
// Результат можно отследить позже по LTR_GetCrates() или LTR_GetListOfIPCrates()
INT LTR_DisconnectAllIPCrates(TLTR* ltr)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_DISCONNECT_ALL
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Изменение флагов для IP-крейта
INT LTR_SetIPCrateFlags(TLTR* ltr, DWORD ip_addr, DWORD new_flags, BOOL permanent)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_SET_FLAGS,
        ip_addr, new_flags, permanent
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Чтение приоритета процесса ltrserver.exe
INT LTR_GetServerProcessPriority(TLTR* ltr, DWORD* Priority)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_GET_SERVER_PROCESS_PRIORITY
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), Priority, sizeof(DWORD));
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Чтение режима поиска IP-крейтов в локальной сети
INT LTR_GetIPCrateDiscoveryMode(TLTR* ltr, BOOL* enabled, BOOL* autoconnect)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_GET_DISCOVERY_MODE
    };
    DWORD rbuf[2];
    INT res = f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), rbuf, sizeof(rbuf));
    if (res != LTR_OK) return res;
    if (enabled)
        *enabled = (rbuf[0] != 0);
    if (autoconnect)
        *autoconnect = (rbuf[1] != 0);
    return res;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Установка режима поиска IP-крейтов в локальной сети
INT LTR_SetIPCrateDiscoveryMode(TLTR* ltr, BOOL enabled, BOOL autoconnect, BOOL permanent)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_IP_SET_DISCOVERY_MODE,
        enabled, autoconnect, permanent
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Чтение уровня журнализации
INT LTR_GetLogLevel(TLTR* ltr, INT* level)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_GET_LOG_LEVEL
    };
    DWORD rbuf[1];
    INT res = f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), rbuf, sizeof(rbuf));
    if (res != LTR_OK) return res;
    if (level)
        *level = rbuf[0];
    return res;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Установка уровня журнализации
INT LTR_SetLogLevel(TLTR* ltr, INT level, BOOL permanent)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_SET_LOG_LEVEL,
        level, permanent
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Рестарт программы ltrserver (с разрывом всех соединений).
// После успешного выполнения команды связь с сервером теряется.
INT LTR_ServerRestart(TLTR* ltr)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_RESTART_SERVER
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Рестарт программы ltrserver (с разрывом всех соединений)
// После успешного выполнения команды связь с сервером теряется.
INT LTR_ServerShutdown(TLTR* ltr)
{
    DWORD sbuf[] =
    {
        CONTROL_COMMAND_START, CONTROL_COMMAND_SHUTDOWN_SERVER
    };
    return f_StdCtlFunc(ltr, sbuf, sizeof(sbuf), NULL, 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Установка таймаута по умолчанию для send/recv
// Используется в LTR_Send и LTR_Recv, если задан таймаут 0,
// а также во всех командах управляющего канала.
INT LTR_SetTimeout(TLTR* ltr, DWORD ms)
{
    if (!ltr) return LTR_ERROR_PARAMETERS;
    TChannel* channel = (TChannel*)ltr->internal;
    if (!channel) return LTR_ERROR_CHANNEL_CLOSED;
    channel->SetDefaultTimeout(ms);
    return LTR_OK;
}

//-----------------------------------------------------------------------------
// Интерфейс к низкоуровневой функции для ltrbootapi и т.п.
INT LTR__GenericCtlFunc
(
        TLTR* ltr,                          // дескриптор LTR
        LPCVOID request_buf,                // буфер с запросом
        DWORD request_size,                 // длина запроса (в байтах)
        LPVOID reply_buf,                   // буфер для ответа (или NULL)
        DWORD reply_size,                   // длина ответа (в байтах)
        INT ack_error_code,                 // какая ошибка, если ack не GOOD (0 = не использовать ack)
        DWORD timeout                       // таймаут, мс
        )
{
    if (!ltr) return LTR_ERROR_PARAMETERS;
    TChannel* channel = (TChannel*)ltr->internal;
    if (!channel) return LTR_ERROR_CHANNEL_CLOSED;

    channel->SetTimer(timeout);
    return f_StdCtlFunc(ltr, request_buf, request_size, reply_buf, reply_size,
                        TRUE, ack_error_code);
}
//-----------------------------------------------------------------------------

