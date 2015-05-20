#include "ltr34api.h"
#include "ltrapi.h"
#include "stdio.h"
#include <algorithm>

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#define LTR34CMD_PERMITWRITEEEPROM							(LTR010CMD_INSTR|0x66)
#define LTR34CMD_BLOCKWRITEEEPROM							(LTR010CMD_INSTR|0x60)
#define LTR34CMD_WRITEEEPROM								(LTR010CMD_INSTR|0x68)
#define LTR34CMD_READEEPROM									(LTR010CMD_INSTR|0x70)
#define LTR34CMD_CLEAREEPROM								(LTR010CMD_INSTR|0x78)
#define LTR34CMD_IO_ROM                                     (LTR010CMD|0x60)

#define TIMEOUT_CMD_SEND								    (2000)
#define TIMEOUT_CMD_RECIEVE								    (4000)

#define SERIAL_ADDR                                         (0)
#define MODULE_NAME_ADDR                                    (24) 
#define FPGA_VER_ADDR                                       (36)
#define CALIBR_VER_ADDR                                     (48) 
#define CH_QNT_ADDR                                         (56)
#define CALIBR_ADDR                                         (100) 
#define CRC_ADDR											(2024) 

#define WORKING_FLASH_SIZE                                  (2000)



const LPCSTR err_str[] =
{
    "LTR34: Невозможно отправить данные в модуль!",
    "LTR34: Модуль не отвечает",
    "LTR34: Невозможно выполнить аппаратный сброс модуля!",
    "LTR34: Модуль не является LTR34!",
    "LTR34: Переполнение буфера в крейте!",
    "LTR34: Ошибка четности!",
    "LTR34: Переполнение буфера в модуле!",
    "LTR34: Неправильный индекс!",
    "LTR34: Ошибка модуля LTR34",
    "LTR34: Ошибка обмена данными",
    "LTR34: Неправильный формат данных",
    "LTR34: Неправильные параметры",
    "LTR34: Неправильный ответ",
    "LTR34: Неверная контрольная сумма ППЗУ!",
    "LTR34: Невозможно выполнить запись в ППЗУ!",
    "LTR34: Невозможно выполнить чтение из ППЗУ!",
    "LTR34: Невозможно записать серийный номер или он имеет неверный формат",
    "LTR34: Невозможно прочитать серийный номер модуля из ППЗУ",
    "LTR34: Невозможно записать версию прошивки ПЛИС или она имеет неверный формат",
    "LTR34: Невозможно прочитать версию прошивки ПЛИС из ППЗУ",
    "LTR34: Невозможно записать версию калибровки или она имеет неверный формат",
    "LTR34: Невозможно прочитать версию калибровки из ППЗУ",
    "LTR34: Невозможно остановить генерацию данных!",
    "LTR34: Невозможно отправить команду в модуль!",
    "LTR34: Невозможно записать имя модуля в ППЗУ!",
    "LTR34: Невозможно записать максимальное количество каналов в ППЗУ!",
    "LTR34: Не открыт интерфейсный канал связи с сервером",
    "LTR34: Неверная конфигурация логических каналов!"

};

void  LTR34_MakeWriteFlashCmd(WORD Address, BYTE ByteToWrite, DWORD *FlashCommand);
void  LTR34_MakeEwenFlashCmd(DWORD *FlashCommand);
void  LTR34_MakeEwdsFlashCmd(DWORD *FlashCommand);
void  LTR34_MakeReadFlashCmd(WORD Address, DWORD *FlashCommand); 
void  LTR34_FlashReadRecover(DWORD *FlashCommand, BYTE *ReadBytes);

INT LTR34_WriteFlash(TLTR34 *module, BYTE *data, WORD size, WORD Address);             

INT LTR34_EvaluateFlashCRC(TLTR34 *module, WORD *Flash_CRC);
INT LTR34_WriteFlashCRC(TLTR34 *module, WORD Flash_CRC);


INT LTR34_WriteSerialNumber(TLTR34 *module, CHAR *sn, int Code);
INT LTR34_ReadSerialNumber(TLTR34 *module, CHAR *sn);
INT LTR34_WriteModuleName(TLTR34 *hnd, CHAR *ModuleName, int Code);
INT LTR34_ReadModuleName(TLTR34 *hnd, CHAR *ModuleName);
INT LTR34_WriteFPGA_Ver(TLTR34 *module, CHAR *FPGA_Ver, int Code);
INT LTR34_Read_FPGA_Ver(TLTR34 *hnd, CHAR *FPGA_Ver);

INT LTR34_WriteCalibrVer(TLTR34 *module, CHAR *CalibrVer, int Code);
INT LTR34_ReadCalibrVer(TLTR34 *hnd, CHAR *CalibrVer);

INT LTR34_WriteChQnt(TLTR34 *module, BYTE ChQnt, int Code);
INT LTR34_ReadChQnt(TLTR34 *module, BYTE *ChQnt);

static WORD UpdateCRC16(WORD *crc, BYTE b);  
static DWORD CalkTickInterval(DWORD PrevTick, DWORD ActTick);             


/***********************************************************************************

Функция инициализации интерфейсного канала связи с модулем

*************************************************************************************/ 


int  LTR34_Init(TLTR34 *module)
{
    INT res=LTR_OK;

    if(module!=NULL)
    {
        memset(module,0,sizeof(TLTR34));

        res=LTR_Init(&module->Channel);
        if(res==LTR_OK)
        {

            module->ChannelQnt=1;     // Задействован только один канал
            module->AcknowledgeType=false; //  тип подтверждения - true - высылать состояние буфера каждые 100 мс
            module->ExternalStart=false;  //  внешний старт отключен, используется внутренний старт
            module->RingMode=false;        // режим кольца отключен, используется потоковый режим
            module->BufferFull=false;	   // флаг "буфер переполнен" обнуляется
            module->BufferEmpty=false;	   // флаг "буфер пуст" обнуляется
            module->DACRunning=false;	   // Генерация не запущена
            module->FrequencyDAC=500000.0;

            module->LChTbl[0]=LTR34_CreateLChannel(1,0); // Создаем один логический канал, соотв. физическому каналу 1, выход 1:1
            module->size = sizeof(TLTR34);


        }
    } else res=LTR_ERROR_PARAMETRS;
    return res;
}

