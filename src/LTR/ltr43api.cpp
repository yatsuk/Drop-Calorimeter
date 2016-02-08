#include <stdio.h> 
#include <stdlib.h>
#include "ltr43api.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

// Коды команд AVR
#define READ_WORD                                     (0x80C1)
#define CONFIG                                        (0x80C2)  
#define START_SECOND_MARK                             (0x80C3)
#define STOP_SECOND_MARK    						  (0x80C4)
#define MAKE_START_MARK                				  (0x80C5)
#define RS485_CONFIG                				  (0x80C6)  
#define RS485_SEND_BYTE                				  (0x80C7)  

#define WRITE_EEPROM                                  (0x80C8) 
#define READ_EEPROM                                   (0x80C9)
#define READ_CONF_RECORD                              (0x80CA) 
#define RS485_RECEIVE                    			  (0x80CB)  
#define RS485_STOP_RECEIVE							  (0x80CC)  
#define START_STREAM_READ							  (0x80CD) 
#define STOP_STREAM_READ                              (0x80CE)
#define CONFIG_READ_RATE                              (0x80D0)


#define INIT                						  (0x80CF)

#define DATA_OUTPUT_CONFIRM     					   (0x16)       
#define PARITY_ERROR                                   (0x17)
#define RS485_ERROR_CONFIRM_TIMEOUT					   (0x18)
#define RS485_ERROR_SEND_TIMEOUT					   (0x19)
#define DATA_ERROR                                     (0x1A) 




#define F_OSC  (15000000) // Тактовая частота микроконтроллера AVR 
#define CMD_TIMEOUT (2000)
#define MODULE_ID (0x2B)

#define TIMEOUT_CMD_SEND								    (4000)
#define TIMEOUT_CMD_RECIEVE								    (6000)



int LTR43_ReadConfRecord(PTLTR43 hnd, CHAR *version, CHAR *date, CHAR *name, CHAR *serial);
int LTR43_RS485_Config(PTLTR43 hnd);                                                                  

INT eval_parity(DWORD cmd);                                                                                
INT check_parity(DWORD cmd); 
INT eval_scalers(double *Freq, WORD *Prescaler, BYTE *PrescalerCode, BYTE *Scaler);

INT HC_LE41_CONNECT(PTLTR43 hnd);                                                                     
static WORD CRC_Calc(BYTE *b, WORD N);                                                                     
static void CRC_Update(WORD *crc, BYTE *b, int c);  
static DWORD CalkTickInterval(DWORD PrevTick, DWORD ActTick);    

/****************************************************************************************************************
  
      Функция заполнения структуры описания модуля значениями по умолчанию и инициализации интерфейсного канала

 ****************************************************************************************************************/


INT LTR43_Init(PTLTR43 hnd)
{
    int err;


    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    hnd->size=sizeof(TLTR43);

    // По умолчания все линии ввода-вывода уст. на вход
    hnd->IO_Ports.Port1=0;
    hnd->IO_Ports.Port3=0;
    hnd->IO_Ports.Port3=0;
    hnd->IO_Ports.Port4=0;

    // По умолчанию частота выдачи данных при потоковом сборе 15 кГц
    hnd->StreamReadRate=15000;

    // RS485
    hnd->RS485.FrameSize=8;
    hnd->RS485.Baud=4800;
    hnd->RS485.StopBit=0;  // 1 стоп-бит
    hnd->RS485.Parity=0;   // Проверка на четность выключена


    // Инициализация множителей таймаутов
    hnd->RS485.SendTimeoutMultiplier=10;
    hnd->RS485.ReceiveTimeoutMultiplier=10;


    strcpy(hnd->ModuleInfo.FirmwareVersion,"\0");
    strcpy(hnd->ModuleInfo.FirmwareDate,"\0");
    strcpy(hnd->ModuleInfo.Serial,"\0");
    strcpy(hnd->ModuleInfo.Name,"\0");



    err=LTR_Init(&(hnd->Channel));		// Инициализируем интерефейсный канал
    if(err==LTR_OK)
        return LTR43_NO_ERR;
    else return err;

}



/**********************************************************************************

   Функция проверки: не открыт ли канал
   
************************************************************************************/
INT LTR43_IsOpened(PTLTR43 hnd)
{

    INT err;
    if(hnd!=NULL)  err=LTR_IsOpened(&hnd->Channel);
    else err=LTR_ERROR_PARAMETRS;
    return err;

}

/**********************************************************************************

   Функция создания и открытия канала связи с LTR43. Также RESET модуля
   
************************************************************************************/


INT LTR43_Open(PTLTR43 hnd, DWORD net_addr, WORD net_port, CHAR *crate_sn, INT slot_num)
{

    DWORD command[2], stop[1], rbuf[1024]; // Массивы для отправки и приема данных в/из ДСП
    int read_cntr;
    int tick;

    int err=0;
    int warning=0;

    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;


    
    // Устанавливаем параметры канала

    hnd->Channel.saddr=net_addr; // Сетевой адрес

    if(crate_sn==NULL)
        return LTR43_ERR_INVALID_CRATE_SN; // Серийный номер

    sprintf(hnd->Channel.csn, crate_sn);


    if((slot_num<1)||(slot_num)>16)
        return LTR43_ERR_INVALID_SLOT_NUM;

    hnd->Channel.cc=slot_num;

    slot_num-=1;

    // Открываем созданный интерфейсный канал связи с модулем
    if((err=LTR_Open(&(hnd->Channel)))!=LTR_OK){
        if(err<0){
            if(err==LTR_WARNING_MODULE_IN_USE)
            {
                warning=err;
                err=LTR_OK;
            }
            else
                return err;
        }
        else
            return LTR43_ERR_CANT_OPEN;
    }

    // Инициализация модуля: посылаем команды: СТОП, СБРОС, СТОП

    command[0]=LTR010CMD_STOP|(slot_num<<8);
    stop[0]=LTR010CMD_STOP|(slot_num<<8);



    if((err=(LTR_Send(&(hnd->Channel), stop, 1, TIMEOUT_CMD_SEND))) != 1 )  // Посылаем команду "STOP"
        return LTR43_ERR_CANT_SEND_COMMAND;


    command[0]=LTR010CMD_RESET|(slot_num<<8);;
    if((err=(LTR_Send(&(hnd->Channel), command, 1, TIMEOUT_CMD_SEND))) != 1 ){  //Посылаем команду "RESET"
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_RESET_MODULE;
    }




    tick=GetTickCount();
    


    while(1)
    {
        read_cntr=LTR_Recv(&(hnd->Channel), rbuf, NULL, 1024, 100); // Загружаем данные, имеющиеся в FIFO

        if(read_cntr!=0)
            if((rbuf[read_cntr-1]&0xFFFF0000)==(0x2B2B0000))
                break;



        if(CalkTickInterval(tick, GetTickCount())>=5000) // В случае превышения 5 с выходим с ошибкой
            return LTR43_ERR_CANT_RESET_MODULE;

    }


    if((err=(LTR_Send(&(hnd->Channel), stop, 1, TIMEOUT_CMD_SEND))) != 1){  // Посылаем команду "STOP"
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }


    Sleep(500);
    command[0]=INIT|(slot_num<<8);

    if((err=(LTR_Send(&(hnd->Channel), command, 1, TIMEOUT_CMD_SEND))) != 1 ){  // Посылаем первую команду INSTR
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }

    if((err=(LTR_Recv(&(hnd->Channel), rbuf, NULL, 1, TIMEOUT_CMD_RECIEVE))!=1)){   // Получаем ответ
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }

    err=LTR43_ReadConfRecord(hnd, hnd->ModuleInfo.FirmwareVersion, hnd->ModuleInfo.FirmwareDate, hnd->ModuleInfo.Name, hnd->ModuleInfo.Serial);
    if(err) return err;


    if(warning)
        return(warning);
    else
        return LTR43_NO_ERR;
}


