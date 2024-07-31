#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_eeprom.h>
#include "MDR1986VE1T.h"  
#include "MDR32F9Qx_eth.h"
#include "MDR32F9Qx_can.h"              // Keil::Drivers:CAN
#include "init.h"
#include "1636RR52.h"
#include "ethcan.h"

//°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•
//°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•° РАЗЛИЧНЫЕ ПЕРЕМЕННЫЕ ДЛЯ РАБОТЫ °•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•
//°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•

//#defines:
#define LED1            PORT_Pin_7
#define LED2            PORT_Pin_8
#define LED3            PORT_Pin_9
#define LED4            PORT_Pin_10
#define LED5            PORT_Pin_11
#define LED6            PORT_Pin_12
#define LED7            PORT_Pin_13
#define LED8            PORT_Pin_14
#define LED_ALL			PORT_Pin_7 | PORT_Pin_8  | PORT_Pin_9  | PORT_Pin_10 | PORT_Pin_11 																															\
						           | PORT_Pin_12 | PORT_Pin_13 | PORT_Pin_14 //все светодиоды, без лишних портов
#define FR_MAC_SIZE 		12												//	Длина МАС полей в заголовке
#define FR_L_SIZE   		2												//  Длина поля Lenth/Eth Type
#define FR_HEAD_SIZE   	(FR_MAC_SIZE + FR_L_SIZE) // 	Длина Заголовка
// Массивы для считывания входящих и формирования посылаемых фреймов, 
// Должны располагаться в адресах начиная с 0х2010_0000 для возможности работы DMA в режиме FIFO
#define  MAX_ETH_TX_DATA_SIZE 1514 / 4
#define  MAX_ETH_RX_DATA_SIZE 1514 / 4

//variables:
uint8_t  FrameTx[MAX_ETH_TX_DATA_SIZE] __attribute__((section("EXECUTABLE_MEMORY_SECTION"))) __attribute__ ((aligned (4)));
uint32_t FrameRx[MAX_ETH_RX_DATA_SIZE] __attribute__((section("EXECUTABLE_MEMORY_SECTION"))) __attribute__ ((aligned (4)));
uint8_t  PaketSost[MAX_ETH_TX_DATA_SIZE] __attribute__((section("EXECUTABLE_MEMORY_SECTION"))) __attribute__ ((aligned (4)));
uint8_t buf_eth[70], output_eth_buf[70];

uint8_t mac1; 			//Считывание перемычек 1 канал
uint8_t mac2;				//Считывание перемычек 1 канал
uint8_t mac3;				//Считывание перемычек 2 канал
uint8_t mac4;				//Считывание перемычек 2 канал
uint8_t mac_1k;			//5ый байт записи в MAC-адрес 1канал
uint8_t mac_2k;			//5ый байт записи в MAC-адрес 2канал
uint8_t error=0xFF;	//Несовпадение MAC-адресов (1к и 2к)
uint8_t N_BI;				//Номер канала Блока Информации
CAN_TxMsgTypeDef TxMsg;	 //struct can

uint8_t TX_eth_pam[70];
uint8_t RX_eth_pam[70];

//func:
//	Обработка входящего фрейма и высылка ответа
void ETH_TaskProcess(MDR_ETHERNET_TypeDef * ETHERNETx);
//	Заполнение массива FrameTx фреймом заданной длины
void Ethernet_FillFrameTX(uint32_t frameL);
void Ethernet_PC (uint32_t mcpp);
__IO uint32_t tx_buf = 0;
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
__IO uint32_t ret = 0; //для возврата обработки прерываний 
__IO uint32_t rx_buf = 0;
__IO uint32_t rtr_request_buf = 1;
__IO uint32_t rtr_reply_buf = 2;