/************************************************************************************

Функция открытия интерфейсного канала связи с модулем


**************************************************************************************/ 
int  LTR34_Open(TLTR34 *module, DWORD saddr, WORD sport, CHAR *csn, WORD cc)
{

    const int SBUF_SIZE=2;
    int res=LTR_OK;
    int warning=0;
    DWORD *rbuf, sbuf[2/*SBUF_SIZE*/];
    int tick;
    int read_cntr;



    rbuf=(DWORD *) malloc(10000*sizeof(DWORD));

    if((module==NULL)||(csn==NULL))
    {
        res=LTR34_ERROR_PARAMETERS;
        goto exit;
    }


    /*	if (LTR34_IsOpened(module)==LTR_OK)
    res=LTR34_Close(module);  */

    if(res)
        goto exit;


    if (saddr!=0)module->Channel.saddr=saddr;
    if (sport!=0)module->Channel.sport=sport;
    if (csn!=NULL) memcpy(module->Channel.csn, csn, SERIAL_NUMBER_SIZE);
    module->Channel.cc=std::max(cc,WORD(1));

    res=LTR_Open(&module->Channel);

    if(res!=LTR_OK){
        if(res<0)
            if(res==LTR_WARNING_MODULE_IN_USE)
            {
                warning=res;
                res=LTR_OK;
            }
            else
                goto exit;
        else
        {
            res=LTR34_ERROR_CHANNEL_NOT_OPENED;
            goto exit;
        }
    }



    sbuf[0]=LTR010CMD_STOP;
    sbuf[1]=LTR010CMD_RESET;


    res=LTR_Send(&module->Channel, sbuf, SBUF_SIZE, TIMEOUT_CMD_SEND);

    if(res!=SBUF_SIZE )
    {
        if((res>0)&&(res<SBUF_SIZE))
            res=LTR34_ERROR_SEND_DATA;

        goto exit;
    }



    tick=GetTickCount();

    while(1)
    {
        read_cntr=LTR_Recv(&(module->Channel), rbuf, NULL, 10000, 100); // Загружаем данные, имеющиеся в FIFO

        if(read_cntr!=0)
            if((rbuf[read_cntr-1]&0xF0C0)==LTR010CMD_RESET)

                break;



        if(CalkTickInterval(tick, GetTickCount())>=5000) // В случае превышения 5 с выходим с ошибкой
        {
            res=LTR34_ERROR_RESET_MODULE;
            goto exit;
        }
    }




    res=LTR_Send(&module->Channel, sbuf, 1, TIMEOUT_CMD_SEND);
    if(res!=1 )
    {
        if(res>=0)
            res=LTR34_ERROR_SEND_DATA;

        goto exit;
    }

    // Проверяем, что именно LTR34 усановлен
    if((rbuf[read_cntr-1]>>16)!=0x2222)
    {
        res=LTR34_ERROR_NOT_LTR34;
        goto exit;
    }

    // Считываем версию прошивки ПЛИС
    sprintf(module->ModuleInfo.FPGA_Version, "%lu", (rbuf[read_cntr-1])&0x3F);


    // Считываем серийный номер
    res=LTR34_ReadSerialNumber(module, module->ModuleInfo.Serial);
    if (res)
        goto exit;

    // Считываем имя модуля
    res=LTR34_ReadModuleName(module, module->ModuleInfo.Name);
    if (res)
        goto exit;

    // Считываем версию калибровки
    res=LTR34_ReadCalibrVer(module, module->ModuleInfo.CalibrVersion);
    if (res)
        goto exit;

    //Считываем макс. кол-во каналов: 4 или 8
    res=LTR34_ReadChQnt(module, &module->ModuleInfo.MaxChannelQnt);
    if (res)
        goto exit;


    //Считываем калибровочные коэффициенты
    res=LTR34_GetCalibrCoeffs(module);
    if (res)
        goto exit;


exit:	  
    if(res!=LTR_OK) LTR_Close(&module->Channel);
    if(rbuf!=NULL)
        free(rbuf);
    if(warning)
        res=warning;

    return res;
}


/**************************************************************************************

Функция закрытия интерфейсного канала связи с модулем

***************************************************************************************/ 

int LTR34_Close(TLTR34 *module)
{
    INT res;
    if(module!=NULL) res=LTR_Close(&module->Channel);
    else res=LTR_ERROR_PARAMETRS;

    return res;
}

/***************************************************************************************

Функция, определяющая, открыт ли интерфейсный канал связи с модулем

****************************************************************************************/   

int LTR34_IsOpened(TLTR34 *module)
{
    INT res;
    if(module!=NULL)  res=LTR_IsOpened(&module->Channel);
    else res=LTR_ERROR_PARAMETRS;
    return res;
}

/***************************************************************************************

Функция конфигурирования модуля

****************************************************************************************/ 