/***************************************************************************************

               Функция окончания работы с модулем
               
****************************************************************************************/               

INT LTR43_Close(PTLTR43 hnd)
{
    int err=0;

    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    if((err=LTR43_StopSecondMark(hnd)))
        return(err);

    if ((err=LTR_Close(&(hnd->Channel)))!=LTR_OK)
        return(err);

    return LTR43_NO_ERR;
}


/**************************************************************************************

                 Функция конфигурирования модуля

**************************************************************************************/


INT LTR43_Config(PTLTR43 hnd)

{
    DWORD command[2];
    DWORD command_ack[2];
    DWORD slot_num   =hnd->Channel.cc -1;
    BYTE Output_Groups=0;
    BYTE Marks_Conf;
    WORD Prescaler;
    BYTE PrescalerCode;
    BYTE Scaler;
    int i;

    int err=0;


    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;


    if(((hnd->IO_Ports.Port1!=0)&&(hnd->IO_Ports.Port1!=1))||((hnd->IO_Ports.Port2!=0)&&(hnd->IO_Ports.Port2!=1))||((hnd->IO_Ports.Port3!=0)&&(hnd->IO_Ports.Port3!=1))||((hnd->IO_Ports.Port4!=0)&&(hnd->IO_Ports.Port4!=1)))
        return LTR43_ERR_WRONG_IO_LINES_CONF;

    // Слово определяет конфигурацию линий ввода вывода
    Output_Groups=(hnd->IO_Ports.Port1)|((hnd->IO_Ports.Port2)<<1)|((hnd->IO_Ports.Port3)<<2)|((hnd->IO_Ports.Port4)<<3);



    if((hnd->Marks.StartMark_Mode<0)||(hnd->Marks.StartMark_Mode>2))
        return LTR43_ERR_WRONG_START_MARK_CONF;

    if((hnd->Marks.SecondMark_Mode<0)||(hnd->Marks.StartMark_Mode>2))
        return LTR43_ERR_WRONG_SECOND_MARK_CONF;

    if((hnd->StreamReadRate<100.0)||(hnd->StreamReadRate>100000.0))
        return LTR43_ERR_WRONG_STREAM_READ_FREQ_SETTINGS;





    Marks_Conf=hnd->Marks.StartMark_Mode;
    Marks_Conf<<=4;
    Marks_Conf|=hnd->Marks.SecondMark_Mode;



    //hnd->StreamReadFreq=1000;

    eval_scalers(&(hnd->StreamReadRate), &Prescaler, &PrescalerCode, &Scaler);

    command[0]=Marks_Conf;
    command[0]<<=8;
    command[0]|=Output_Groups;
    command[0]<<=16;
    command[0]|=CONFIG;
    command[0]|=(slot_num<<8);

    command[0]|=(eval_parity(command[0]))<<5;


    // В младшем байте контекста команды будет значение кода предделителя, в старшем - значение делителя.
    command[1]=Scaler;
    command[1]<<=8;
    command[1]|=PrescalerCode;
    command[1]<<=16;
    command[1]|=CONFIG_READ_RATE;
    command[1]|=(slot_num<<8);

    command[1]|=(eval_parity(command[1]))<<5;




    if((err=(LTR_Send(&(hnd->Channel), command, 2, TIMEOUT_CMD_SEND)))!= 2 ){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }

    if((err=(LTR_Recv(&(hnd->Channel), command_ack, NULL, 2, TIMEOUT_CMD_RECIEVE)))!=2){
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }


    for(i=0;i<2;i++)
    {
        // Проверка на четность пришедшего пакета

        if(check_parity(command_ack[i]))
            return LTR43_ERR_PARITY_FROM_MODULE;


        if((command_ack[i]&0x0000001F)==PARITY_ERROR)
            return LTR43_ERR_PARITY_TO_MODULE;


        if((command_ack[i]&0x0000FF1F)!=(command[i]&0x0000FF1F)) // Проверяем отв. команду, предварительно "убив" бит четности
            return LTR43_ERR_CANT_CONFIG;

    }
    
    // Выполняем конфигурацию RS485
    
    err=LTR43_RS485_Config(hnd);
    if(err) return(err);

    return LTR43_NO_ERR;
}


/**************************************************************************************

    Функция отправки 32-битного слова в выходной порт модуля
    
**************************************************************************************/    