//°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•
//°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•° БЛОК РАБОТЫ С Ethernet •°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•
//°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•
//----------------------------------------
// Считываем MAC-адрес ПП 
void MAC_ID(void)
{
	//ВАЖНО!!! ПИН 5 и 13 не считываются из-за смещения!!! Если зададут на них перемычки, то необходимо использовать побитное считывание!!!
		PORT_ReadInputData(MDR_PORTB);
		mac1 = ~(PORT_ReadInputData(MDR_PORTB))<<4;				//pin 0-3 read 
		mac2 = ~(PORT_ReadInputData(MDR_PORTB)>>4)<<4;		//pin 4-7, по сути не читаем его, смысла нет, максимум бит паритета если надо
		mac3 = ~(PORT_ReadInputData(MDR_PORTB)>>7)<<4;		//pin 7-10 read 
		mac4 = ~(PORT_ReadInputData(MDR_PORTB)>>12)<<4;		//pin 12-15, 15 пин - определение канала, если к +3.3 подтянут то первый, если нет, то второй
		mac1 = mac1>>4;
		mac2 = mac2>>4;
		mac3 = mac3>>4;
		mac4 = mac4>>4;
		
		if ((MDR_PORTB->RXTX & PORT_Pin_15) != RESET)
			{
				N_BI	= 0x01;
			}
			else
			{
				N_BI	=	0x02;
			}
		
			if (mac1==mac3)
			mac_1k=mac1;
		else mac_1k=error;
		
			MAC_SRC [4] = mac_1k;
			MAC_SRC	[5]	= 1;
					
		/*	switch (N_BI)
		{			
			case 0x01:
			{
				MAC_SRC [4] = mac_1k;
				MAC_SRC	[5]	= N_BI;
			}
			case 0x02:
			{
				MAC_SRC [4] = mac_1k;
				MAC_SRC	[5]	= N_BI;
			}
		}*/
}
uint8_t  MAC_SRC [] = {0x11, 0x22, 0x33, 0x44, 0xFF, 0xFF};
//----------------------------------------
//  Работа с ПУ
void Ethernet_Start(void)
{
	// Запуск блока Ethernet
	ETH_Start(MDR_ETHERNET1);
}


//----------------------------------------
//	Цикл обработки входящих фреймов
void Ethernet_ProcessLoop(void)
{
	while(1)
	{
		//проверка на наличие посылки eth
		ETH_TaskProcess(MDR_ETHERNET1);
	}
}


//----------------------------------------
//	Обработка входящего фрейма и отправка ответа
void ETH_TaskProcess(MDR_ETHERNET_TypeDef * ETHERNETx)
{
	//	Поле состояния приема пакета
	volatile ETH_StatusPacketReceptionTypeDef ETH_StatusPacketReceptionStruct;
	
	//	Указатель для работы с входными данными
	uint8_t * ptr_inpFrame = (uint8_t *) &FrameRx[0];
	//	Входные параметры от PC
	uint16_t frameL = 70;		//Длина ответного фрейма
	uint16_t frameCount = 1;	//кол-во ответных фреймов 
	uint16_t mcpp = 70;
	uint32_t sector_pam =	0x00036;
	uint8_t out_while = 0x00;
	uint32_t memory = 0;
	//	Внутренние переменные
	uint32_t i;
	volatile uint32_t isTxBuffBusy = 0;
	volatile uint32_t isRxBuffBusy = 0;
	
		//	Проверяем наличие в буфере приемника данных для считывания
	if(ETHERNETx->ETH_R_Head != ETHERNETx->ETH_R_Tail)
	{
		// Считывание входного фрейма
	
		ETH_StatusPacketReceptionStruct.Status = ETH_ReceivedFrame(ETHERNETx, FrameRx);
		
		//-----------------ЗАПОЛНЯЕМ МАССИВЫ ДЛЯ ДАЛЬНЕЙШЕЙ ОБРАБОТКИ-----------------
		//считывание требуемого действия от ПП
		//buf_eth[] заполняем для дальнейшего анализа в работе CAN 
		for(int i = 0; i < 70; i++){
			buf_eth[i] = (uint8_t)(ptr_inpFrame[i]); //задаём считываемый байт.
		}
		//-----------------------------------------------------------------------------
		// 	Заполнение массива FrameTx пакетом отправки
		Ethernet_FillFrameTX(frameL);
		
		//	Посылка пакетов в цикле
		for (i = 0; i < frameCount; ++i)
		{
			//  Ожидаем опустошения буфера передатчика наполовину
			//  Размер буфера приемника задан Delimeter и равен 4Кбайт
			//  Максимальная длина пакета может составлять 1518 байт (в данном исполнении используется 70 байт)
			//	Выбор условия опустошения на половину дает бОльшую надежность при интенсивном обмене большими пакетами.			
			do 
			{
				isTxBuffBusy = ETH_GetFlagStatus(ETHERNETx, ETH_MAC_FLAG_X_HALF) == SET;				
			}	
			while (isTxBuffBusy);
			//  Посылка пакета (переслали принятый пакет, отправитель поймёт что мы получили запрос и обрабатываем его)
			ETH_SendFrame(ETHERNETx, (uint32_t *) FrameTx, frameL);
			//Сохранение ПУ (принятого пакета)
			for(i=0; i<70; i++)
			{
				RX_eth_pam[i] = ptr_inpFrame[i];
			}
			do 
			{
				isRxBuffBusy = ETH_GetFlagStatus(ETHERNETx, ETH_MAC_FLAG_X_EMPTY) == SET;				
			}	
			while (isTxBuffBusy);
			//Передача полученного запроса по CAN
			
			CAN_Transmition();
			for(uint32_t i_c=0; i_c <2000000; i_c++){}
			otpravka_eth();
			//сохранение ПС (отправленного)
			//ETH_transmite_save(); 
			
		}
	}
}
//----------------------------------------
//	Заполнение ответного фрейма (который просто переписывает входящий)
void Ethernet_FillFrameTX(uint32_t frameL)
	{
		uint8_t RX_eth_pam[70];
	
		uint32_t	i = 0;
		uint8_t		j	=	0;
		//	Определение колличества данных в Payload
		uint32_t payloadL = frameL - FR_HEAD_SIZE;
		//	Указатель на входящий фрейм для копирования SrcMAC
		uint8_t * ptr_inpFrame = (uint8_t *) &FrameRx[0];
		//	Указатель на заполняемый фрейм, 
		//  оставлено место "Поле управления передачей пакета"
		uint8_t * ptr_TXFrame  = (uint8_t *) &FrameTx[4];

		//	Запись "Поле управления передачей пакета"
		//  Указывается длина фрейма
		*(uint32_t *)&FrameTx[0] = frameL;
		
		//	Копируем адрес PC из входного пакета в DestMAC
		ptr_TXFrame[0] 	= ptr_inpFrame[6];
		ptr_TXFrame[1] 	= ptr_inpFrame[7];
		ptr_TXFrame[2] 	= ptr_inpFrame[8];
		ptr_TXFrame[3] 	= ptr_inpFrame[9];
		ptr_TXFrame[4] 	= ptr_inpFrame[10];
		ptr_TXFrame[5] 	= ptr_inpFrame[11];		
		
		//	Заполняем SrcMAC
		ptr_TXFrame[6] 	= MAC_SRC[0];
		ptr_TXFrame[7] 	= MAC_SRC[1];
		ptr_TXFrame[8] 	= MAC_SRC[2];
		ptr_TXFrame[9] 	= MAC_SRC[3];
		ptr_TXFrame[10] = MAC_SRC[4];
		ptr_TXFrame[11] = MAC_SRC[5];	
		
		//Поле L/T = длина МУПП
		ptr_TXFrame[12] = ptr_inpFrame[12];		
		ptr_TXFrame[13] = ptr_inpFrame[13];			
		//	заполняем DATA 
		for(int k = 14; k < 66; k++)
		{
			ptr_TXFrame[k] = ptr_inpFrame[k];
		}			

	}	