int LTR34_Config(TLTR34 *module)
{
    int res;
    int i;

    // Проверяем конфигурацию логических каналов

    if(module->ChannelQnt>module->ModuleInfo.MaxChannelQnt)
        return LTR34_ERROR_WRONG_LCH_CONF;

    for(i=0;i<module->ChannelQnt;i++)
    {
        BYTE PhysChannel=(BYTE)((module->LChTbl[i])&0xFF);
        int j;

        if((PhysChannel<1)||(PhysChannel>module->ModuleInfo.MaxChannelQnt))
            return LTR34_ERROR_WRONG_LCH_CONF;

        // анализируем повторы в физ. каналах
        for(j=i+1;j<module->ChannelQnt;j++)
        {
            if(PhysChannel==((module->LChTbl[j])&0xFF))
                return LTR34_ERROR_WRONG_LCH_CONF;
        }
    }

    if (module->ChannelQnt>8||module->FrequencyDivisor>60) return LTR_ERROR_PARAMETRS;
    if((module->ChannelQnt==0)
            ||(module->ChannelQnt==3)
            ||(module->ChannelQnt==5)
            ||(module->ChannelQnt==7)) return LTR_ERROR_PARAMETRS;

    res=LTR34_IsOpened(module);
    if(res==LTR_OK)
    {
        byte NumOfChFactor;
        WORD  config;
        DWORD cmd;

        if(module->ChannelQnt==1)
            NumOfChFactor=0;

        if(module->ChannelQnt==2)
            NumOfChFactor=1;

        if(module->ChannelQnt==4)
            NumOfChFactor=2;

        if(module->ChannelQnt==8)
            NumOfChFactor=3;





        config=((NumOfChFactor&0x3)<<14)|
                ((~(module->AcknowledgeType)&1)<<10)|
                ((module->RingMode&1)<<9)|
                ((module->ExternalStart&1)<<8)|
                ((module->FrequencyDivisor&0xff)<<0)|
                (1<<11);
        cmd=(config<<16)|LTR010CMD|0xE0;

        if(LTR_Send(&module->Channel, &cmd, 1, TIMEOUT_CMD_SEND)==1)
        {
            module->FrequencyDAC=(2*1000000)/(float)((64-module->FrequencyDivisor)*(module->ChannelQnt));
        } else res=LTR34_ERROR;
    }
    return res;
}


/*****************************************************************************************

Функция создания логического канала

*****************************************************************************************/ 


DWORD LTR34_CreateLChannel(BYTE PhysChannel, bool ScaleFlag)
{
    DWORD LChannel;
    // Формирует запись в Таблице Логических Каналов
    // Младший байт - номер физического канала (начиная с единицы)
    // Биты [8..15] - флаг применения выхода 1:1 или 1:10


    LChannel=(BYTE) ScaleFlag;
    LChannel<<=8;
    LChannel|=PhysChannel;

    return(LChannel);

}

/*****************************************************************************************

Функция старта генерации

*****************************************************************************************/ 
int LTR34_DACStart(TLTR34 *module)
{
    INT res=LTR34_IsOpened(module);
    if(res==LTR_OK)
    {
        DWORD cmd=LTR010CMD|0xC0;
        if(LTR_Send(&module->Channel, &cmd, 1, TIMEOUT_CMD_SEND)==1)
        {
            module->DACRunning=true;

        } else res=LTR34_ERROR;
    }
    return res;
}


/**************************************************************************************

Функция аппаратного сброса модуля

****************************************************************************************/

int LTR34_Reset(TLTR34 *module)
{


    INT res=LTR34_IsOpened(module);


    if(res==LTR_OK)
    {
        const int SBUF_SIZE=3;
        DWORD sbuf[SBUF_SIZE];
        DWORD rbuf[2000];
        //int err=LTR_OK;

        int tick;
        int read_cntr;
        sbuf[0]=LTR010CMD_STOP;
        sbuf[1]=LTR010CMD_RESET;
        sbuf[2]=LTR010CMD_STOP;




        if((res=(LTR_Send(&(module->Channel), sbuf, 3, TIMEOUT_CMD_SEND))) <3){
            if(res<0)
                return res;
            else
                return LTR34_ERROR_SEND_CMD;
        }

        res=LTR_OK;

        tick=GetTickCount();

        while(1)
        {
            read_cntr=LTR_Recv(&(module->Channel), rbuf, NULL, 2000, 100); // Загружаем данные, имеющиеся в FIFO
            if (read_cntr<0)
            {
                res=read_cntr;
                break;
            }

            if(read_cntr!=0)
                if((rbuf[read_cntr-1]&0xFFFF8000)==(0x22228000))
                    break;


            if(CalkTickInterval(tick, GetTickCount())>=7000) // В случае превышения 7 с выходим с ошибкой
                return LTR34_ERROR_RESET_MODULE;

        }

    } else res=LTR34_ERROR_CHANNEL_NOT_OPENED;
    return res;
}



/*******************************************************************************************

Функция остановки генерации

********************************************************************************************/ 

int LTR34_DACStop(TLTR34 *module)
{
    INT res=LTR34_IsOpened(module);
    uint slot_num     =module->Channel.cc -1;
    //int cntr=0;
    int tick=0;
    int read_cntr;
    DWORD *stop_buf;


    // Вводим команду СТОП и фиктивное чтение из ППЗУ для получения ответа
    DWORD cmd[2]={ LTR010CMD_STOP, LTR34CMD_IO_ROM } ;

    if(LTR_Send(&module->Channel, cmd, 2, TIMEOUT_CMD_SEND)==2)
    {
        module->DACRunning=false;
    }
    else
    {
        res=LTR34_ERROR;
        return(res);
    }

    stop_buf=(DWORD *) malloc(10000*sizeof(DWORD));

    tick=GetTickCount();

    // Откачиваем данные до появления ответа на ROM_IO
    for(;;)
    {
        read_cntr=LTR_Recv(&module->Channel, stop_buf, NULL, 10000, 100); // Загружаем данные, имеющиеся в FIFO

        if(read_cntr!=0)
        {
            if(read_cntr<0)
            {
                res=read_cntr;
                break;
            }

            if((stop_buf[read_cntr-1]&0xFFFF)==(LTR34CMD_IO_ROM|(slot_num<<8)))
                break;

        }

        if(CalkTickInterval(tick, GetTickCount())>=5000) // В случае превышения 5 с выходим с ошибкой
        {
            res=LTR34_ERROR_CANT_STOP;
            break;
        }

    }

    if(stop_buf!=NULL)
        free(stop_buf);

    return res;
}