INT LTR43_WritePort(PTLTR43 hnd, DWORD OutputData)
{

    DWORD data[4];
    DWORD command_ack;
    DWORD slot_num   =hnd->Channel.cc -1;
    int err=0;
    DWORD val;


    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;


    val=OutputData;


    // Посылаем команду 4 пакета данных, т.е. две пары одинаковых пакетов. Каждая пара определяет 32-битное слово


    data[0]=(val&0xFFFF0000)|1|(slot_num<<8); // Первым отправляем пакет со старшими 2-мя байтами
    data[1]=((val&0x0000FFFF)<<16)|(slot_num<<8); // Затем пакет с младшими 2-мя байтами

    data[2]=(val&0xFFFF0000)|1|(slot_num<<8); // Первым отправляем пакет со старшими 2-мя байтами
    data[3]=((val&0x0000FFFF)<<16)|(slot_num<<8); // Затем пакет с младшими 2-мя байтами


    if((err=(LTR_Send(&(hnd->Channel), data, 4, TIMEOUT_CMD_SEND))) !=4 ){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_DATA;
    }


    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }

    
    // Проверка на четность пришедшей команды
    
    if(check_parity(command_ack))
        return LTR43_ERR_PARITY_FROM_MODULE;

    // Если пришла ошибка четности
    if((command_ack&0x0000001F)==PARITY_ERROR)
        return LTR43_ERR_PARITY_TO_MODULE;
    

    if(((command_ack)&0x1F)==DATA_ERROR)
        return LTR43_ERR_DATA_TRANSMISSON_ERROR;

    if(((command_ack)&0x00000F1F)!=(DATA_OUTPUT_CONFIRM|(slot_num<<8)))
        return LTR43_ERR_CANT_SEND_DATA;

    return LTR43_NO_ERR;
}


/****************************************************************************************

   Функция чтения 32-битного слова из входного порта модуля
   
****************************************************************************************/   


INT LTR43_ReadPort(PTLTR43 hnd, DWORD *InputData)
{
    DWORD command[1];
    DWORD data[2];
    DWORD slot_num   =hnd->Channel.cc -1;
    DWORD val;

    int i;
    int err=0;


    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;


    for(i=0;i<2;i++)
        data[i]=0;


    command[0]=0;
    command[0]|=READ_WORD;
    command[0]|=(slot_num<<8);

    //Здесь не делаем проверку на четность, т.к. это дополнительное время. Правильно ли выполнилась команда- станет ясно по ответу

    if((err=(LTR_Send(&(hnd->Channel), command, 1, TIMEOUT_CMD_SEND))) != 1 )
    {
        if(err<0)
            return(err);
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }

    if((err=(LTR_Recv(&(hnd->Channel), data, NULL, 2, TIMEOUT_CMD_RECIEVE)))!=2)  //Было 2
    {
        if(err<0)
            return(err);
        else
            return LTR43_ERR_CANT_READ_DATA;
    }
    
    
    for(i=0;i<2;i++)
        if((data[i]&0x8000))
            return LTR43_ERR_CANT_READ_DATA;


    if(((data[1]&0xFF)-(data[0]&0xFF))!=1)
        if(((data[0]&0xFF)!=255)&&((data[1]&0xFF)!=0))
            return LTR43_ERR_CANT_READ_DATA;

    
    
    val=((data[0]&0xFFFF0000)|(data[1]>>16));
    
    *InputData=val;
    
    return LTR43_NO_ERR;
}


/****************************************************************************************

   Функция посылки массива данных в модуль
   
****************************************************************************************/   


INT LTR43_WriteArray(PTLTR43 hnd, DWORD *OutputArray, BYTE ArraySize)
{

    DWORD command_ack;
    DWORD *data;
    DWORD slot_num   =hnd->Channel.cc -1;
    int i;
    int err=0;


    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;


    data=(DWORD *) calloc(4*ArraySize, sizeof(DWORD));
    if(data==NULL)
        return LTR_ERROR_MEMORY_ALLOC;


    for(i=0;i<ArraySize;i++)
    {
        data[4*i]=(OutputArray[i]&0xFFFF0000)|ArraySize|(slot_num<<8); // Первым отправляем пакет со старшими 2-мя байтами
        data[4*i+1]=((OutputArray[i]&0x0000FFFF)<<16)|(slot_num<<8); // Затем пакет с младшими 2-мя байтами
        data[4*i+2]=(OutputArray[i]&0xFFFF0000)|(slot_num<<8); // Первым отправляем пакет со старшими 2-мя байтами
        data[4*i+3]=((OutputArray[i]&0x0000FFFF)<<16)|(slot_num<<8); // Затем пакет с младшими 2-мя байтами

    }

    if((err=(LTR_Send(&(hnd->Channel), data, 4*ArraySize, TIMEOUT_CMD_SEND))) !=4*ArraySize ){
        if(err<0)
            goto exit;
        else
        {
            err=LTR43_ERR_CANT_SEND_DATA;
            goto exit;
        }
    }

    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            goto exit;
        else
        {
            err=LTR43_ERR_MODULE_NO_RESPONCE;
            goto exit;
        }
    }

    err=LTR_OK;

    if(check_parity(command_ack))
    {
        err=LTR43_ERR_PARITY_FROM_MODULE;
        goto exit;
    }
    

    if(((command_ack)&0x1F)==(DATA_ERROR|(slot_num<<8)))
    {
        err=LTR43_ERR_DATA_TRANSMISSON_ERROR;
        goto exit;
    }

    if(((command_ack)&0xF1F)!=(DATA_OUTPUT_CONFIRM|(slot_num<<8)))
    {
        err=LTR43_ERR_CANT_SEND_DATA;
        goto exit;
    }

exit:

    if(!err)
        err=LTR43_NO_ERR;

    free(data);

    
    return err;


}


/****************************************************************************************

   Функция старта потокового чтения данных из модуля
   
****************************************************************************************/   


INT LTR43_StartStreamRead(PTLTR43 hnd)
{

    DWORD command;
    DWORD command_ack;
    DWORD slot_num   =hnd->Channel.cc -1;
    int err=0;

    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    command=START_STREAM_READ;
    command|=(slot_num<<8);

    command|=(eval_parity(command))<<5;

    if((err=(LTR_Send(&(hnd->Channel), &command, 1, TIMEOUT_CMD_SEND))) != 1 ){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }
    
    

    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }
    

    if(check_parity(command_ack))
        return LTR43_ERR_PARITY_FROM_MODULE;


    if((command_ack&0x0000001F)==PARITY_ERROR)
        return LTR43_ERR_PARITY_TO_MODULE;
    

    if((command_ack&0x0000FF1F)!=(command&0x0000FF1F))
        return LTR43_ERR_CANT_START_STREAM_READ;


    return LTR43_NO_ERR;

}



/****************************************************************************************

   Функция получения массива входных входных данных при непрерывном вводе
   
****************************************************************************************/ 

INT LTR43_Recv(PTLTR43 hnd, DWORD *data, DWORD *tmark, DWORD size, DWORD timeout)
{
    INT res=LTR_Recv(&(hnd->Channel), data, tmark, size, timeout);

    if(res>0)
        if(hnd->Channel.flags&FLAG_RBUF_OVF)
            res=LTR43_ERR_ERROR_OVERFLOW;
    
    return res;
}


