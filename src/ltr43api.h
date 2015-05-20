#ifndef __LTR43API__
#define __LTR43API__


#include "ltrapitypes.h"
#include "ltrapi.h"

// Коды ошибок
#define LTR43_NO_ERR                                  (0)
#define LTR43_ERR_WRONG_MODULE_DESCR				  (-4001)
#define LTR43_ERR_CANT_OPEN                           (-4002)
#define LTR43_ERR_INVALID_CRATE_SN 			          (-4003)
#define LTR43_ERR_INVALID_SLOT_NUM					  (-4004)
#define LTR43_ERR_CANT_SEND_COMMAND 				  (-4005)
#define LTR43_ERR_CANT_RESET_MODULE				      (-4006)
#define LTR43_ERR_MODULE_NO_RESPONCE				  (-4007)
#define LTR43_ERR_CANT_SEND_DATA					  (-4008)
#define LTR43_ERR_CANT_CONFIG                         (-4009) 
#define LTR43_ERR_CANT_RS485_CONFIG                   (-4010)
#define LTR43_ERR_CANT_LAUNCH_SEC_MARK				  (-4011)
#define LTR43_ERR_CANT_STOP_SEC_MARK				  (-4012)
#define LTR43_ERR_CANT_LAUNCH_START_MARK			  (-4013)
#define LTR43_ERR_CANT_STOP_RS485RCV				  (-4014)
#define LTR43_ERR_RS485_CANT_SEND_BYTE			      (-4015)  
#define LTR43_ERR_RS485_FRAME_ERR_RCV			      (-4016) 
#define LTR43_ERR_RS485_PARITY_ERR_RCV			      (-4017)
#define LTR43_ERR_WRONG_IO_GROUPS_CONF			      (-4018)   
#define LTR43_ERR_RS485_WRONG_BAUDRATE			      (-4019)
#define LTR43_ERR_RS485_WRONG_FRAME_SIZE			  (-4020)
#define LTR43_ERR_RS485_WRONG_PARITY_CONF			  (-4021)
#define LTR43_ERR_RS485_WRONG_STOPBIT_CONF			  (-4022)
#define LTR43_ERR_DATA_TRANSMISSON_ERROR			  (-4023)
#define LTR43_ERR_RS485_CONFIRM_TIMEOUT       		  (-4024) 
#define LTR43_ERR_RS485_SEND_TIMEOUT       			  (-4025)
#define LTR43_ERR_LESS_WORDS_RECEIVED            	  (-4026)
#define LTR43_ERR_PARITY_TO_MODULE              	  (-4027) 
#define LTR43_ERR_PARITY_FROM_MODULE              	  (-4028)
#define LTR43_ERR_WRONG_IO_LINES_CONF              	  (-4029)   
#define LTR43_ERR_WRONG_SECOND_MARK_CONF			  (-4030)
#define LTR43_ERR_WRONG_START_MARK_CONF				  (-4031)
#define LTR43_ERR_CANT_READ_DATA 					  (-4032)
#define LTR43_ERR_RS485_CANT_SEND_PACK				  (-4033)
#define LTR43_ERR_RS485_CANT_CONFIGURE                (-4034)
#define LTR43_ERR_CANT_WRITE_EEPROM					  (-4035)
#define LTR43_ERR_CANT_READ_EEPROM					  (-4036) 
#define LTR43_ERR_WRONG_EEPROM_ADDR 				  (-4037)
#define LTR43_ERR_RS485_WRONG_PACK_SIZE 			  (-4038)  
#define LTR43_ERR_RS485_WRONG_OUT_TIMEOUT 			  (-4039) 
#define LTR43_ERR_RS485_WRONG_IN_TIMEOUT 			  (-4040)
#define LTR43_ERR_CANT_READ_CONF_REC				  (-4041)
#define LTR43_ERR_WRONG_CONF_REC                      (-4042)
#define LTR43_ERR_RS485_CANT_STOP_TST_RCV             (-4043) 
#define LTR43_ERR_CANT_START_STREAM_READ              (-4044)   
#define LTR43_ERR_CANT_STOP_STREAM_READ               (-4045) 
#define LTR43_ERR_WRONG_IO_DATA						  (-4046)
#define LTR43_ERR_WRONG_STREAM_READ_FREQ_SETTINGS	  (-4047)
#define LTR43_ERR_ERROR_OVERFLOW					  (-4048)



