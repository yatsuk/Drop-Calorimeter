
#include <algorithm>
#include <stdio.h>
#include "ltr27api.h"
#include "crc.h"

using namespace std;

//-----------------------------------------------------------------------------
#define LTR27CMD_ECHO                         (LTR010CMD_INSTR|0x00)
#define LTR27CMD_ADC_STOP                     (LTR010CMD_INSTR|0x02)
#define LTR27CMD_ADC_START                    (LTR010CMD_INSTR|0x03)
#define LTR27CMD_EEPROM_WRITE_ENABLE_DISABLE  (LTR010CMD_INSTR|0x07)
#define LTR27CMD_READ_BYTE_AVR                (LTR010CMD_INSTR|0x08)
#define LTR27CMD_READ_BYTE_AVR0               (LTR010CMD_INSTR|0x08)
#define LTR27CMD_READ_BYTE_AVR1               (LTR010CMD_INSTR|0x09)
#define LTR27CMD_READ_BYTE_AVR2               (LTR010CMD_INSTR|0x0A)
#define LTR27CMD_READ_BYTE_AVR3               (LTR010CMD_INSTR|0x0B)
#define LTR27CMD_WRITE_BYTE_AVR               (LTR010CMD_INSTR|0x0C)
#define LTR27CMD_READ_BYTE_SUB                (LTR010CMD_INSTR|0x10)
#define LTR27CMD_WRITE_BYTE_SUB               (LTR010CMD_INSTR|0x18)
#define AVR_VAR_BASE  (0x60)
// максимальный размер пакета команд обрабатываемых модулем
#define PACKET_SIZE   (118)
// тайм-аут приема ответа на посланую команду
#define TIMEOUT_CMD_SEND								2000
#define TIMEOUT_CMD_RECIEVE								5000
//
#define MODULE_EEPROM_SIZE         (sizeof(TLTR27_DeviceDescription))
#define MEZZANINE_EEPROM_SIZE      (sizeof(TLTR27_MezzanineDescription))