/****************************************************************************************

   Функция проверки входных данных при непрерывном вводе и формирования 32-битных слов
   
****************************************************************************************/ 

INT LTR43_ProcessData(PTLTR43 hnd, DWORD *src, DWORD *dest, DWORD *size)
{

    DWORD i=0;
    DWORD j=0;

    for (i=0;i<*size;i+=2)
    {


        if((src[i]&0x8000)||(src[i+1]&0x8000))
            return LTR43_ERR_WRONG_IO_DATA;


        if(((src[i+1]&0xFF)-(src[i]&0xFF))!=1)
            if(((src[i]&0xFF)!=255)&&((src[i+1]&0xFF)!=0))
                return LTR43_ERR_WRONG_IO_DATA;

        /*if(i)
    if((src[i]&0xFF)==0)
     if((src[i-1]&0xFF)!=255)
      return LTR43_ERR_WRONG_IO_DATA;   */

        dest[j]=((src[i]&0xFFFF0000)|(src[i+1]>>16));
        j++;
    }

    *size=j;

    return LTR43_NO_ERR;
}







/****************************************************************************************

   Функция остановки потокового чтения данных из модуля
   
****************************************************************************************/   


INT LTR43_StopStreamRead(PTLTR43 hnd)
{
    DWORD command;
    DWORD *stop_buf;
    DWORD slot_num   =hnd->Channel.cc -1;
    int read_cntr;
    int tick;
    int err=0;

    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;


    command=STOP_STREAM_READ;
    command|=(slot_num<<8);

    command|=(eval_parity(command))<<5;

    if((err=(LTR_Send(&(hnd->Channel), &command, 1, TIMEOUT_CMD_SEND))) != 1 ){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }
    
    err=LTR_OK;
    


    stop_buf=(DWORD *) calloc(15000, sizeof(DWORD));
    if(stop_buf==NULL)
        return LTR_ERROR_MEMORY_ALLOC;


    tick=GetTickCount();
    
    while(1)
    {
        read_cntr=LTR_Recv(&(hnd->Channel), stop_buf, NULL, 15000, 100); // Загружаем данные, имеющиеся в FIFO

        if(read_cntr!=0)
            if((stop_buf[read_cntr-1]&0xFFDF)==(STOP_STREAM_READ|(slot_num<<8)))
                break;



        if(CalkTickInterval(tick, GetTickCount())>=5000) // В случае превышения 5 с выходим с ошибкой
        {
            err=LTR43_ERR_CANT_STOP_STREAM_READ;
            goto exit;
        }

    }

    if(check_parity(stop_buf[read_cntr-1]))
    {
        err=LTR43_ERR_PARITY_FROM_MODULE;
        goto exit;
    }

exit:

    free(stop_buf);
    return err;
}



/****************************************************************************************

                Функция включения генерации секундной метки

****************************************************************************************/   

INT LTR43_StartSecondMark(PTLTR43 hnd)
{

    DWORD command;
    DWORD command_ack;
    DWORD slot_num   =hnd->Channel.cc -1;
    int err=0;


    //int CommandPacks_Qntr=0;


    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    command=0x0;
    command|=START_SECOND_MARK;
    command|=(slot_num<<8);

    command|=(eval_parity(command))<<5;

    if((err=(LTR_Send(&(hnd->Channel), &command, 1, TIMEOUT_CMD_SEND))) != 1 ){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }

    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }
    

    if(check_parity(command_ack))
        return LTR43_ERR_PARITY_FROM_MODULE;


    if((command_ack&0x0000001F)==PARITY_ERROR)
        return LTR43_ERR_PARITY_TO_MODULE;
    

    if((command_ack&0x0000FF1F)!=(command&0x0000FF1F))
        return LTR43_ERR_CANT_LAUNCH_SEC_MARK;
    
    // hnd->Marks.SecondMarks_Qnt=0;

    return LTR43_NO_ERR;
}



/****************************************************************************************

                Функция вЫключения генерации внутренней секундной метки

****************************************************************************************/   




INT LTR43_StopSecondMark(PTLTR43 hnd)
{

    DWORD command;
    DWORD command_ack;
    DWORD slot_num   =hnd->Channel.cc -1;
    int err=0;


    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    command=0x0;
    command|=STOP_SECOND_MARK;
    command|=(slot_num<<8);

    command|=(eval_parity(command))<<5;

    if((err=(LTR_Send(&(hnd->Channel), &command, 1, TIMEOUT_CMD_SEND))) != 1 ){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }


    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }


    if(check_parity(command_ack))
        return LTR43_ERR_PARITY_FROM_MODULE;


    if((command_ack&0x0000001F)==PARITY_ERROR)
        return LTR43_ERR_PARITY_TO_MODULE;


    if((command_ack&0x0000FF1F)!=(command&0x0000FF1F))
        return LTR43_ERR_CANT_STOP_SEC_MARK;

    return LTR43_NO_ERR;
}


/****************************************************************************************

                Функция формирования метки "СТАРТ"

****************************************************************************************/   


INT LTR43_MakeStartMark(PTLTR43 hnd)
{

    DWORD command;
    DWORD command_ack;
    DWORD slot_num   =hnd->Channel.cc -1;
    int parity;
    int err=0;

    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    command=0;
    command|=MAKE_START_MARK;
    command|=(slot_num<<8);


    parity=eval_parity(command);
    command|=(parity)<<5;

    if((err=(LTR_Send(&(hnd->Channel), &command, 1, TIMEOUT_CMD_SEND))) != 1 ){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }


    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }

    if(check_parity(command_ack))
        return LTR43_ERR_PARITY_FROM_MODULE;


    if((command_ack&0x0000001F)==PARITY_ERROR)
        return LTR43_ERR_PARITY_TO_MODULE;


    if((command_ack&0x0000FF1F)!=(command&0x0000FF1F)) // Проверяем отв. команду, предварительно "убив" бит четности
        return LTR43_ERR_CANT_LAUNCH_START_MARK;


    return LTR43_NO_ERR;

}


/*****************************************************************************************************

                Функция конфигурирования интерфейса RS485

******************************************************************************************************/   