/* Структура описания модуля */


typedef struct 
{
	CHAR Name[16];
	CHAR Serial[24];
	CHAR FirmwareVersion[8];// Версия БИОСа
	CHAR FirmwareDate[16];  // Дата создания данной версии БИОСа


} TINFO_LTR43,*PTINFO_LTR43; 

typedef struct
{

	INT size;   // размер структуры    
	TLTR Channel;

	double StreamReadRate;

	struct
	{

		INT Port1;	   // направление линий ввода/вывода группы 1
		INT Port2;	   // направление линий ввода/вывода группы 2 
		INT Port3;    // направление линий ввода/вывода группы 3 
		INT Port4;	   // направление линий ввода/вывода группы 4 

	} IO_Ports;

	struct
	{

		INT FrameSize;	  // Кол-во бит в кадре
		INT Baud;		  // Скорость обмена в бодах
		INT StopBit;	  // Кол-во стоп-бит
		INT Parity;		  // Включение бита четности
		INT SendTimeoutMultiplier; // Множитель таймаута отправки
		INT ReceiveTimeoutMultiplier; // Множитель таймаута приема подтверждения

	} RS485; // Структура для конфигурации RS485

	struct
	{

		INT SecondMark_Mode; // Режим меток. 0 - внутр., 1-внутр.+выход, 2-внешн
		INT StartMark_Mode; // 

	} Marks;  // Структура для работы с временными метками

	TINFO_LTR43 ModuleInfo;  

} TLTR43, *PTLTR43; // Структура описания модуля


    INT LTR43_Init(TLTR43 *hnd);
    INT LTR43_IsOpened(PTLTR43 hnd);
    INT LTR43_Open(TLTR43 *hnd, DWORD net_addr, WORD net_port, CHAR *crate_sn, INT slot_num);
    INT LTR43_Close(TLTR43 *hnd);
    INT LTR43_WritePort(TLTR43 *hnd, DWORD OutputData);
    INT LTR43_WriteArray(TLTR43 *hnd, DWORD *OutputArray, BYTE ArraySize);
    INT LTR43_ReadPort(TLTR43 *hnd, DWORD *InputData);
    INT LTR43_StartStreamRead(TLTR43 *hnd);
    INT LTR43_StopStreamRead(TLTR43 *hnd);
    INT LTR43_Recv(TLTR43 *hnd, DWORD *data, DWORD *tmark, DWORD size, DWORD timeout);
    INT LTR43_ProcessData(TLTR43 *hnd, DWORD *src, DWORD *dest, DWORD *size);
    INT LTR43_Config(TLTR43 *hnd);
    INT LTR43_StartSecondMark(TLTR43 *hnd);
    INT LTR43_StopSecondMark(TLTR43 *hnd);
    LPCSTR LTR43_GetErrorString(INT Error_Code);
    INT LTR43_MakeStartMark(TLTR43 *hnd);
    INT LTR43_RS485_Exchange(TLTR43 *hnd, SHORT *PackToSend, SHORT *ReceivedPack, INT OutPackSize, INT InPackSize);
    INT LTR43_WriteEEPROM(TLTR43 *hnd, INT Address, BYTE val);
    INT LTR43_ReadEEPROM(TLTR43 *hnd, INT Address, BYTE *val);
    INT  LTR43_RS485_TestReceiveByte(TLTR43 *hnd, INT OutBytesQnt,INT InBytesQnt);
    INT  LTR43_RS485_TestStopReceive(TLTR43 *hnd);

#endif
