/***********************************************************************************************

Функция получения команд или данных из модуля

**********************************************************************************************/ 

int LTR34_Recv(TLTR34 *module, DWORD *data, DWORD *tstamp, DWORD size, DWORD timeout)
{
    int res;
    int i;
    if(module==NULL) return LTR_ERROR_PARAMETRS;

    module->BufferFull=false;
    module->BufferEmpty=false;

    res=LTR_Recv(&module->Channel, data, tstamp, size, timeout);
    if (res>0)
    {
        if (res==(int) size)
        {
            for (i=0;i<(int) size;i++)
            {
                // Анализируем, чему равны флаги переполнения и опустошения FIFO
                if (data[i]&0x11)
                {
                    if (data[i]&0x1)module->BufferFull=true;
                    if (data[i]&0x10)module->BufferEmpty=true;
                }
            }

        }else res=LTR_ERROR_RECV;
    }

    return res;
}


/****************************************************************************

Функция отправки команд или данных в модуль

******************************************************************************/ 

int LTR34_Send(TLTR34 *module, DWORD *data, DWORD size, DWORD timeout)
{	
    return (LTR_Send(&module->Channel, data, size, timeout));

}


/***************************************************************************************

Функция формирования массива для отправки данных в ЦАП

****************************************************************************************/    


int LTR34_ProcessData(TLTR34 *module, double *src, DWORD *dest, DWORD size, bool volt)
{
    int i;
    DWORD ChannelCounter=0;
    //DWORD ChCntrTmp=0;

    int CalibrIndex;
    SHORT DataValue;


    DWORD NumChannels=module->ChannelQnt;

    for (i=0, ChannelCounter=0;i<(int) size;i++)
    {
        double TempValue;
        BYTE PhysChannel=(BYTE) ((module->LChTbl[ChannelCounter]&0xF)-1);
        BYTE OutScaleFlag=(BYTE) (((module->LChTbl[ChannelCounter]>>8)&0xF));

        if(!volt)  // Если входной массив задан в кодах
            TempValue=src[i];
        else  // Если входной массив задан в вольтах
        {
            if(!OutScaleFlag)
                TempValue=src[i]*65535/20.0; // Если используем выход 1:1
            else
                TempValue=src[i]*65535/2.0;  // Если используем выход 1:10

        }  // к else

        if (module->UseClb)
        {

            if(!OutScaleFlag)
                CalibrIndex=2*PhysChannel;

            if(OutScaleFlag)
                CalibrIndex=2*PhysChannel+1;


            TempValue=(TempValue*module->DacCalibration.FactoryCalibrScale[CalibrIndex])
                    +module->DacCalibration.FactoryCalibrOffset[CalibrIndex];
        }


        if(TempValue>=0.0)
            DataValue=(SHORT) (TempValue+0.5);
        else
            DataValue=(SHORT) (TempValue-0.5);



        dest[i]=(DataValue<<16)|(((PhysChannel)&0x7)<<1);

        //	tmp=(((PhysChannel)&0x7)<<1);

        ChannelCounter++;
        if (ChannelCounter>=NumChannels)ChannelCounter=0;
    }

    return LTR_OK;

}


/******************************************************************************************    

Функция записи в ППЗУ

*******************************************************************************************/

int LTR34_WriteFlash(TLTR34 *module, BYTE *data, WORD size, WORD Address)
{
    int i,j,k;
    int err=0;
    int res=0;
    DWORD *command;
    DWORD *command_ack;


    if ((Address+size)>=LTR34_EEPROM_SIZE)
        return LTR34_ERROR_PARAMETERS;

    // Выделяем память под буфера
    command=(DWORD *) malloc(size*(23+6500)*sizeof(DWORD));
    command_ack=(DWORD *) malloc(size*23*sizeof(DWORD));

    // Формируем команду разрешения записи в ППЗУ
    LTR34_MakeEwenFlashCmd(command);

    // Посылаем команды в модуль
    res=LTR34_Send(module, command, 15, TIMEOUT_CMD_SEND);

    if(res!=15)
    {
        if(res<0)
        {
            err=res;
            goto exit;
        }
        else
        {
            err=LTR34_ERROR_CANT_WRITE_FLASH;
            goto exit;
        }
    }


    if((res=(LTR34_Recv(module, command_ack, NULL, 15, TIMEOUT_CMD_RECIEVE)))!=15){ // Получаем ответные команды
        if(res<0)
        {
            err=res;
            goto exit;
        }
        else
        {
            err=LTR34_ERROR_RECV_DATA;
            goto exit;
        }
    }

    // Проверка на правильность ответных команд
    for(i=0;i<15;i++)
    {
        if((command_ack[i]&0xF0FF)!=LTR34CMD_IO_ROM)
        {
            err=LTR34_ERROR_CANT_WRITE_FLASH;
            goto exit;
        }
    }



    for (k=0;k<size;k++)
    {

        LTR34_MakeWriteFlashCmd(Address+k,data[k],command+k*(23+6500));

        for(j=0; j<6500;j++)
        {
            int index=(k+1)*23+6500*k+j;
            command[index]=LTR010CMD_STOP;
        }



    }

    res=LTR34_Send(module, command, size*(23+6500), 10000);
    if(res!=size*(23+6500))
    {
        if(res<0)
        {
            err=res;
            goto exit;
        }
        else
        {
            err=LTR34_ERROR_CANT_WRITE_FLASH;
            goto exit;
        }
    }




    res=LTR34_Recv(module, command_ack, NULL, size*23, 20000);

    if(res!=size*23)
    {
        err=LTR34_ERROR_CANT_WRITE_FLASH;
        goto exit;
    }

    for(i=0;i<size*23;i++)
        if((command_ack[i]&0xF0FF)!=LTR34CMD_IO_ROM)
        {
            err=LTR34_ERROR_CANT_WRITE_FLASH;
            goto exit;
        }



exit:
    if (command!=NULL) free(command);
    if (command_ack!=NULL) free(command_ack);



    return err;

}