int LTR43_RS485_Config(PTLTR43 hnd)
{

    DWORD command[3];
    DWORD slot_num   =hnd->Channel.cc -1;
    DWORD command_ack;

    int BAUD_UBR; // битрейт. Число для регистра

    int err=0;


    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    if((hnd->RS485.Baud<2400))
        return LTR43_ERR_RS485_WRONG_BAUDRATE;

    if((hnd->RS485.FrameSize<5)||(hnd->RS485.FrameSize>9))
        return LTR43_ERR_RS485_WRONG_FRAME_SIZE;

    if((hnd->RS485.Parity<0)||(hnd->RS485.Parity>2))
        return LTR43_ERR_RS485_WRONG_PARITY_CONF;

    if((hnd->RS485.StopBit<0)||(hnd->RS485.StopBit>1))
        return LTR43_ERR_RS485_WRONG_STOPBIT_CONF;


    BAUD_UBR=(int) ((F_OSC/(16*hnd->RS485.Baud)-1)+0.5);

    // hnd->RS485.Baud=(int) (F_OSC/(16*(BAUD_UBR+1))+0.5);  *** Нужно ли это?!

    // Определяем битрейт

    command[0]=BAUD_UBR;
    command[0]<<=16;
    command[0]|=(slot_num<<8);
    command[0]|=RS485_CONFIG; // Первая команда содержит только битрейт
    command[0]|=(eval_parity(command[0]))<<5;

    // Два младших бита старшего байта контекста - режим проверки четности
    command[1]=hnd->RS485.Parity;

    // Бит 2 того же байта - кол-во стоп битов
    command[1]|=((hnd->RS485.StopBit)<<2);
    command[1]<<=8;

    // Младший байт контекста - кол-во бит в кадре
    if(hnd->RS485.FrameSize!=9)
        command[1]|=hnd->RS485.FrameSize-5;
    else
        command[1]|=0x7;

    command[1]<<=16;
    command[1]|=(slot_num<<8);
    command[1]|=RS485_CONFIG; //Вторая команда содержит кол-во байт в кадре, а также режим проверки четности и кол-во стоп-бит
    command[1]|=(eval_parity(command[1]))<<5;

    command[2]=(hnd->RS485.SendTimeoutMultiplier)<<24;
    command[2]|=(hnd->RS485.ReceiveTimeoutMultiplier)<<16;
    command[2]|=(slot_num<<8);
    command[2]|=RS485_CONFIG; //Третья команда содержит значение таймаутов в миллисекундах для отправки и ожидения подтверждения
    command[2]|=(eval_parity(command[2]))<<5;

    if((err=(LTR_Send(&(hnd->Channel), command, 3, TIMEOUT_CMD_SEND))) != 3 ){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }


    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }


    if(check_parity(command_ack))
        return LTR43_ERR_PARITY_FROM_MODULE;


    if((command_ack&0x0000001F)==PARITY_ERROR)
        return LTR43_ERR_PARITY_TO_MODULE;
    

    if((command_ack&0x0000FF1F)!=(command[0]&0x0000FF1F)) // Проверяем отв. команду, предварительно "убив" бит четности
        return LTR43_ERR_RS485_CANT_CONFIGURE;
    
    return LTR43_NO_ERR;
}


/****************************************************************************************************************

                  Функция передачи пакета слов (5-9 бит) по интерфейсу RS485

*****************************************************************************************************************/   



INT LTR43_RS485_Exchange(PTLTR43 hnd, SHORT *PackToSend, SHORT *ReceivedPack, INT OutPackSize, INT InPackSize)
{
    DWORD command[11];
    DWORD command_ack[20];
    DWORD slot_num   =hnd->Channel.cc -1;

    int i;
    int err=0;
    int WordsReceivedCntr=0;

    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    if(((OutPackSize<1)||(OutPackSize>10))||((OutPackSize<0)||(OutPackSize>10)))
        return LTR43_ERR_RS485_WRONG_PACK_SIZE;

    if((hnd->RS485.SendTimeoutMultiplier*OutPackSize)>255)
        return LTR43_ERR_RS485_WRONG_OUT_TIMEOUT;

    if((hnd->RS485.ReceiveTimeoutMultiplier*InPackSize)>255)
        return LTR43_ERR_RS485_WRONG_IN_TIMEOUT;


    /* В первом пакете мы должны указать, сколько байт будет содержать исходящий пакет для пересылки по RS485 */
    /* Сатрший байт контекста будет содержать кол-во слов в исходящем пакете, младший - во входящем           */

    command[0]=OutPackSize;
    command[0]<<=24;
    command[0]|=(InPackSize<<16);
    command[0]|=(slot_num<<8);
    command[0]|=RS485_SEND_BYTE;

    command[0]|=(eval_parity(command[0]))<<5;

    for(i=0; i<OutPackSize;i++)
    {
        command[i+1]=*(PackToSend+i) ;
        command[i+1]<<=16;
        command[i+1]|=(slot_num<<8);
        command[i+1]|=RS485_SEND_BYTE;
        command[i+1]|=(eval_parity(command[i+1]))<<5;
    }


    if((err=(LTR_Send(&(hnd->Channel), command, OutPackSize+1, TIMEOUT_CMD_SEND))) !=OutPackSize+1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }


    WordsReceivedCntr=LTR_Recv(&(hnd->Channel), command_ack, NULL, 2*InPackSize, TIMEOUT_CMD_RECIEVE);

    if(WordsReceivedCntr==0)
        return LTR43_ERR_MODULE_NO_RESPONCE;

    if(WordsReceivedCntr<0)
        return (WordsReceivedCntr);

    // Если мы получили меньше сэмплов, чем запрашивали, то, скорее всего, имел место тайм-аут. Обработаем эту ситуацию

    if(WordsReceivedCntr<2*InPackSize)
    {
        // Если был таймаут или ошибка четности при передаче команды, то должна придти только одна команда-ошибка

        if(WordsReceivedCntr==1)
        {
            // Если ошибка четности при приеме команды, тут же выходим
            if(check_parity(command_ack[WordsReceivedCntr-1]))
                return LTR43_ERR_PARITY_FROM_MODULE;

            // Превышен таймаут подтверждения
            if(((command_ack[WordsReceivedCntr-1])&0x1F)==RS485_ERROR_CONFIRM_TIMEOUT)
                return LTR43_ERR_RS485_CONFIRM_TIMEOUT;

            // Превыцшен таймаут отправки
            if(((command_ack[WordsReceivedCntr-1])&0x1F)==RS485_ERROR_SEND_TIMEOUT)
                return LTR43_ERR_RS485_SEND_TIMEOUT;

            if((command_ack[WordsReceivedCntr-1]&0x0000001F)==PARITY_ERROR)
                return LTR43_ERR_PARITY_TO_MODULE;
        }
        else
            return LTR43_ERR_LESS_WORDS_RECEIVED;
    } // на if(WordsReceivedCntr<...)


    for(i=0;i<InPackSize;i++)
    {

        // В старшем бите младшего байта контекста - флаг ошибки четности при приеме подтверждения
        // В бите 6 младшего байта контекста - флаг ошибки кадра при приеме подтверждения

        //command_ack[0]=0x009D82E7;

        if(check_parity(command_ack[i]))
            return LTR43_ERR_PARITY_FROM_MODULE;  // Проверка на четность входящих команд
        
        if((command_ack[i]&0x0000FF1F)!=(command[0]&0x0000FF1F))
            return LTR43_ERR_RS485_CANT_SEND_PACK;
        

        ReceivedPack[i]=(command_ack[2*i]>>16)|(((command_ack[2*i+1]>>16)&0x01)<<8);

        if((command_ack[2*i+1]>>23))
            return LTR43_ERR_RS485_PARITY_ERR_RCV;

        if((command_ack[2*i+1]>>22))
            return LTR43_ERR_RS485_FRAME_ERR_RCV;

    }

    return LTR43_NO_ERR;
}