//----------------------------------------
// Заполнение ответного фрейма
void Ethernet_PC (uint32_t mcpp)
	{

		uint32_t	i = 0;
		uint8_t		j	=	0;
		uint8_t TX_eth_pam[70];
		//	Определение колличества данных в Payload
		uint32_t payloadL = mcpp - FR_HEAD_SIZE;
		//	Указатель на входящий фрейм для копирования SrcMAC
		uint8_t * ptr_inpFrame = (uint8_t *) &FrameRx[0];
		//	Указатель на заполняемый фрейм, 
		//  оставлено место "Поле управления передачей пакета"
		uint8_t * ptr_TXFrame  = (uint8_t *) &PaketSost[4];

		//	Запись "Поле управления передачей пакета"
		//  Указывается длина фрейма
		*(uint32_t *)&PaketSost[0] = mcpp;
		
		//	Копируем адрес PC из входного пакета в DestMAC
		ptr_TXFrame[0] 	= ptr_inpFrame[6];
		ptr_TXFrame[1] 	= ptr_inpFrame[7];
		ptr_TXFrame[2] 	= ptr_inpFrame[8];
		ptr_TXFrame[3] 	= ptr_inpFrame[9];
		ptr_TXFrame[4] 	= ptr_inpFrame[10];
		ptr_TXFrame[5] 	= ptr_inpFrame[11];		
		
		//	Заполняем SrcMAC
		ptr_TXFrame[6] 	= MAC_SRC[0];
		ptr_TXFrame[7] 	= MAC_SRC[1];
		ptr_TXFrame[8] 	= MAC_SRC[2];
		ptr_TXFrame[9] 	= MAC_SRC[3];
		ptr_TXFrame[10] = MAC_SRC[4];
		ptr_TXFrame[11] = MAC_SRC[5];	
		
		//Поле L/T
		ptr_TXFrame[12] = 0x00;
		ptr_TXFrame[13] = 0x36; 
		
		//заполнение данных
		for(uint8_t i = 14; i < 70; i++) //было <18, без выравниваний, изм.22/07/2024
		{
			ptr_TXFrame[i] = output_eth_buf[i];
	
		}

			for(i=0; i<100; i++)
			{
				TX_eth_pam[i] = ptr_TXFrame[i];
			}

	}	