/******************************************************************************************

Функция чтения из ППЗУ

*******************************************************************************************/

int LTR34_ReadFlash(TLTR34 *module, BYTE *data, WORD size, WORD Address)
{  
    DWORD *command, *command_ack;
    int k;
    int res=LTR_OK;
    int rcv_cntr=0;
    int i;
    int err=0;

    if ((Address+size)>LTR34_EEPROM_SIZE)return LTR34_ERROR_PARAMETERS;


    if (module->DACRunning)res=LTR34_DACStop(module);
    if (res!=LTR_OK)return res;



    command=(DWORD *) malloc(size*23*sizeof(DWORD));
    command_ack=(DWORD *) malloc(size*23*sizeof(DWORD));


    for (k=0;k<size;k++)
    {
        LTR34_MakeReadFlashCmd(Address+k, command+23*k);
    }

    res=LTR34_Send(module, command, size*23, 10000);
    rcv_cntr=LTR34_Recv(module, command_ack, NULL, size*23, 20000);

    if(rcv_cntr!=size*23)
    {
        err=LTR34_ERROR_CANT_READ_FLASH;
        goto exit;
    }



    for(i=0;i<size*23;i++)
        if((command_ack[i]&0xF0FF)!=LTR34CMD_IO_ROM)
        {
            err=LTR34_ERROR_CANT_READ_FLASH;
            goto exit;
        }



    for (k=0;k<size;k++)
    {
        BYTE ReadByte;

        LTR34_FlashReadRecover(command_ack+k*23, &ReadByte);
        data[k]=ReadByte;

    }

exit:
    if (command!=NULL) free(command);
    if (command_ack!=NULL) free (command_ack);



    return err;
}


/****************************************************************************

Считывает из ППЗУ заводские калибровочные коэффициенты

*****************************************************************************/

int LTR34_GetCalibrCoeffs(TLTR34 *module)
{
    int res=LTR_OK;
    WORD addr;

    addr=CALIBR_ADDR;

    res=LTR34_ReadFlash(module,(byte *)&module->DacCalibration.FactoryCalibrOffset,
                        sizeof(module->DacCalibration.FactoryCalibrOffset), addr/*sizeof(module->ModuleInfo)*/);
    if(res)
        return(res);

    res=LTR34_ReadFlash(module,(byte *)&module->DacCalibration.FactoryCalibrScale,
                        sizeof(module->DacCalibration.FactoryCalibrScale), addr+sizeof(module->DacCalibration.FactoryCalibrOffset)/*sizeof(module->ModuleInfo)*/);
    if(res)
        return(res);


    return res;

}



/*******************************************************************************

Функция записи калибровочных коэффициентов в ППЗУ

********************************************************************************/

int LTR34_WriteCalibrCoeffs(TLTR34 *module)
{
    int res=LTR_OK;
    WORD CRC;
    int tmp;
    WORD addr;

    addr=CALIBR_ADDR;


    // Записываем базовые коэфициенты смещения
    res=LTR34_WriteFlash(module,(byte *)&module->DacCalibration.FactoryCalibrOffset,
                         sizeof(module->DacCalibration.FactoryCalibrOffset), addr/*sizeof(module->ModuleInfo)*/);
    if(res)
        return(res);

    tmp=addr+sizeof(module->DacCalibration.FactoryCalibrOffset);

    // Записываем базовые коэффициенты усиления
    res=LTR34_WriteFlash(module,(byte *)&module->DacCalibration.FactoryCalibrScale,
                         sizeof(module->DacCalibration.FactoryCalibrScale), tmp/*sizeof(module->DacCalibration.FactoryCalibrOffset) //sizeof(module->ModuleInfo)*/);
    if(res)
        return(res);

    tmp=addr+sizeof(module->DacCalibration.FactoryCalibrOffset)+sizeof(module->DacCalibration.FactoryCalibrScale);

    res=LTR34_EvaluateFlashCRC(module, &CRC);
    if(res)
        return(res);

    res=LTR34_WriteFlashCRC(module, CRC);

    return res;
}

/****************************************************************************

Получение строки, описывающей ошибку, соответствующую коду ErrorCode

*****************************************************************************/

LPCSTR LTR34_GetErrorString(INT ErrorCode)
{

    const int err_qnt = sizeof err_str / sizeof err_str[0];
    const int min_err_id=3000;
    LPCSTR    ret_val = NULL;


    //error_range=-min_range_id + 1+sizeof err_str / sizeof err_str[0];

    //	if ((-Error_Code < -min_range_id )|| (Error_Code < -error_range/*-min_range_id + 1+sizeof err_str / sizeof err_str[0]*/))



    if (ErrorCode > -min_err_id || ErrorCode < -(min_err_id + err_qnt))
    {
        ret_val = LTR_GetErrorString(ErrorCode);
    }
    else
    {

        ret_val = err_str[-ErrorCode - min_err_id-1];
    }

    return ret_val;
}

/****************************************************************************  

Функция формирования команд для записи в ППЗУ 

*****************************************************************************/