/*********************************************************************************************************

      Тестовая функция для ожидания приема байта по интерфейсу RS485 и отправки подстверждения

**********************************************************************************************************/   

INT  LTR43_RS485_TestReceiveByte(PTLTR43 hnd, int OutPackSize, int InPackSize)
{
    DWORD command;
    DWORD command_ack;
    DWORD slot_num   =hnd->Channel.cc -1;
    int err=0;

    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    command=OutPackSize;
    command<<=24;
    command|=(InPackSize<<16);
    command|=(slot_num<<8);
    command|=RS485_RECEIVE;
    command|=(eval_parity(command))<<5;


    if((err=(LTR_Send(&(hnd->Channel), &command, 1, TIMEOUT_CMD_SEND))) != 1 ){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }


    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }
    

    if(check_parity(command_ack))
        return LTR43_ERR_PARITY_FROM_MODULE;


    if((command_ack&0x0000001F)==PARITY_ERROR)
        return LTR43_ERR_PARITY_TO_MODULE;
    

    if((command_ack&0x0000FF1F)!=(command&0x0000FF1F)) // Проверяем отв. команду, предварительно "убив" бит четности
        return LTR43_ERR_RS485_CANT_STOP_TST_RCV;


    return LTR43_NO_ERR;
}



static const int min_range_id = -4000; 
const LPCSTR err_str[] = 
{
    "LTR43:Неверный указатель на структуру описания модуля!",
    "LTR43:Невозможно открыть модуль!",
    "LTR43:Неверный серийный номер крейта!",
    "LTR43:Неверный номер посадочного места!",
    "LTR43:Невозможно отправить команду!",
    "LTR43:Невозможно выполнить аппаратный сброс модуля!",
    "LTR43:Модуль не отвечает!",
    "LTR43:Невозможно отправить данные на выходные линии!",
    "LTR43:Невозможно выполнить конфигурацию модуля!",
    "LTR43:Невозможно выполнить конфигурацию RS485!",
    "LTR43:Невозможно запустить генерацию секундных меток!",
    "LTR43:Невозможно остановить генерацию секундных меток!",
    "LTR43:Невозможно сформировать метку СТАРТ!",
    "LTR43:Невозможно остановить ожидание кадра RS-485!",
    "LTR43:Невозможно отправить слово по RS-485!",
    "LTR43:Ошибка кадра при получении подтверждения при обмене по RS-485!",
    "LTR43:Ошибка четности при получении подтверждения при обмене по RS-485!",
    "LTR43:Неверная конфигураци групп ввода-вывода!",
    "LTR43:Неверная скорость обмена по RS-485!",
    "LTR43:Неверный размер кадра RS-485!",
    "LTR43:Неверная конфигурация четности для кадра RS-485!",
    "LTR43:Неверная конфигурация СТОП-бита для кадра RS-485!",
    "LTR43:Ошибка при передаче данных модулю!",
    "LTR43:Таймаут получения подтверждения при обмене по RS-485 истек!",
    "LTR43:Таймаут отправки по RS-485 истек!",
    "LTR43:Получено меньше слов, чем было заказано!",
    "LTR43:Ошибка четности при передаче команды модулю!",
    "LTR43:Ошибка четности при получении команды от модуля!",	  //Посмотри, что у нас с СТАРТ и Сек! (0,1,2)?
    "LTR43:Неверная конфигурация линий ввода-вывода! Только значения 1 и 0 допустимы в IO_Groups.Group(n)!",
    "LTR43:Неверная конфигурация секундных меток! Только значения 0, 1 и 2 допустимы в Marks.SecondMark_Mode!",
    "LTR43:Неверная конфигурация метки СТАРТ! Только значения 0, 1 и 2 допустимы в Marks.StartMark_Mode!",
    "LTR43:Невозможно прочитать данные!",
    "LTR43:Невозможно отправить пакет по RS-485",
    "LTR43:Невозможно выполнить конфигурацию RS-485",
    "LTR43:Невозможно выполнить запись в  EEPROM!",
    "LTR43:Невозможно выполнить чтение из EEPROM!",
    "LTR43:Неверный адрес EEPROM!",
    "LTR43:RS-485 Неверный размер исходящего или входящего пакета!",
    "LTR43:RS-485 Неверный таймаут для исходящего пакета!",
    "LTR43:RS-485 Неверный таймаут для пакета-подтверждения!",
    "LTR43:Невозможно считать идентификационную запись!",
    "LTR43:Неверная идентификационная запись!",
    "LTR43:RS-485 Невозможно остановить прием при тестировании интерфейса!",
    "LTR43:Невозможно запустить потоковый ввод данных!",
    "LTR43:Невозможно остановить потоковый ввод данных!",
    "LTR43:Неверные данные с портов ввода-вывода!",
    "LTR43:Неверные установки часоты выдачи данных при потоковом вводе!",
    "LTR43:Переполнение входного буфера крейт-контроллера!"
};

/*****************************************************************************************************

                Функция определения строки ошибки

******************************************************************************************************/   

LPCSTR LTR43_GetErrorString(int Error_Code)
{
    LPCSTR ret_val = NULL;
    int ret_val_index;
    double error_range;

    error_range=-min_range_id + 1+sizeof err_str / sizeof err_str[0];

    if ((-Error_Code < -min_range_id )|| (Error_Code < -error_range))
    {
        ret_val = LTR_GetErrorString(Error_Code);
    }
    else
    {
        ret_val_index=-Error_Code + min_range_id-1;
        ret_val = err_str[ret_val_index];
    }

    return ret_val;
}