//----------------------------------------
	// Отправка МСПП
void otpravka_eth()
{
	int i;
	volatile uint32_t isTxBuffBusy = 0;
		 
		uint16_t mcpp = 70; 

	//	Посылка пакетов в цикле
		Ethernet_PC (mcpp);
	
	
		for (i = 0; i < 2; ++i)
		{
			do {
				isTxBuffBusy = ETH_GetFlagStatus(MDR_ETHERNET1, ETH_MAC_FLAG_X_HALF) == SET;
					for(int i = 0; i < 8500; i++){}
			}	
			while (isTxBuffBusy);
			ETH_SendFrame(MDR_ETHERNET1, (uint32_t *) PaketSost, mcpp);
		}
}

//°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•
//°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•° БЛОК РАБОТЫ С CAN °•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•
//°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•°•
void speed_can() 														 
{
	CAN_InitTypeDef  speed_CAN;
	CAN_StructInit (&speed_CAN);
   	speed_CAN.CAN_ROP  = DISABLE;           	// Контроллер принимает только чужие пакеты
   	speed_CAN.CAN_SAP  = DISABLE;				// контроллер подтверждает прием только чужих пакетов
   	speed_CAN.CAN_STM  = DISABLE;						 // контроллер работает в нормальном режиме
   	speed_CAN.CAN_ROM  = DISABLE;						 // контроллер работает в нормальном режиме
	speed_CAN.CAN_PSEG = CAN_PSEG_Mul_1TQ; 	 // задал скорость 1мбит
   	speed_CAN.CAN_SEG1 = CAN_SEG1_Mul_3TQ; 	 // задал скорость 1мбит
   	speed_CAN.CAN_SEG2 = CAN_SEG2_Mul_3TQ; 	 // задал скорость 1мбит
	speed_CAN.CAN_SJW  = CAN_SJW_Mul_1TQ;	 	 // задал скорость 1мбит
    speed_CAN.CAN_SB   = CAN_SB_3_SAMPLE; 	 // задал скорость 1мбит
    speed_CAN.CAN_BRP  = 1;									 // Делитель частоты
    CAN_Init (MDR_CAN2,&speed_CAN);
	
	int i = 0;	
		for(i=0; i<32; i++) //отключаем буферы
		{
			MDR_CAN2->BUF_CON[i] = 0x00000000;
		}
		for(i=0;i<32;i++) //чистим буферы
		{
			MDR_CAN2->CAN_BUF[i].ID = 0x00000000;
			MDR_CAN2->CAN_BUF[i].DLC = 0x00000000;
			MDR_CAN2->CAN_BUF[i].DATAH = 0x00000000;
			MDR_CAN2->CAN_BUF[i].DATAL = 0x00000000;
		}
		//filters id ставим фильтры
		//--------------------------------------
		CAN_FilterInitTypeDef CAN_Filter_1;
		CAN_Filter_1.Filter_ID = 0x00000001;
		CAN_Filter_1.Mask_ID = 0x000000FF;
		//--------------------------------------
		CAN_FilterInitTypeDef CAN_Filter_2;
		CAN_Filter_2.Filter_ID = 0x00000002;
		CAN_Filter_2.Mask_ID = 0x000000FF;
		//--------------------------------------
		CAN_FilterInitTypeDef CAN_Filter_3;
		CAN_Filter_3.Filter_ID = 0x00000003;
		CAN_Filter_3.Mask_ID = 0x000000FF;
		//--------------------------------------
		CAN_FilterInitTypeDef CAN_Filter_4;
		CAN_Filter_4.Filter_ID = 0x00000004;
		CAN_Filter_4.Mask_ID = 0x000000FF;
		//--------------------------------------
		CAN_FilterInitTypeDef CAN_Filter_5;
		CAN_Filter_5.Filter_ID = 0x00000005;
		CAN_Filter_5.Mask_ID = 0x000000FF;
		//--------------------------------------
		CAN_FilterInitTypeDef CAN_Filter_6;
		CAN_Filter_6.Filter_ID = 0x00000006;
		CAN_Filter_6.Mask_ID = 0x000000FF;
		//--------------------------------------
		CAN_FilterInitTypeDef CAN_Filter_7;
		CAN_Filter_7.Filter_ID = 0x00000007;
		CAN_Filter_7.Mask_ID = 0x000000FF;
		//--------------------------------------
		CAN_FilterInitTypeDef CAN_Filter_8;
		CAN_Filter_8.Filter_ID = 0x00000008;
		CAN_Filter_8.Mask_ID = 0x000000FF;
		//--------------------------------------
		CAN_FilterInitTypeDef CAN_FilterREZERV;
		CAN_FilterREZERV.Filter_ID = 0x00000099;
		CAN_FilterREZERV.Mask_ID = 0x000000FF;
			
		
		//присваеваем фильтры буферам
		for (i=6; i<8; i++)
			{
				CAN_FilterInit(MDR_CAN2, i, &CAN_Filter_1);
			}
		for (i=8; i<10; i++)
			{
				CAN_FilterInit(MDR_CAN2, i, &CAN_Filter_2);
			}
		for (i=10; i<12; i++)
			{
				CAN_FilterInit(MDR_CAN2, i, &CAN_Filter_3);
			}
		for (i=12; i<14; i++)
			{
				CAN_FilterInit(MDR_CAN2, i, &CAN_Filter_4);
			}
		for (i=14; i<16; i++)
			{
				CAN_FilterInit(MDR_CAN2, i, &CAN_Filter_5);
			}
		for (i=16; i<18; i++)
			{
				CAN_FilterInit(MDR_CAN2, i, &CAN_Filter_6);
			}
		for (i=18; i<20; i++)
			{
				CAN_FilterInit(MDR_CAN2, i, &CAN_Filter_7);
			}
		for (i=20; i<22; i++)
			{
				CAN_FilterInit(MDR_CAN2, i, &CAN_Filter_8);
			}
		for (i=22; i<30; i++)
			{
				CAN_FilterInit(MDR_CAN2, i, &CAN_FilterREZERV);
			}
		for (i=1; i<6; i++)
		{
			MDR_CAN2->BUF_CON[i] = 0x01;		//буффер на отправку		
					
		}	
		for (i=30; i<32; i++)
		{
			MDR_CAN2->BUF_CON[i] = 0x01;		//буффер на отправку (резервный)			 
		}		
		for (i=6; i<30; i++)
		{
			MDR_CAN2->BUF_CON[i] = 0x03;		//буффер на приём и перезапись			 
		}	
		CAN_Cmd(MDR_CAN2, ENABLE);
}