// описание модуля в упакованном формате (128 байт)
typedef struct {
	BYTE  CompanyName[16];
	BYTE  DeviceName[16];
	BYTE  SerialNumber[16];
	BYTE  CpuName[16];
	DWORD CpuClockRate;
	DWORD FirmwareVersion;
	BYTE  DeviceRevision;
	BYTE  Comment[53];
	WORD Crc;
} TLTR27_DeviceDescription;
// описание мезонина в упакованном формате (128 байт)
typedef struct {
	WORD  Version;
	WORD  ID;
	WORD  Crc;
	BYTE   SerialNumber[6];
	WORD  CalibrCoef[4];
	BYTE   Comment[108];
} TLTR27_MezzanineDescription;
#pragma pack()
// число элементов в массиве MezzanineInfo
#define MINFO_SIZE    (13)
// идентификаторы мезонинов
#define H27__U01      9
#define H27__U10      10
#define H27__U20      11
#define H27__U30      12
#define H27__U300     13
#define H27__I5       20
#define H27__I10      21
#define H27__I20      22
#define H27__R100     31
#define H27__R250     32
#define H27__T        40
#define H27__EMPTY   255
// описание мезанинов
typedef struct                                 //
{                                              //
	WORD ID;                                    // идентификатор субмодуля
	CHAR   Name[16];                              // название модуля
	CHAR   Unit[16];                              // измеряемая субмодулем физ.величина
	double ConvCoeff[2];                          // масштаб и смещение для пересчета кода в физ.величину
} TMezzanineInfo;                              //
// перечислены все известные h27-мезонины и их параметры
const TMezzanineInfo MezzanineInfo[MINFO_SIZE]=
{
    {H27__U01  , "U01\0" , "В\0" , {   2.0/0x8000,  -1.0}},
    {H27__U10  , "U10\0" , "В\0" , {  20.0/0x8000, -10.0}},
    {H27__U20  , "U20\0" , "В\0" , {  20.0/0x8000,   0.0}},
    {H27__U30  , "U30\0" , "В\0" , {  30.0/0x8000,   0.0}},
    {H27__U300 , "U300\0", "В\0" , { 300.0/0x8000,   0.0}},
    {H27__I5   , "I5\0"  , "мА\0", {   5.0/0x8000,   0.0}},
    {H27__I10  , "I10\0" , "мА\0", {  20.0/0x8000, -10.0}},
    {H27__I20  , "I20\0" , "мА\0", {  20.0/0x8000,   0.0}},
    {H27__R100 , "R100\0", "Ом\0", { 100.0/0x8000,   0.0}},
    {H27__R250 , "R250\0", "Ом\0", { 250.0/0x8000,   0.0}},
    {H27__T    , "T\0"   , "мВ\0", { 100.0/0x8000, -25.0}},
    {H27__EMPTY, "EMPTY\0" , "\0", { 100.0/0x8000,   0.0}},
    {H27__EMPTY, "UDEF\0" , "\0" , { 100.0/0x8000,   0.0}},
};
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
DWORD evalParity(DWORD data)
{
	DWORD parity=0, temp=data&0xFFFF00DF;
	for(int i=0; i<32; i++) {
		parity^=temp&1;
		temp>>=1;
	}
	return (data&0xFFFFFFDF)|(parity<<5);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int exchange(TLTR27 *module, DWORD *src_data, DWORD *des_data, DWORD size)
{
	int res=LTR_OK;
	if(module!=NULL) 
	{
		for(unsigned int i=0; i<size; i++) 
		{
            src_data[i]=evalParity(src_data[i]|((module->Channel.cc-CC_MODULE1)<<8));
		}		

		DWORD sz=0;
		const int RBUF_SIZE=32768;
		DWORD * rbuf = new DWORD[RBUF_SIZE];
		if (rbuf==NULL) res=LTR_ERROR_MEMORY_ALLOC;
		DWORD tbegin=GetTickCount();

		while(res==LTR_OK && sz<size) 
        {
            int wsz=(size-sz<PACKET_SIZE)?size-sz:PACKET_SIZE;

			if(LTR_Send(&module->Channel, src_data+sz, wsz, TIMEOUT_CMD_SEND)==wsz) 
			{
				int rsz=0;					
				while(res==LTR_OK && rsz<wsz) 
				{				
                    DWORD MaxValueRecieve = min(wsz-rsz, RBUF_SIZE);
					int tsz=LTR_Recv(&module->Channel, rbuf, NULL, MaxValueRecieve, TIMEOUT_CMD_RECIEVE+5000);
					for(int i=0; res==LTR_OK&&rsz<wsz && i<tsz; i++) 
					{
						DWORD data=rbuf[i];
						if((data&0xC000)==0x8000) 
						{
                            if(data==evalParity(data))
							{
								des_data[sz+rsz]=data;
								rsz++;
							}else res=LTR27_ERROR_EXCHANGE_RECIEVE;																	
						};
					}

					if(((GetTickCount()-tbegin)>(TIMEOUT_CMD_RECIEVE*2+TIMEOUT_CMD_SEND)))
					{ // проверка на время
						res=LTR27_ERROR_EXCHANGE_TIME;
					}
				}
				sz+=rsz;
			}else res=LTR27_ERROR_EXCHANGE_SEND;							
		};

		if(rbuf!=NULL) delete rbuf;
		if(res==LTR_OK&&sz!=size) res=LTR27_ERROR_EXCHANGE_SIZE;
	} else res=LTR27_ERROR_EXCHANGE_PARAM;
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_Init(TLTR27 *module)
{
	INT res;
	if(module!=NULL) 
	{
		memset(module,0, sizeof(TLTR27));
		res=LTR_Init(&module->Channel);
		if(res==LTR_OK) 
		{
			module->size=sizeof(TLTR27);
			module->subchannel=0;
			module->FrequencyDivisor=0;
			for(int i=0; i<LTR27_MEZZANINE_NUMBER; i++) 
			{
				strcpy(module->Mezzanine[i].Name, MezzanineInfo[MINFO_SIZE-1].Name);
				strcpy(module->Mezzanine[i].Unit, MezzanineInfo[MINFO_SIZE-1].Unit);
				module->Mezzanine[i].ConvCoeff[0]=MezzanineInfo[MINFO_SIZE-1].ConvCoeff[0];
				module->Mezzanine[i].ConvCoeff[0]=MezzanineInfo[MINFO_SIZE-1].ConvCoeff[1];
				module->Mezzanine[i].CalibrCoeff[0]=1.0;
				module->Mezzanine[i].CalibrCoeff[1]=0.0;
				module->Mezzanine[i].CalibrCoeff[2]=1.0;
				module->Mezzanine[i].CalibrCoeff[3]=0.0;
			}
		}
	} else res=LTR_ERROR_PARAMETRS;
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_Open(TLTR27 *module, DWORD saddr, WORD sport, CHAR *csn, WORD cc)
{
	int res=LTR_OK;
	int WarningRes = LTR_OK;
	if(module!=NULL && csn!=NULL) 
	{
		if (LTR27_IsOpened(module)==LTR_OK)
		{
			res=LTR27_Close(module);
		}	
		
		if(res==LTR_OK) 
		{
			module->Channel.saddr=saddr;
			module->Channel.sport=sport;
			module->Channel.cc=cc;
			memcpy(module->Channel.csn, csn, SERIAL_NUMBER_SIZE);
			WarningRes=LTR_Open(&module->Channel);
			if(WarningRes==LTR_OK||WarningRes==LTR_WARNING_MODULE_IN_USE) res=LTR_OK;
			else res = WarningRes;

			if(res==LTR_OK) 
			{
				const int SBUF_SIZE=2;
				DWORD	rbuf[1], 
						sbuf[SBUF_SIZE]={LTR010CMD_STOP,LTR010CMD_RESET};				
				if(LTR_Send(&module->Channel, sbuf, SBUF_SIZE, TIMEOUT_CMD_SEND)!=SBUF_SIZE) res=LTR27_ERROR_SEND_DATA;
				else 
				{
					while(res==LTR_OK) 
					{
						if(LTR_Recv(&module->Channel, rbuf, NULL, 1, TIMEOUT_CMD_RECIEVE)<=0) res=LTR27_ERROR_RECV_DATA;
						else if((rbuf[0]&0xC0C0)==LTR010CMD_RESET) 
						{
							if(LTR_Send(&module->Channel, sbuf, 1, TIMEOUT_CMD_SEND)!=1) res=LTR27_ERROR_SEND_DATA; // засылаем стоп
							else
							{
								if((rbuf[0]>>16)!=0x1B1B) res=LTR27_ERROR_NOT_LTR27;
								else
								{
									Sleep(20); // необходимо немножко подождать, иначе - модуль еще не готов к работе
									break;
								}
							}
						}
					}
				}
				//
				if(res!=LTR_OK) LTR_Close(&module->Channel);
			}
		}
	} else res=LTR_ERROR_PARAMETRS;

	if(WarningRes==LTR_WARNING_MODULE_IN_USE&&res==LTR_OK)res = WarningRes;
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_Close(TLTR27 *module)
{
	INT res;
	if(module!=NULL)  res=LTR_Close(&module->Channel);
	else res=LTR_ERROR_PARAMETRS;

	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_IsOpened(TLTR27 *module)
{
	INT res;
	if(module!=NULL) res=LTR_IsOpened(&module->Channel);
	else res=LTR_ERROR_PARAMETRS;

	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#define ECHO_SIZE (PACKET_SIZE)
INT LTR27_Echo(TLTR27 *module)
{
	INT res=LTR27_IsOpened(module);
	if(res==LTR_OK) 
	{
		DWORD src_data[ECHO_SIZE], des_data[ECHO_SIZE];
		for(int i=0; i<ECHO_SIZE; i++) src_data[i]=LTR27CMD_ECHO|(rand()<<16);
		res=exchange(module, src_data, des_data, ECHO_SIZE);
		if(res==LTR_OK) 
		{
			for(int i=0; res==LTR_OK && i<ECHO_SIZE; i++)
			{
				if(src_data[i]!=des_data[i])
				{
					res=LTR27_ERROR;
				}
			}
		}
	}
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_GetConfig(TLTR27 *module)
{
	INT res=LTR27_IsOpened(module);
	if(res==LTR_OK) 
	{
		DWORD i, src_data[1+LTR27_MEZZANINE_NUMBER], des_data[1+LTR27_MEZZANINE_NUMBER];
		src_data[0]=((AVR_VAR_BASE+0)<<24)|(LTR27CMD_READ_BYTE_AVR+0);
		for(i=0; i<LTR27_MEZZANINE_NUMBER; i++)
			src_data[i+1]=(2<<24)|(LTR27CMD_READ_BYTE_SUB+i);
		res=exchange( module, src_data, des_data, 1+LTR27_MEZZANINE_NUMBER);
		if(res==LTR_OK)
		{
			for(i=0; i<(1+LTR27_MEZZANINE_NUMBER); i++)
			{
				if((src_data[i]&0xFF0000DF)!=(des_data[i]&0xFF0000DF)) break;
			}
			if(i==(1+LTR27_MEZZANINE_NUMBER)) 
			{
				module->FrequencyDivisor=(BYTE)((des_data[0]>>16)&0xFF);
				for(i=0; i<LTR27_MEZZANINE_NUMBER; i++) 
				{
					BYTE j, id=(BYTE)((des_data[i+1]>>16)&0xFF);
					for(j=0; j<(MINFO_SIZE-1); j++)
					{
						if(MezzanineInfo[j].ID==id) break;
					}
					strcpy(module->Mezzanine[i].Name, MezzanineInfo[j].Name);
					strcpy(module->Mezzanine[i].Unit, MezzanineInfo[j].Unit);
					module->Mezzanine[i].ConvCoeff[0]=MezzanineInfo[j].ConvCoeff[0];
					module->Mezzanine[i].ConvCoeff[1]=MezzanineInfo[j].ConvCoeff[1];
				}
				res=LTR_OK;
			} else res=LTR27_ERROR_PARITY;
		} 
	}
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_SetConfig(TLTR27 *module)
{ 
	DWORD src_data, des_data;	
	src_data=((AVR_VAR_BASE+0)<<24)|(module->FrequencyDivisor<<16)|(LTR27CMD_WRITE_BYTE_AVR+0);
	INT res=exchange(module, &src_data, &des_data, 1);
	if(res==LTR_OK&&src_data!=des_data) res=LTR27_ERROR;
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_ADCStart(TLTR27 *module)
{
	INT res=LTR_OK;
	DWORD src_data, des_data;
	src_data=LTR27CMD_ADC_STOP;
	res=exchange(module, &src_data, &des_data, 1);
	if(res==LTR_OK&&src_data!=des_data) res=LTR27_ERROR;
	if(res==LTR_OK) 
	{
		src_data=LTR27CMD_ADC_START;
		res=exchange(module, &src_data, &des_data, 1);
		if(res==LTR_OK && src_data!=des_data) res=LTR27_ERROR;
		if(res==LTR_OK)
		{
			module->subchannel=0;		
		} 
	} 
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_ADCStop(TLTR27 *module)
{
	DWORD src_data, des_data;
	src_data=LTR27CMD_ADC_STOP;
	INT res=exchange(module, &src_data, &des_data, 1);
	if(res==LTR_OK&&src_data!=des_data) res=LTR27_ERROR;
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_Recv(TLTR27 *module, DWORD *data, DWORD *tstamp, DWORD size, DWORD timeout)
{
	INT res=LTR_Recv(&module->Channel, data, tstamp, size, timeout);
	if(res>0) {
		if(!(module->Channel.flags&FLAG_RBUF_OVF)) 
		{
			for(int i=0; i<res; i++) 
			{
				DWORD dw=data[i];
                if(dw==evalParity(dw))
				{
					if((dw&0xF)==module->subchannel) 
					{
						module->subchannel=(module->subchannel+1)&0xF;
					} else res=LTR27_ERROR_INDEX;

				} else res=LTR27_ERROR_PARITY;
			}
		} else res=LTR27_ERROR_OVERFLOW;
	}
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_ProcessData(TLTR27 *module, DWORD *src_data, double *dst_data, DWORD *size, BOOL calibr, BOOL value)
{
	INT res=LTR_OK;
	if(module!=NULL && src_data!=NULL && dst_data!=NULL) 
	{
		// подготовка коэффициентов
		double scale1[16], offset1[16], scale2[16], offset2[16];
		for(int chn=0; chn<16; chn++) 
		{
			// калибровочные коэффициенты
			if(calibr) 
			{
				scale1[chn] =module->Mezzanine[chn>>1].CalibrCoeff[((chn&1)<<1)+0];
				offset1[chn]=module->Mezzanine[chn>>1].CalibrCoeff[((chn&1)<<1)+1];
			} 
			else 
			{
				scale1[chn] =1.0;
				offset1[chn]=0.0;
			}
			// пересчет масштаба для выбраной частоты дискретизации
			scale1[chn]=scale1[chn]*0x7FFF/250/(module->FrequencyDivisor+1);
			// коэффициенты для пересчета в физические величиныж
			if(value) 
			{
				scale2[chn] =module->Mezzanine[chn>>1].ConvCoeff[0];
				offset2[chn]=module->Mezzanine[chn>>1].ConvCoeff[1];
			} 
			else 
			{
				scale2[chn] =1.0;
				offset2[chn]=0.0;
			}
		}
		//
		DWORD j=0;	
		for(DWORD i=0,Channel_Number=src_data[0]&0xF; i<*size; i++) 
		{
			DWORD dw =src_data[i];
			DWORD chn=dw&0xF;
			double data;

			if (Channel_Number!=chn)
			{
				res=LTR27_ERROR_INDEX;
			}

			// корректировка данных
			dw=(DWORD)((dw>>16)*scale1[chn]+offset1[chn]);
			if(dw<0xC000) data=*(WORD *)&dw;
			else data=*(SHORT*)&dw;
			// перевод в физ.величины
			dst_data[j++]=scale2[chn]*data+offset2[chn];

			if (++Channel_Number>15)Channel_Number=0;
		}
		*size=j;
	} 
	else res=LTR_ERROR_PARAMETRS;
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
INT LTR27_GetDescription(TLTR27 *module, WORD flags)
{
	INT res=LTR_OK;
	const DWORD DATA_SIZE_MAX=MODULE_EEPROM_SIZE+LTR27_MEZZANINE_NUMBER*MEZZANINE_EEPROM_SIZE;
	DWORD src_data[DATA_SIZE_MAX], des_data[DATA_SIZE_MAX];
	DWORD DATA_SIZE=0;
	// формируем блок команд
	if(flags&LTR27_MODULE_DESCRIPTION) 
	{
        for(unsigned int i=0; i<MODULE_EEPROM_SIZE; i++)
		{
			src_data[i]=((0x80+i)<<24)|(LTR27CMD_READ_BYTE_AVR3);
		}
		DATA_SIZE+=MODULE_EEPROM_SIZE;
		memset(&module->ModuleInfo.Module, 0, sizeof(TDESCRIPTION_MODULE));
		memset(&module->ModuleInfo.Cpu   , 0, sizeof(TDESCRIPTION_CPU));
	}
	for(int i=0; i<LTR27_MEZZANINE_NUMBER; i++) 
	{
		if(flags&(1<<(i+1))) 
		{
            for(unsigned int j=0; j<MEZZANINE_EEPROM_SIZE; j++)
			{
				src_data[DATA_SIZE+j]=(j<<24)|(LTR27CMD_READ_BYTE_SUB+i);
			}
			DATA_SIZE+=MEZZANINE_EEPROM_SIZE; 
			memset(&module->ModuleInfo.Mezzanine[i], 0, sizeof(TDESCRIPTION_MEZZANINE));
		}
	}
	// отсылаем блок команд и принимаем ответ
	res=exchange(module, src_data, des_data, DATA_SIZE);
	if(res==LTR_OK) 
	{
		// проверка полей адреса и команд
		for(DWORD i=0; res==LTR_OK && i<DATA_SIZE; i++)
		{
			if((src_data[i]&0xFF0000DF)!=(des_data[i]&0xFF0000DF)) res=LTR27_ERROR;
		}
		//
		DATA_SIZE=0;
		// распаковка полей структуры
		if(res==LTR_OK) 
		{
			// распаковываем описание модуля
			if(flags&LTR27_MODULE_DESCRIPTION) 
			{
				TLTR27_DeviceDescription ddescr;
                for(unsigned int i=0; i<MODULE_EEPROM_SIZE; i++)
				{
					((BYTE*)&ddescr)[i]=(BYTE)(des_data[i]>>16);
				}
				// проверяем кс
                WORD Crc=eval_crc16(0, (BYTE*)&ddescr, MODULE_EEPROM_SIZE-sizeof(WORD));
				if(ddescr.Crc==Crc) 
				{
					// module module->ModuleInfo
                    memcpy(module->ModuleInfo.Module.CompanyName,  ddescr.CompanyName,  min(sizeof(module->ModuleInfo.Module.CompanyName),  sizeof(ddescr.CompanyName)));
					memcpy(module->ModuleInfo.Module.DeviceName,   ddescr.DeviceName,   min(sizeof(module->ModuleInfo.Module.DeviceName),   sizeof(ddescr.DeviceName)));
					memcpy(module->ModuleInfo.Module.SerialNumber, ddescr.SerialNumber, min(sizeof(module->ModuleInfo.Module.SerialNumber), sizeof(ddescr.SerialNumber)));
					module->ModuleInfo.Module.Revision=ddescr.DeviceRevision;
					//memcpy(module->ModuleInfo.Module.Comment,      ddescr.Comment,      sizeof(module->ModuleInfo.Module.Comment));
					// cpu module->ModuleInfo
					module->ModuleInfo.Cpu.Active=1;
					memcpy(module->ModuleInfo.Cpu.Name,  ddescr.CpuName,  min(sizeof(module->ModuleInfo.Cpu.Name), sizeof(ddescr.CpuName)));
					module->ModuleInfo.Cpu.ClockRate      =ddescr.CpuClockRate;
					module->ModuleInfo.Cpu.FirmwareVersion=ddescr.FirmwareVersion;
					memcpy(module->ModuleInfo.Cpu.Comment, ddescr.Comment, min(sizeof(module->ModuleInfo.Cpu.Comment), sizeof(ddescr.Comment)));
				} 
				else 
				{
                    strcpy((char *)module->ModuleInfo.Module.CompanyName,"L-CARD Inc.");
                    strcpy((char *)module->ModuleInfo.Module.DeviceName, "LTR27");
                    strcpy((char *)module->ModuleInfo.Module.SerialNumber, "EMPTY");
				}
				//
				DATA_SIZE+=MODULE_EEPROM_SIZE;
			}
			// распаковываем описание мезонинов
			for(int i=0; i<LTR27_MEZZANINE_NUMBER; i++) 
			{
				if(flags&(1<<(i+1))) 
				{
					TLTR27_MezzanineDescription mdescr;
                    for(unsigned int j=0; j<MEZZANINE_EEPROM_SIZE; j++)
					{
						((BYTE*)&mdescr)[j]=(BYTE)(des_data[DATA_SIZE+j]>>16);
					}
					// проверяем кс
					WORD Crc;
					Crc=eval_crc16(  0, ((BYTE*)&mdescr)+0, 4);
					Crc=eval_crc16(Crc, ((BYTE*)&mdescr)+6, MEZZANINE_EEPROM_SIZE-6);
					if(mdescr.Crc==Crc)
					{
						module->ModuleInfo.Mezzanine[i].Active=1;
						int j;
						char DATA_TEXT[8];
						char INSERTION[6];
						int len,lenData;
						BYTE RR;
						for(j=0; j<(MINFO_SIZE-1); j++)
						{
							if(MezzanineInfo[j].ID==(mdescr.ID&0xFF)) break;
						}
						strcpy((char *)module->ModuleInfo.Mezzanine[i].Name, MezzanineInfo[j].Name);
						//strcpy(module->ModuleInfo.Mezzanine[i].Unit, MezzanineInfo[j].Unit);
						//module->ModuleInfo.Mezzanine[i].ConvCoeff[0]=MezzanineInfo[j].ConvCoeff[0];
						//module->ModuleInfo.Mezzanine[i].ConvCoeff[1]=MezzanineInfo[j].ConvCoeff[1];
						RR=mdescr.SerialNumber[5]&0xF8;
						if(RR==0xA8)
						{//читаем с учётом длинны серийника
							len=mdescr.SerialNumber[5]&0x07;
                            sprintf(DATA_TEXT,"%ld",(*(DWORD*)(mdescr.SerialNumber+2))&0xFFFFFF);
							lenData=strlen(DATA_TEXT);
							INSERTION[0]=0;
							if(lenData<len)
							for(j=0;j<(len)-lenData;j++)
							{
								INSERTION[j]=48;
								INSERTION[j+1]=0;
							}
							sprintf((char *)module->ModuleInfo.Mezzanine[i].SerialNumber,"%i%c%s%s",
								mdescr.SerialNumber[0], mdescr.SerialNumber[1],INSERTION,DATA_TEXT);
						}else
						{//читаем постарому
                            sprintf((char *)module->ModuleInfo.Mezzanine[i].SerialNumber, "%i%c%ld",
								mdescr.SerialNumber[0], mdescr.SerialNumber[1], (*(DWORD*)(mdescr.SerialNumber+2))&0xFFFFFF);
						}
						
						for(j=2;j<8;j++)
						if(module->ModuleInfo.Mezzanine[i].SerialNumber[j]==32)
							module->ModuleInfo.Mezzanine[i].SerialNumber[j]=48;
						
						module->ModuleInfo.Mezzanine[i].Revision='0'+mdescr.Version;
						for(int j=0; j<4; j+=2)
						{
							WORD scale =mdescr.CalibrCoef[j+0],
								offset=mdescr.CalibrCoef[j+1];
							module->ModuleInfo.Mezzanine[i].Calibration[j+0]=(double)scale/0x8000;
							if(offset<0xC000) module->ModuleInfo.Mezzanine[i].Calibration[j+1]=(double)offset;
							else  module->ModuleInfo.Mezzanine[i].Calibration[j+1]=(double)(*(SHORT*)&offset);
						}
						memcpy(module->ModuleInfo.Mezzanine[i].Comment, mdescr.Comment, min(sizeof(module->ModuleInfo.Mezzanine[i].Comment), sizeof(mdescr.Comment)));
					}
					//
					DATA_SIZE+=MEZZANINE_EEPROM_SIZE;
				}
			}
		} else res=LTR27_ERROR_FORMAT;
	}
	return res;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

INT LTR27_WriteMezzanineDescr(TLTR27 *module, BYTE mn)
{
	INT result=LTR_OK;
	int len;
	char MSN[16];
	TLTR27_MezzanineDescription mdescr;

	if(mn<LTR27_MEZZANINE_NUMBER)
	{
		DWORD src_data[MEZZANINE_EEPROM_SIZE+2], des_data[MEZZANINE_EEPROM_SIZE+2];

		// упаковка дескриптора
		mdescr.SerialNumber[0]=atoi((char *)&module->ModuleInfo.Mezzanine[mn].SerialNumber[0]);
		mdescr.SerialNumber[1]=module->ModuleInfo.Mezzanine[mn].SerialNumber[1];	
		*(DWORD *)&mdescr.SerialNumber[2]=atoi((char *)&module->ModuleInfo.Mezzanine[mn].SerialNumber[2]);
		memcpy(MSN,module->ModuleInfo.Mezzanine[mn].SerialNumber,16);
		len=strlen(MSN);//длинна серийного номера (-2, т.к. первые символы есть всегда)
		if(len<=8)
		len=0xA8+(len-2);		
		mdescr.SerialNumber[5]=len; // не используется в старых субмодулях, а в новых будем писать длинну серийника
		// 0xFF - значит старый модуль   0xA8 - признак что тут есть длинна , длинна в последних 3х битах

		mdescr.CalibrCoef[0] =(WORD)(module->ModuleInfo.Mezzanine[mn].Calibration[0]*0x8000);
		mdescr.CalibrCoef[1]=(SHORT)(module->ModuleInfo.Mezzanine[mn].Calibration[1]);
		mdescr.CalibrCoef[2] =(WORD)(module->ModuleInfo.Mezzanine[mn].Calibration[2]*0x8000);	
		mdescr.CalibrCoef[3]=(SHORT)(module->ModuleInfo.Mezzanine[mn].Calibration[3]);

		memcpy(&mdescr.Comment,&module->ModuleInfo.Mezzanine[mn].Comment,min(sizeof(mdescr.Comment),sizeof(module->ModuleInfo.Mezzanine[mn].Comment)));

		mdescr.Version=module->ModuleInfo.Mezzanine[mn].Revision-'0';

		for (int i=0; i <MINFO_SIZE; i++)
		{
			if (strcmp((char *)module->ModuleInfo.Mezzanine[mn].Name,MezzanineInfo[i].Name)==0)
		 {
			 mdescr.ID=MezzanineInfo[i].ID; break;
		 }
		}

		// подсчет кс
		WORD Crc;
		Crc=eval_crc16(  0, ((BYTE*)&mdescr)+0, 4);
		Crc=eval_crc16(Crc, ((BYTE*)&mdescr)+6, MEZZANINE_EEPROM_SIZE-6);
		mdescr.Crc=Crc;

		// формируем блок команд
		src_data[0]=(mn<<24)|(1<<16)|LTR27CMD_EEPROM_WRITE_ENABLE_DISABLE;
        for(unsigned int i=0; i<MEZZANINE_EEPROM_SIZE; i++)
		{		
			src_data[i+1]=(i<<24)|(((BYTE *)&mdescr)[i]<<16)|(LTR27CMD_WRITE_BYTE_SUB+mn);
		}
		src_data[MEZZANINE_EEPROM_SIZE+1]=(mn<<24)|(0<<16)|LTR27CMD_EEPROM_WRITE_ENABLE_DISABLE;
		// отсылаем блок команд и принимаем ответ
		// не более 110 комманд - за раз, поскольку будет переполнение буффера комманд в модуле
		// а MEZZANINE_EEPROM_SIZE == 128
		result=exchange(module, src_data, des_data, MEZZANINE_EEPROM_SIZE+2-30);
		if(result==LTR_OK) 			
		{
			result=exchange(module, &src_data[MEZZANINE_EEPROM_SIZE+2-30], &des_data[MEZZANINE_EEPROM_SIZE+2-30], 30);
			if(result==LTR_OK)
			{
				// проверка полей адреса и команд
                unsigned int i=0;
				for(i=0; i<MEZZANINE_EEPROM_SIZE+2; i++)
				{
					if((src_data[i]&0xFF0000DF)!=(des_data[i]&0xFF0000DF)) 
					{
						break;
					}
				}

				if(i==(MEZZANINE_EEPROM_SIZE+2))
				{
				}
				else 
				{
					result=LTR27_ERROR_FORMAT;
				}
			}
		}		
	}
	else result=LTR_ERROR_PARAMETRS;
	return result;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
LPCSTR LTR27_GetErrorString(INT error)
{
	return LTR_GetErrorString(error);
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