/*****************************************************************************************************

                Функция остановки ожидания приема при тестировании RS-485

******************************************************************************************************/   

INT LTR43_RS485_TestStopReceive(PTLTR43 hnd)
{
    DWORD command;
    DWORD command_ack;
    DWORD slot_num   =hnd->Channel.cc -1;

    int err=0;

    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;

    command=0;
    command|=(slot_num<<8);
    command|=RS485_STOP_RECEIVE;

    if((err=(LTR_Send(&(hnd->Channel), &command, 1, TIMEOUT_CMD_SEND)))!= 1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }

    
    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_MODULE_NO_RESPONCE;
    }
    

    if((command_ack&0x0000FFFF)!=(command&0x0000FFFF))
        return LTR43_ERR_CANT_STOP_RS485RCV;


    return LTR43_NO_ERR;
}


/**********************************************************************************************

        Функция записи байта в указанную ячейку ППЗУ

 ************************************************************************************************/

INT LTR43_WriteEEPROM(PTLTR43 hnd, INT Address, BYTE val)
{

    DWORD command[2], command_ack;
    INT slot_num     =hnd->Channel.cc -1;
    INT err=0;
    INT parity=0;
    
    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;


    if((Address<0)||(Address)>=512)
        return LTR43_ERR_WRONG_EEPROM_ADDR;

    command[0]=Address; // Добавляем адрес ячейки
    command[0]<<=16;
    command[0]|=WRITE_EEPROM|(slot_num<<8);  // Формируем команду для DSP

    parity=eval_parity(command[0]);
    command[0]|=(parity)<<5;
    

    command[1]=val; // Данные для записи
    command[1]<<=16;
    command[1]|=WRITE_EEPROM|(slot_num<<8);

    parity=eval_parity(command[1]);
    command[1]|=(parity)<<5;

    if((err=(LTR_Send(&(hnd->Channel), command, 2, TIMEOUT_CMD_SEND)))!=2){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }


    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_WRITE_EEPROM;
    }
    

    // Проверка на четность пришедшего пакета

    if(check_parity(command_ack))
        return LTR43_ERR_PARITY_FROM_MODULE;


    if((command_ack&0x0000001F)==PARITY_ERROR)
        return LTR43_ERR_PARITY_TO_MODULE;
    

    if((command_ack&0x0000FF1F)!=(command[0]&0x0000FF1F)) // Проверяем отв. команду, предварительно "убив" бит четности
        return LTR43_ERR_CANT_WRITE_EEPROM;

    return LTR43_NO_ERR;
}


/**********************************************************************************************

        Функция чтения байта из указанной ячейки ППЗУ

 ************************************************************************************************/


