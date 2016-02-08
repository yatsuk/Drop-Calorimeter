
#ifndef __LTR34API__
#define __LTR34API__

#include "ltrapitypes.h"
#include "ltrapi.h"

	// константы
#define LTR34_ERROR_SEND_DATA				(-3001)
#define LTR34_ERROR_RECV_DATA				(-3002)
#define LTR34_ERROR_RESET_MODULE			(-3003)
#define LTR34_ERROR_NOT_LTR34				(-3004)
#define LTR34_ERROR_CRATE_BUF_OWF			(-3005)
#define LTR34_ERROR_PARITY					(-3006)
#define LTR34_ERROR_OVERFLOW				(-3007)
#define LTR34_ERROR_INDEX					(-3008)
	//
#define LTR34_ERROR							(-3009)
#define LTR34_ERROR_EXCHANGE				(-3010)
#define LTR34_ERROR_FORMAT					(-3011)
#define LTR34_ERROR_PARAMETERS				(-3012)
#define LTR34_ERROR_ANSWER					(-3013)
#define LTR34_ERROR_WRONG_FLASH_CRC			(-3014)
#define LTR34_ERROR_CANT_WRITE_FLASH		(-3015)
#define LTR34_ERROR_CANT_READ_FLASH			(-3016)
#define LTR34_ERROR_CANT_WRITE_SERIAL_NUM	(-3017)
#define LTR34_ERROR_CANT_READ_SERIAL_NUM	(-3018) 
#define LTR34_ERROR_CANT_WRITE_FPGA_VER		(-3019)
#define LTR34_ERROR_CANT_READ_FPGA_VER		(-3020)
#define LTR34_ERROR_CANT_WRITE_CALIBR_VER	(-3021)
#define LTR34_ERROR_CANT_READ_CALIBR_VER	(-3022)
#define LTR34_ERROR_CANT_STOP				(-3023)
#define LTR34_ERROR_SEND_CMD				(-3024)
#define LTR34_ERROR_CANT_WRITE_MODULE_NAME	(-3025)
#define LTR34_ERROR_CANT_WRITE_MAX_CH_QNT	(-3026)
#define LTR34_ERROR_CHANNEL_NOT_OPENED		(-3027)
#define LTR34_ERROR_WRONG_LCH_CONF			(-3028)


	//