void  LTR34_MakeWriteFlashCmd(WORD Address, BYTE ByteToWrite, DWORD *FlashCommand)
{

#define WRITE  (0x5) 
    DWORD CurrentCommand;
    int i;

    CurrentCommand=0;
    CurrentCommand|=(WRITE<<19);
    CurrentCommand|=(Address<<8);
    CurrentCommand|=ByteToWrite;


    for (i=0;i<22;i++)
    {
        FlashCommand[i]=LTR34CMD_IO_ROM;
        FlashCommand[i]|=((CurrentCommand>>(22-i-1))&0x1);
        FlashCommand[i]|=0x2;
    }

    FlashCommand[22]=LTR34CMD_IO_ROM;

}

/****************************************************************************  

Функция формирования команд для чтепния из ППЗУ 

*****************************************************************************/

void  LTR34_MakeReadFlashCmd(WORD Address, DWORD *FlashCommand)
{
#define READ   (0x6) 
    DWORD CurrentCommand;
    int i;

    CurrentCommand=0;
    CurrentCommand|=(READ<<11);
    CurrentCommand|=Address;

    for (i=0;i<14;i++)
    {
        FlashCommand[i]=LTR34CMD_IO_ROM;
        FlashCommand[i]|=((CurrentCommand>>(14-i-1))&0x1);
        FlashCommand[i]|=0x2;
    }

    CurrentCommand=LTR34CMD_IO_ROM|0x2;
    for (i=14;i<22;i++)
    {
        FlashCommand[i]=CurrentCommand;
    }
    FlashCommand[22]=LTR34CMD_IO_ROM;
}


/****************************************************************************  

Функция формирования команд для разрешения записи в ППЗУ 

*****************************************************************************/

void  LTR34_MakeEwenFlashCmd(DWORD *FlashCommand)
{
#define EWEN   (0x27FF)   
    int i;

    for (i=0;i<14;i++)
    {
        FlashCommand[i]=LTR34CMD_IO_ROM;
        FlashCommand[i]|=((EWEN>>(14-i-1))&0x1);
        FlashCommand[i]|=0x2;
    }

    FlashCommand[14]=LTR34CMD_IO_ROM;
}

/****************************************************************************  

Функция формирования команд для запрещения записи в ППЗУ 

*****************************************************************************/

void  LTR34_MakeEwdsFlashCmd(DWORD *FlashCommand)
{
#define EWDS   (0x2000)   
    int i;

    for (i=0;i<14;i++)
    {
        FlashCommand[i]=LTR34CMD_IO_ROM;
        FlashCommand[i]|=((EWDS>>(14-i-1))&0x1);
        FlashCommand[i]|=0x2;
    }

    FlashCommand[14]=LTR34CMD_IO_ROM;

}

/****************************************************************************  

Функция формирования байт, считанных из ППЗУ 

*****************************************************************************/

void  LTR34_FlashReadRecover(DWORD *FlashCommand, BYTE *ReadBytes)
{
    int i,j;

    *ReadBytes=0;
    j=7;
    for(i=14;i<22;i++)
    {
        *ReadBytes|=((FlashCommand[i]>>16)&1)<<j;
        j--;
    }

}


/****************************************************************************  

Функция проверки контрольной суммы ППЗУ 

*****************************************************************************/


int LTR34_TestEEPROM(TLTR34 *module)
{
    WORD Flash_CRC;
    INT err;
    BYTE CRC_From_Flash_Bytes[2];
    WORD CRC_From_Flash;


    CRC_From_Flash=0;
    Flash_CRC=0;

    err=LTR34_EvaluateFlashCRC(module, &Flash_CRC);
    if (err) return(err);

    err=LTR34_ReadFlash(module, CRC_From_Flash_Bytes, 2, CRC_ADDR); //Считываем КС по адр. 2001
    if (err) return(err);


    CRC_From_Flash=CRC_From_Flash_Bytes[1];
    CRC_From_Flash<<=8;
    CRC_From_Flash|=CRC_From_Flash_Bytes[0];


    if(Flash_CRC!=CRC_From_Flash)
        return LTR34_ERROR_WRONG_FLASH_CRC;

    return LTR_OK;
}



/****************************************************************************  

Функция расчета контрольной суммы ППЗУ 

*****************************************************************************/


INT LTR34_EvaluateFlashCRC(TLTR34 *module, WORD *Flash_CRC)

{
    INT Flash_Size=WORKING_FLASH_SIZE;//510; // Рамер контролируемого объема ППЗУ в байтах
    BYTE Flash_Byte_CRC;
    BYTE Flash_Content[2000];
    INT i;
    INT err;

    Flash_Byte_CRC=0;
    *Flash_CRC=0;

    err=LTR34_ReadFlash(module, Flash_Content, Flash_Size, 0);
    if (err) return(err);

    for(i=0;i<Flash_Size;i++)
    {

        Flash_Byte_CRC=Flash_Content[i];
        UpdateCRC16(Flash_CRC, Flash_Byte_CRC);
    }

    return LTR_OK;
}


/****************************************************************************  

Функция записи контрольной суммы в ППЗУ 

*****************************************************************************/


INT LTR34_WriteFlashCRC(TLTR34 *module, WORD Flash_CRC)
{
    INT err;
    BYTE CRC_HByte, CRC_LByte;
    BYTE CRC_Bytes[2];

    CRC_LByte=Flash_CRC&0x00FF;       // Младший байт контрольной суммы
    CRC_HByte=(Flash_CRC>>8)&0x00FF;   //Старший байт контрольной суммы


    CRC_Bytes[0]=CRC_LByte;
    CRC_Bytes[1]=CRC_HByte;

    err=LTR34_WriteFlash(module, CRC_Bytes, 2, CRC_ADDR); //Записываем КС по адр. CRC_ADDR
    if (err) return(err);


    return LTR_OK;
}


/****************************************************************************  

Функция записи серийного номера в ППЗУ

*****************************************************************************/