INT LTR43_ReadEEPROM(PTLTR43 hnd, INT Address, BYTE *val)
{

    DWORD command, command_ack;
    INT slot_num     =hnd->Channel.cc -1;
    INT err=0;
    INT parity=0;
    
    
    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;


    if((Address<0)||(Address)>=512)
        return LTR43_ERR_CANT_READ_EEPROM;

    command=Address; // Добавляем адрес ячейки
    command<<=16;
    command|=READ_EEPROM|(slot_num<<8);  // Формируем команду для DSP

    parity=eval_parity(command);
    command|=(parity)<<5;
    
    
    if((err=(LTR_Send(&(hnd->Channel), &command, 1, TIMEOUT_CMD_SEND)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }


    if((err=(LTR_Recv(&(hnd->Channel), &command_ack, NULL, 1, TIMEOUT_CMD_RECIEVE)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_READ_EEPROM;
    }

    // Проверка на четность пришедшего пакета

    if(check_parity(command_ack))
        return LTR43_ERR_PARITY_FROM_MODULE;


    if((command_ack&0x0000001F)==PARITY_ERROR)
        return LTR43_ERR_PARITY_TO_MODULE;
    

    if((command_ack&0x0000FF1F)!=(command&0x0000FF1F)) // Проверяем отв. команду, предварительно "убив" бит четности
        return LTR43_ERR_CANT_READ_EEPROM;

    *val=(command_ack>>16)&0xFF;

    return LTR43_NO_ERR;
}

/*****************************************************************************************************

                Функция чтения конфигурационной записи модуля модуля

******************************************************************************************************/ 

int LTR43_ReadConfRecord(PTLTR43 hnd, CHAR *version, CHAR *date, CHAR *name, CHAR *serial)
{

#define CR_LENGTH (42)
#define CRC_LENGTH (2)

    BYTE conf_rec[44];
    BYTE ver[2];
    DWORD command, command_ack[50];
    INT slot_num     =hnd->Channel.cc -1;
    INT err=0;
    INT parity=0;
    WORD CR_CRC;      // Контр. сумма конф. записи, посчитанная тут
    WORD CR_CRC_RCV;  // Контр. сумма конф. записи, считанная из Flash
    int i;
    
    
    if(hnd==NULL)
        return LTR43_ERR_WRONG_MODULE_DESCR;


    command=0; // Добавляем адрес ячейки
    command|=READ_CONF_RECORD|(slot_num<<8);  // Формируем команду для DSP

    parity=eval_parity(command);
    command|=(parity)<<5;


    if((err=(LTR_Send(&(hnd->Channel), &command, 1, TIMEOUT_CMD_SEND)))!=1){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_SEND_COMMAND;
    }


    if((err=(LTR_Recv(&(hnd->Channel), command_ack, NULL, CR_LENGTH+CRC_LENGTH, TIMEOUT_CMD_RECIEVE)))!=(CR_LENGTH+CRC_LENGTH)){
        if(err<0)
            return err;
        else
            return LTR43_ERR_CANT_READ_CONF_REC;
    }


    for(i=0;i<CR_LENGTH;i++)
    {

        if((command_ack[i]&0x0000001F)==PARITY_ERROR)
            return LTR43_ERR_PARITY_TO_MODULE;


        // В старшем бите младшего байта контекста - флаг ошибки четности при приеме подтверждения
        // В бите 6 младшего байта контекста - флаг ошибки кадра при приеме подтверждения

        if(check_parity(command_ack[i]))
            return LTR43_ERR_PARITY_FROM_MODULE;  // Проверка на четность входящих команд
        
        if((command_ack[i]&0x0000FF1F)!=(command&0x0000FF1F))
            return LTR43_ERR_CANT_READ_CONF_REC;
    }


    for(i=0;i<CR_LENGTH;i++)
        conf_rec[i]=(command_ack[i]>>16)&0xFF;
    
    CR_CRC=0;
    
    CR_CRC=CRC_Calc(conf_rec, CR_LENGTH);
    CR_CRC_RCV=(((command_ack[CR_LENGTH]>>16)&0xFFF)|((command_ack[CR_LENGTH+1]>>8)&0xFF00));
    
    
    //Вот тут! Сравнить с записываемой контрольной суммой!
    
    if(CR_CRC!=CR_CRC_RCV)
        return LTR43_ERR_WRONG_CONF_REC;


    if(conf_rec[0]!=MODULE_ID)
        return LTR43_ERR_WRONG_CONF_REC;
    
    
    for(i=0;i<2;i++)
        ver[i]=*(conf_rec+1+i); // Версия БИОСа

    sprintf(version, "%d.%d",ver[0], ver[1]);

    for(i=0;i<14;i++)
        date[i]=*(conf_rec+3+i); // Дата БИОСа

    for(i=0;i<8;i++)
        name[i]=*(conf_rec+17+i); // Имя модуля

    for(i=0;i<17;i++)
        serial[i]=*(conf_rec+25+i); // Серийный номер



    
    return LTR43_NO_ERR;
}



/******************************************************************************************

                     Функция расчета делителя и пределителя таймера при потоковом вводе

*****************************************************************************************/
INT eval_scalers(double *Freq, WORD *Prescaler, BYTE *PrescalerCode, BYTE *Scaler)
{

    if((*Freq>=100.0)&&(*Freq<230.0))
    {
        *Prescaler=1024;
        *PrescalerCode=4;
    }

    if((*Freq>=230.0)&&(*Freq<920.0))
    {
        *Prescaler=256;
        *PrescalerCode=3;
    }

    if((*Freq>=920.0)&&(*Freq<7325.0))
    {
        *Prescaler=64;
        *PrescalerCode=2;
    }

    if((*Freq>=7325.0)&&(*Freq<58595.0))
    {
        *Prescaler=8;
        *PrescalerCode=1;
    }

    if((*Freq>=58595.0)&&(*Freq<=100000.0))
    {
        *Prescaler=1;
        *PrescalerCode=0;
    }


    *Scaler=(BYTE) (((F_OSC/((*Prescaler)*(*Freq)))-1)+0.5);

    *Freq=F_OSC/((*Prescaler)*(*Scaler+1));

    return(0);

}


/*******************************************************************************

                     Расчет четности уходящих команд

*******************************************************************************/

INT eval_parity(DWORD cmd)
{
    unsigned long index;
    INT           parity;


    parity = 0;
    for (index = 0x01UL; index <= 0x10UL; index <<= 1)
        parity ^= (cmd & index) != 0;

    // Проверку четности выполняем только для байт контекста команды
    for (index = 0x80000000UL; index >= 0x00010000UL; index >>= 1)
        parity ^= (cmd & index) != 0;


    return parity;
}


/*******************************************************************************

                   Расчет четности входящих команд

*******************************************************************************/

INT eval_parity_in(DWORD cmd)
{
    unsigned long index;
    INT           parity;


    parity = 0;
    for (index = 0x01UL; index <= 0x10UL; index <<= 1)
        parity ^= (cmd & index) != 0;

    // Проверку четности выполняем только для байт контекста команды
    for (index = 0x800000UL; index >= 0x010000UL; index >>= 1)
        parity ^= (cmd & index) != 0;


    return parity;
}


INT check_parity(DWORD cmd)
{
    int tmp1, tmp2;

    tmp1=(cmd&0x20)>>5;
    tmp2=eval_parity_in(cmd);

    if(((cmd&0x20)>>5)==eval_parity_in(cmd))
        return(0);
    else
        return(1);
}



int HC_LE41_CONNECT(PTLTR43 hnd)
/*Подключиться к единственному модулю.
 В отличие от HC_LE41_SELECT, переводит в активное состояние все модули, подключенные к COM-порту,
 поэтому, должна применяться только тогда, когда подключен один модуль.
Результат:
  Код ошибки. 0 - успешное выполнение.
*/
{
    BYTE outbuf[3];
    SHORT PackToSend[5];
    SHORT BytesToReceive[3];
    WORD CRC;
    int err;

    outbuf[0]= 0xB0;
    outbuf[1]= 0xB5;
    outbuf[2]= 1;

    CRC=0;

    CRC=CRC_Calc(outbuf, 3);

    PackToSend[0]=(WORD) outbuf[0];
    PackToSend[1]=(WORD) outbuf[1];
    PackToSend[2]=(WORD) outbuf[2];
    PackToSend[3]=CRC&0xFF;
    PackToSend[4]=CRC>>8;


    err=LTR43_RS485_Exchange(hnd, PackToSend, BytesToReceive, 5, 3);


    outbuf[0]= 5;
    outbuf[1]= 0;
    outbuf[2]= 0;

    CRC=0;

    CRC=CRC_Calc(outbuf, 3);

    PackToSend[0]=(WORD) outbuf[0];
    PackToSend[1]=(WORD) outbuf[1];
    PackToSend[2]=(WORD) outbuf[2];
    PackToSend[3]=CRC&0xFF;
    PackToSend[4]=CRC>>8;


    err=LTR43_RS485_Exchange(hnd, PackToSend, BytesToReceive, 5, 3);


    return LTR43_NO_ERR;
}


/*******************************************************************************

  Расчет 16-битной CRC
  
*******************************************************************************/


static WORD CRC_Calc(BYTE *b, WORD N)
/*Вычисление контрольной суммы.
Вход:
  b -- массив байтов.
  N -- длина массива.
Результат:
  Значение контрольной суммы.
*/
{
    WORD crc= 0;
    int i, j;

    for(i=0; i<N; i++) {
        crc ^= b[i] << 8;
        for(j=0;j<8;j++) {
            if(crc&0x8000) crc = (crc<<1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc;
}

static void CRC_Update(WORD *crc, BYTE *b, int c)
/*Обновление контрольной суммы с учётом данных, не использованных
 при вычислении предыдущего значения.
Вход:
  crc -- указатель на старое значение контрольной суммы.
  b -- массив байтов.
  c -- длина массива.
Выход:
  crc -- указатель на новое значение контрольной суммы.
*/
{
    unsigned w;
    int i,j;

    w = *crc;
    for(i=0;i<c;i++) {
        w ^= b[i] << 8;
        for(j=0;j<8;j++)  {
            if(w&0x8000) w = (w<<1) ^ 0x1021;
            else w <<= 1;
        }
    }
    *crc = w;
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

#ifdef _MANAGED
#pragma managed(pop)
#endif