#define LTR34_MAX_BUFFER_SIZE		2097151
#define LTR34_EEPROM_SIZE			2048
#define LTR34_USER_EEPROM_SIZE		1024
#define LTR34_DAC_NUMBER_MAX		8

	typedef unsigned char byte;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef enum
	{
		ltr_34_gen_type_sin=0,
		ltr_34_gen_type_pila=1,
		ltr_34_gen_type_mean=2,
		ltr_34_gen_type_max=3
	}_ltr34_gen_type;

	typedef struct
	{
		double angle;
		double Period;
		double Freq;
		double Min;
		double Max;
		_ltr34_gen_type GenType;
	}_ltr34_gen_param_struct;



	typedef struct 
	{
		float FactoryCalibrOffset[2*LTR34_DAC_NUMBER_MAX];
		float FactoryCalibrScale[2*LTR34_DAC_NUMBER_MAX];

	} DAC_CHANNEL_CALIBRATION;


	typedef struct 
	{
		CHAR Name[16];
		CHAR Serial[24];
		CHAR FPGA_Version[8];
		CHAR CalibrVersion[8];  
		BYTE MaxChannelQnt;

	} TINFO_LTR34,*PTINFO_LTR34;

	//**** конфигурация модуля
	typedef struct 
	{
		int size;    // Размер структуры 

		TLTR Channel;					  // структура описывающая модуль в крейте – описание в ltrapi.pdf 
		DWORD LChTbl[8];                  // Таблица логических каналов
		//**** настройки модуля           
		byte FrequencyDivisor;            // делитель частоты дискретизации 0..60 (31.25..500 кГц)
		byte ChannelQnt;             // число каналов 0, 1, 2, 3 соотвествнно (1, 2, 4, 8)
		bool UseClb;
		bool AcknowledgeType;             // тип подтверждения true - высылать подтверждение каждого слова, false- высылать состояние буфера каждые 100 мс
		bool ExternalStart;               // внешний старт true - внешний старт, false - внутренний
		bool RingMode;                    // режим кольца  true - режим кольца, false - потоковый режим
		bool BufferFull;					// статус - буфер переполнен - ошибка
		bool BufferEmpty;					// статус - буфер пуст - ошибка
		bool DACRunning;					// статус - запущена ли генерация
		float FrequencyDAC;				// статус - частота - на которую настроен ЦАП в текущей конфигурации
		DAC_CHANNEL_CALIBRATION	DacCalibration;
		TINFO_LTR34 ModuleInfo;
	}TLTR34,*PTLTR34;                      


	// Инициализация полей структуры TLTR34
    INT LTR34_Init (TLTR34 *module);

	// Установление связи с модулем LTR34.
	// Модулю посылается пакет STOP+RESET и ожидается подтверждение сброса.
    INT LTR34_Open (TLTR34 *module, DWORD saddr, WORD sport, CHAR *csn, WORD cc);

	// Разрыв связи с модулем.
    INT LTR34_Close (TLTR34 *module);

	// Определение сотояния канала связи с модулем.
    INT LTR34_IsOpened (TLTR34 *module);

	/* 
	Функция для приема данных от модуля.
	1) при работе цап в режиме ECHO - все данные, генерируемые
	модулем, сервер обрабатвает и отдает клиенту в том виде
	в котором принял. Таким образом, в режиме ECHO клиент
	должен всегда откачивать данные от модуля.
	2) При работе цап в режиме PERIOD - все данные, генерируемые
	модулем, сервер обрабатвает и "проглатывает".
	*/
    INT LTR34_Recv (TLTR34 *module, DWORD *data, DWORD *tstamp, DWORD size, DWORD timeout);

	// Формирование логического канала
    DWORD LTR34_CreateLChannel(BYTE PhysChannel, bool ScaleFlag);

	/*
	функция для отправки данных модулю
	1) В ltr-сервере реализовано управление потоком данных
	поступающих на модуль LTR34 от клиента. Поэтому при
	работе с ЦАП в потоковом режиме (RingMode=0)
	данные можно высылать в любых количествах, не заботясь
	о переполнении буфера в LTR34. Однако, следует
	позаботиться о правильном подборе таймаута, т.к.
	при достаточном заполнении буфера в модуле
	прием данных сервером от клиента будет производиться со скоростью
	пропорцианальной частоте дискретизации ЦАП, т.е. при низких частотах
	дискретизации и достаточно большом количестве отсылаемых данных
	может потребоваться достаточно большое значение таймаута.

	2) Управление потоком модуля LTR34 строится на подсчете количества
	отправленных сэмплов и принятых подтверждений, что в некоторых
	случаях может приводить к блокировке потока в модуль (Например,
	на LTR34 уже было отправлено достаточное количество данных, но
	ЦАП еще не стартовал и подтверждения от модуля еще не поступают.
	В этом случае сервер прикратит прием данных от клиента).
	Для сброса механизма управления потоком следует выслать модулю
	команду RESET (при этом будут сброшены внутренние счетчики в ltr-сервере)
	или произвести переподключение к модулю (вызвать LTR34_Close() и LTR34_Open())
	*/
    INT LTR34_Send (TLTR34 *module, DWORD *data, DWORD size, DWORD timeout);

    INT LTR34_ProcessData(TLTR34 *module, double *src, DWORD *dest, DWORD size, bool volt);

	// запись данных в FIFO буфер 
    int LTR34_SendData(TLTR34 *module, double *data, DWORD size, DWORD timeout, bool calibrMainPset, bool calibrExtraVolts);
	// Запись регистра CONFIG
    INT LTR34_Config  (TLTR34 *module);

	// Запуск ЦАП.
    INT LTR34_DACStart   (TLTR34 *module);

	// Останов ЦАП.
    INT LTR34_DACStop    (TLTR34 *module);
    INT LTR34_Reset(TLTR34 *module);

    INT LTR34_SetDescription(TLTR34 *module);
    INT LTR34_GetDescription(TLTR34 *module);

    INT LTR34_GetCalibrCoeffs(TLTR34 *module);
    INT LTR34_WriteCalibrCoeffs(TLTR34 *module);

    INT LTR34_ReadFlash(TLTR34 *module, BYTE *data, WORD size, WORD Address);


	// функции вспомагательного характера
    LPCSTR LTR34_GetErrorString(int error);

	// Проверка целостности ППЗУ	
    INT LTR34_TestEEPROM(TLTR34 *module);

	// недокументированная функция - пользовать на свой страх и риск, но работает :)
    INT LTR34_PrepareGenData(TLTR34 *module, double* GenerateData, int GenLength, _ltr34_gen_param_struct * GenParam);


#endif