INT LTR34_WriteSerialNumber(TLTR34 *module, CHAR *sn, int Code)
{
    uint sn_length; // Длина строки серийного номера
    INT err;
    //INT slot_num     =module->Channel.cc -1;
    INT EEPROM_Addr=SERIAL_ADDR;
    WORD Flash_CRC;
    CHAR Module_Name[8]="LTR34";
    int CodeFromFile;
    FILE *f;

    f=fopen("serialcode.dat","r");

    if(f==NULL)
        return LTR34_ERROR_CANT_WRITE_SERIAL_NUM;

    fscanf(f,"%d", &CodeFromFile);

    if(Code!=CodeFromFile)
        return LTR34_ERROR_CANT_WRITE_SERIAL_NUM;

    sn_length=(int)strlen(sn);

    if(sn_length>(sizeof(module->ModuleInfo.Serial)-1)/*-1*//*17*/)
        return LTR34_ERROR_CANT_WRITE_SERIAL_NUM;

    err=LTR34_WriteFlash(module, (BYTE *) sn, 16, EEPROM_Addr);
    if(err) return(err);

    // Записываем имя модуля в ППЗУ

    EEPROM_Addr=MODULE_NAME_ADDR;

    err=LTR34_WriteFlash(module, (BYTE *) Module_Name, (WORD)strlen(Module_Name), EEPROM_Addr);
    if(err) return(err);

    // Сюда ввести подсчет и запись контрольной суммы!!!


    err=LTR34_EvaluateFlashCRC(module, &Flash_CRC);
    if(err) return(err);

    err=LTR34_WriteFlashCRC(module, Flash_CRC);
    if(err) return(err);

    return LTR_OK;
}



/************************************************************************************************
Внимание! Распределение памяти в ППЗУ!

Серийный номер:  адрес 0;
Имя модуля: адрес 20;
Версия прошивки ПЛИС: адрес 30;
Калибровочные коэффициенты: адрес 100-228 
228-2047 зарезервировано для возможных дополнительных калибровочных коэффициентов

*************************************************************************************************/


/*****************************************************************************

Функция чтения серийного номера из ППЗУ

******************************************************************************/   

INT LTR34_ReadSerialNumber(TLTR34 *module, CHAR *sn)
{
    INT err;
    INT EEPROM_Addr=SERIAL_ADDR;
    INT string_flag=0;


    err=LTR34_ReadFlash(module, (BYTE *) sn, 16, EEPROM_Addr);
    if(err)
        return(err);

    for(uint i=0;i<sizeof(module->ModuleInfo.Serial);i++)
        if(sn[i]=='\0')
        {
            string_flag=1;
            break;
        }

    if(!string_flag)
        sn[0]='\0';


    return LTR_OK;
}




/*****************************************************************************

Функция записи имени модуля в ППЗУ

******************************************************************************/   

INT LTR34_WriteModuleName(TLTR34 *module, CHAR *ModuleName, int Code)
{	
    INT err;
    //INT slot_num     =module->Channel.cc -1;
    INT EEPROM_Addr=SERIAL_ADDR;
    WORD Flash_CRC;
    int CodeFromFile;
    FILE *f;



    f=fopen("serialcode.dat","r");

    if(f==NULL)
        return LTR34_ERROR_CANT_WRITE_MODULE_NAME;

    fscanf(f,"%d", &CodeFromFile);

    if(Code!=CodeFromFile)
        return LTR34_ERROR_CANT_WRITE_MODULE_NAME;


    // Записываем имя модуля в ППЗУ

    EEPROM_Addr=MODULE_NAME_ADDR;

    err=LTR34_WriteFlash(module, (BYTE *) ModuleName, (WORD)strlen(ModuleName), EEPROM_Addr);
    if(err) return(err);

    // Сюда ввести подсчет и запись контрольной суммы!!!


    err=LTR34_EvaluateFlashCRC(module, &Flash_CRC);
    if(err) return(err);

    err=LTR34_WriteFlashCRC(module, Flash_CRC);
    if(err) return(err);

    return LTR_OK;
}



/*****************************************************************************

Функция чтения имени модуля из ППЗУ

******************************************************************************/   


INT LTR34_ReadModuleName(TLTR34 *module, CHAR *ModuleName)
{
    INT EEPROM_Addr=MODULE_NAME_ADDR;
    INT string_flag;


    LTR34_ReadFlash(module, (BYTE *) ModuleName,7, EEPROM_Addr);


    for(uint i=0;i<sizeof(module->ModuleInfo.Name);i++)
        if(ModuleName[i]=='\0')
        {
            string_flag=1;
            break;
        }

    if(!string_flag)
        ModuleName[0]='\0';



    return LTR_OK;
}


/******************************************************************************

Функция записи версии прошивки ПЛИС в ППЗУ

*******************************************************************************/   


INT LTR34_WriteFPGA_Ver(TLTR34 *module, CHAR *FPGA_Ver, int Code)
{
    uint sn_length; // Длина строки серийного номера
    INT err;
    //INT slot_num     =module->Channel.cc -1;
    INT EEPROM_Addr=FPGA_VER_ADDR;
    WORD Flash_CRC;
    int CodeFromFile;
    FILE *f;

    f=fopen("serialcode.dat","r");

    if(f==NULL)
        return LTR34_ERROR_CANT_WRITE_FPGA_VER;

    fscanf(f,"%d", &CodeFromFile);

    if(Code!=CodeFromFile)
        return LTR34_ERROR_CANT_WRITE_FPGA_VER;



    sn_length=(int)strlen(FPGA_Ver);

    if(sn_length>(sizeof(module->ModuleInfo.FPGA_Version)-1))
        return LTR34_ERROR_CANT_WRITE_FPGA_VER;

    err=LTR34_WriteFlash(module, (BYTE *) FPGA_Ver, 3, EEPROM_Addr);
    if(err) return(err);


    // Подсчет и запись контрольной суммы


    err=LTR34_EvaluateFlashCRC(module, &Flash_CRC);
    if(err) return(err);

    err=LTR34_WriteFlashCRC(module, Flash_CRC);
    if(err) return(err);

    return LTR_OK;
}