void CAN_Transmition(void){
	uint8_t buf = 2;
	uint8_t id = 0x01;
	Buf_Clear();
	for(int delay = 0; delay < 100000; delay++){}
	for (int i = 0; i < 63; i = i + 8)
	{
		if(buf == 6)
		{
			Buf_Clear();
			buf = 2;
			for(int delay = 0; delay < 100000; delay++){} //задержка для очиски буферов, без неё апаратно не успеют очиститься буферы
		}
		TxMsg.ID      = id;					//идентификатор  
		TxMsg.DLC     = 0x08;								//количество байт данных
		TxMsg.PRIOR_0 = DISABLE;						//приоритет 
		TxMsg.IDE     = CAN_ID_EXT;					//задаём стандартный идентификатор, EXT(расширенный), STD(стандартный)
		TxMsg.Data[1]	= (buf_eth[7]<<24)+(buf_eth[6]<<16)+(buf_eth[5]<<8)+(buf_eth[4]);						//заполнение байта данных (data hight)
		TxMsg.Data[0] = (buf_eth[3]<<24)+(buf_eth[2]<<16)+(buf_eth[1]<<8)+(buf_eth[1]);		//заполнение байта данных (data low)
		CAN_BufferRelease(MDR_CAN2, buf);			//выдача буфера
		CAN_Transmit(MDR_CAN2, buf, &TxMsg);  // отправка сообщения
		buf++;
		id++;
	}
}

//----------------------------------------
/*Очистка буферов*/
void Buf_Clear(void)
{
	uint8_t i = 0;
	for(i = 6; i < 30; i++) //чистим буферы
		{
			MDR_CAN2->CAN_BUF[i].ID = 0x00000000;
			MDR_CAN2->CAN_BUF[i].DLC = 0x00000000;
			MDR_CAN2->CAN_BUF[i].DATAH = 0x00000000;
			MDR_CAN2->CAN_BUF[i].DATAL = 0x00000000;
			MDR_CAN2->BUF_CON[i] = 0x03;
			//buf_status[i] = 0x00;
		}
		

}
//----------------------------------------