/******************************************************************************

Функция чтения версии прошивки ПЛИС из ППЗУ

*******************************************************************************/   

INT LTR34_Read_FPGA_Ver(TLTR34 *module, CHAR FPGA_Ver[7])
{
    INT EEPROM_Addr=FPGA_VER_ADDR;
    INT err;
    int string_flag=0;

    err=LTR34_ReadFlash(module, (BYTE *) FPGA_Ver,3, EEPROM_Addr);
    if(err) return(err);

    //Все же разобраться с количеством символов для слов

    for(uint i=0;i<sizeof(module->ModuleInfo.FPGA_Version);i++)
        if(FPGA_Ver[i]=='\0')
        {
            string_flag=1;
            break;
        }

    if(!string_flag)
        FPGA_Ver[0]='\0';


    return LTR_OK;

}



/******************************************************************************

Функция записи версии калибровки в ППЗУ

*******************************************************************************/   


INT LTR34_WriteCalibrVer(TLTR34 *module, CHAR *CalibrVer, int Code)
{
    uint sn_length; // Длина строки серийного номера
    INT err;
    //INT slot_num     =module->Channel.cc -1;
    INT EEPROM_Addr=CALIBR_VER_ADDR;
    WORD Flash_CRC;
    int CodeFromFile;
    FILE *f;

    f=fopen("serialcode.dat","r");

    if(f==NULL)
        return LTR34_ERROR_CANT_WRITE_CALIBR_VER;

    fscanf(f,"%d", &CodeFromFile);

    if(Code!=CodeFromFile)
        return LTR34_ERROR_CANT_WRITE_CALIBR_VER;

    sn_length=strlen(CalibrVer);

    if(sn_length>(sizeof(module->ModuleInfo.CalibrVersion)-1)/*-1*//*17*/)
        return LTR34_ERROR_CANT_WRITE_CALIBR_VER;

    err=LTR34_WriteFlash(module, (BYTE *) CalibrVer, sn_length+1, EEPROM_Addr);
    if(err) return(err);

    // Подсчет и запись контрольной суммы
    err=LTR34_EvaluateFlashCRC(module, &Flash_CRC);
    if(err) return(err);

    err=LTR34_WriteFlashCRC(module, Flash_CRC);
    if(err) return(err);

    return LTR_OK;
}


/******************************************************************************

Функция чтения версии калибровки из ППЗУ

*******************************************************************************/   

INT LTR34_ReadCalibrVer(TLTR34 *module, CHAR *CalibrVer)
{
    INT EEPROM_Addr=CALIBR_VER_ADDR;
    INT err;
    INT string_flag=0;



    err=LTR34_ReadFlash(module, (BYTE *) CalibrVer,sizeof(module->ModuleInfo.CalibrVersion), EEPROM_Addr);
    if(err) return(err);

    for(uint i=0;i<sizeof(module->ModuleInfo.CalibrVersion);i++)
    {
        if(CalibrVer[i]=='\0')
        {
            string_flag=1;
            break;
        }
    }

    if(!string_flag)
    {
        CalibrVer[0]='1';
        CalibrVer[1]='\0';
    }

    return LTR_OK;
}



/******************************************************************************

Функция записи максимального числа каналов (4 или 8) в ППЗУ

*******************************************************************************/   


INT LTR34_WriteChQnt(TLTR34 *module, BYTE ChQnt, int Code)
{

    INT err;
    //INT slot_num     =module->Channel.cc -1;
    INT EEPROM_Addr=CH_QNT_ADDR;
    WORD Flash_CRC;
    int CodeFromFile;
    FILE *f;


    f=fopen("serialcode.dat","r");

    if(f==NULL)
        return LTR34_ERROR_CANT_WRITE_MAX_CH_QNT;

    fscanf(f,"%d", &CodeFromFile);

    if(Code!=CodeFromFile)
        return LTR34_ERROR_CANT_WRITE_MAX_CH_QNT;



    err=LTR34_WriteFlash(module, &ChQnt, 1, EEPROM_Addr);
    if(err) return(err);


    // Подсчет и запись контрольной суммы


    err=LTR34_EvaluateFlashCRC(module, &Flash_CRC);
    if(err) return(err);

    err=LTR34_WriteFlashCRC(module, Flash_CRC);
    if(err) return(err);

    return LTR_OK;
}


/******************************************************************************

Функция чтения максимального числа каналов  (4 или 8)

*******************************************************************************/   

INT LTR34_ReadChQnt(TLTR34 *module, BYTE *ChQnt)
{
    INT EEPROM_Addr=CH_QNT_ADDR;
    INT err;
    err=LTR34_ReadFlash(module, ChQnt,1, EEPROM_Addr);
    if(err) return(err);


    return LTR_OK;

}






static WORD UpdateCRC16(WORD *crc, BYTE b)
{
    WORD w;
    INT j;
    w = *crc;
    w ^= b << 8;
    for(j=0;j<8;j++)  {
        if(w&0x8000) w = (w<<1) ^ 0x1021;
        else w <<= 1;
    }
    *crc = w;
    return w;
}


static DWORD CalkTickInterval(DWORD PrevTick, DWORD ActTick)
{
    DWORD Diff;

    if(ActTick>=PrevTick)
        Diff=ActTick-PrevTick;
    else
        Diff=(0xFFFFFFFF-PrevTick)+ActTick;

    return Diff;

}

