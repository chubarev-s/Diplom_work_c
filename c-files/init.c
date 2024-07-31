#include "MDR1986VE1T.h"                // Keil::Device:Startup
#include "MDR32F9Qx_rst_clk.h"          // Keil::Drivers:RST_CLK
#include "MDR32F9Qx_can.h"              // Keil::Drivers:CAN
#include "system_MDR1986VE1T.h"         // Keil::Device:Startup
#include "MDR32F9Qx_port.h"             // Keil::Drivers:PORT
#include "MDR32F9Qx_eeprom.h"           // Keil::Drivers:EEPROM
#include "MDR32F9Qx_eth.h"
#include "init.h"
#include "MDR32F9Qx_timer.h"            // Keil::Drivers:TIMER




//**********************************************************************************************************
//_________________________ТАКТИРОВАНИЕ МК ОТ HSE 8 МГц_____________________________________________________
//**********************************************************************************************************
void Clock_Init(void)
{
	MDR_RST_CLK->PER_CLOCK = (1<<3)|(1<<4)|(1<<27);		//Enable clock of EEPROM[3], RST_CLK[4], BKP[27]	
	MDR_RST_CLK->HS_CONTROL = 0x00000001;							//HSE - On[0]
	while((MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_HSE_RDY) == 0)
	{
		//Wait until HSE not ready
	}
	MDR_RST_CLK->CPU_CLOCK = 0x00000002;							//CPU_C1=HSE[0,1]
	MDR_BKP->REG_0E |= 0x7;				//Low > 80MGz[0..2]
	MDR_BKP->REG_0E |= (0x7<<3);	//SelectRI = Low[3..5]
	MDR_EEPROM->CMD |= (0x4<<3);	//Delay < 125MGz[3..5]

	MDR_RST_CLK->PLL_CONTROL = (PLL_CPU_MULL<<8)|RST_CLK_PLL_CONTROL_PLL_CPU_ON;	//PLL_CPU_MULL=14[8..11], PLL_CPU On[2], PLLCPU0 = 120MGz
	while((MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_PLL_CPU_RDY) == 0);		//Wait until CPU PLL not ready
	MDR_RST_CLK->CPU_CLOCK = 0x106;						//HCLK=CPU_C3[8,9]=CPU_C2[4..7]=PLLCPU0[2] //CPU_C1=HSE[0,1]

}
//__________________________________________________________________________________________________________
//ТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТ



//**********************************************************************************************************
//_________________________Инициализация блока Ethernet_____________________________________________________
//**********************************************************************************************************
void Ethernet_Init(void)
{	
	static ETH_InitTypeDef  ETH_InitStruct;
	volatile uint32_t ETH_Dilimiter;
	ETH_ClockDeInit(); 																														// Сброс тактирования Ethernet
	RST_CLK_HSE2config(RST_CLK_HSE2_ON);																					// Включение генератора HSE2 = 25МГц
    while (RST_CLK_HSE2status() != SUCCESS);	
	ETH_PHY_ClockConfig(ETH_PHY_CLOCK_SOURCE_HSE2, ETH_PHY_HCLKdiv1);							// Тактирование PHY от HSE2 = 25МГц
	ETH_BRGInit(ETH_HCLKdiv1);																										// Делитель равен 1
	ETH_ClockCMD(ETH_CLK1, ENABLE);																								// Включение тактирования блока MAC
	ETH_DeInit(MDR_ETHERNET1);																										// Сброс регистров блока MAC
	ETH_StructInit(&ETH_InitStruct);																							// Инициализация настроек Ethernet по умолчанию
	//**************************************************************
	//	Переопределение настроек PHY:
	//   - разрешение автонастройки, передатчик и приемник включены
	ETH_InitStruct.ETH_PHY_Mode = ETH_PHY_MODE_AutoNegotiation;
	ETH_InitStruct.ETH_Transmitter_RST = SET;
	ETH_InitStruct.ETH_Receiver_RST = SET;
	ETH_InitStruct.ETH_Buffer_Mode = ETH_BUFFER_MODE_AUTOMATIC_CHANGE_POINTERS;		// Режим работы буферов
	//___________________________________________________________________
	//	Задание МАС адреса микроконтроллера
	ETH_InitStruct.ETH_MAC_Address[2] = (MAC_SRC[5] << 8) | MAC_SRC[4];
	ETH_InitStruct.ETH_MAC_Address[1] = (MAC_SRC[3] << 8) | MAC_SRC[2];
	ETH_InitStruct.ETH_MAC_Address[0] = (MAC_SRC[1] << 8) | MAC_SRC[0];
	//___________________________________________________________________
	//	Разделение общей памяти на буферы для приемника и передатчика
	ETH_InitStruct.ETH_Dilimiter = 0x1000;
	
	//	Разрешаем прием пакетов только на свой адрес, прием коротких пакетов также разрешен
	ETH_InitStruct.ETH_Receive_All_Packets 			= DISABLE;
	ETH_InitStruct.ETH_Short_Frames_Reception 		= ENABLE; 
	ETH_InitStruct.ETH_Long_Frames_Reception 	    = DISABLE;
	ETH_InitStruct.ETH_Broadcast_Frames_Reception   = DISABLE;
	ETH_InitStruct.ETH_Error_CRC_Frames_Reception   = DISABLE;
	ETH_InitStruct.ETH_Control_Frames_Reception 	= DISABLE;
	ETH_InitStruct.ETH_Unicast_Frames_Reception 	= ENABLE;
	ETH_InitStruct.ETH_Source_Addr_HASH_Filter 	    = DISABLE;

	//	Инициализация блока Ethernet
	ETH_Init(MDR_ETHERNET1, &ETH_InitStruct);

	// Запуск блока PHY
	ETH_PHYCmd(MDR_ETHERNET1, ENABLE);		
}
//__________________________________________________________________________________________________________
//ТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТ



//**********************************************************************************************************
//____________________________Инициализация портов__________________________________________________________
//**********************************************************************************************************

//ПОРТ ДЛЯ CAN
void PortE_init()
{	
		PORT_DeInit(MDR_PORTE); 
		CAN_DeInit(MDR_CAN2);
		PORT_InitTypeDef PORT_InitE;
	//	PORT_DeInit(MDR_PORTE);				//clear PORTE
	  CAN_BRGInit(MDR_CAN2, CAN_HCLKdiv8);
		RST_CLK_PCLKcmd((RST_CLK_PCLK_RST_CLK | RST_CLK_PCLK_CAN2),ENABLE);	
		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTE,ENABLE);
		//------------------------------------------
		PORT_InitE.PORT_Pin	  = PORT_Pin_14;
    PORT_InitE.PORT_OE    = PORT_OE_OUT;
		PORT_InitE.PORT_FUNC  = PORT_FUNC_MAIN;
		PORT_InitE.PORT_SPEED = PORT_SPEED_MAXFAST;
		PORT_InitE.PORT_MODE  = PORT_MODE_DIGITAL;
		PORT_Init(MDR_PORTE, &PORT_InitE);
		//-------------------------------------------	
		PORT_InitE.PORT_Pin	  = PORT_Pin_15;
    PORT_InitE.PORT_OE    = PORT_OE_OUT;
		PORT_InitE.PORT_FUNC  = PORT_FUNC_MAIN;
		PORT_InitE.PORT_SPEED = PORT_SPEED_MAXFAST;
		PORT_InitE.PORT_MODE  = PORT_MODE_ANALOG;
		PORT_Init(MDR_PORTE, &PORT_InitE);
		//-------------------------------------------	
		PORT_InitE.PORT_Pin	  = PORT_Pin_13;
    PORT_InitE.PORT_OE    = PORT_OE_IN;
		PORT_InitE.PORT_FUNC  = PORT_FUNC_MAIN;
		PORT_InitE.PORT_SPEED = PORT_SPEED_MAXFAST;
		PORT_InitE.PORT_MODE  = PORT_MODE_DIGITAL;
		//-------------------------------------------
		PORT_Init(MDR_PORTE, &PORT_InitE);
		
}
//ПОРТ ДЛЯ СВЕТОДИОДОВ(ДЛЯ ОТЛАДОЧНОЙ ПЛАТЫ), при эксплуатационной прошивки закомментить
void PortD_init()
{
		PORT_InitTypeDef PORT_InitD;
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTD,ENABLE);
	PORT_DeInit(MDR_PORTD);
	PORT_InitD.PORT_FUNC  = PORT_FUNC_PORT;
  PORT_InitD.PORT_Pin   = PORT_Pin_7 | PORT_Pin_8 | PORT_Pin_9 | PORT_Pin_10 | PORT_Pin_11 | PORT_Pin_12 | PORT_Pin_13 | PORT_Pin_14;
  PORT_InitD.PORT_OE    = PORT_OE_OUT;
  PORT_InitD.PORT_MODE  = PORT_MODE_DIGITAL;
  PORT_InitD.PORT_SPEED = PORT_SPEED_SLOW;
  PORT_Init(MDR_PORTD, &PORT_InitD);
}
//ПОРТЫ ДЛЯ ПЕРЕМЫЧЕК ОПРЕДЕЛЕНИЯ АДРЕСОВ
void PortB_init()
{
	PORT_InitTypeDef PORT_InitB;
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTB,ENABLE);
	PORT_DeInit(MDR_PORTB);
	PORT_InitB.PORT_FUNC  = PORT_FUNC_MAIN;
  PORT_InitB.PORT_Pin   = PORT_Pin_All;
  PORT_InitB.PORT_OE    = PORT_OE_IN;
  PORT_InitB.PORT_MODE  = PORT_MODE_DIGITAL;
  PORT_InitB.PORT_SPEED = PORT_SPEED_FAST;
  PORT_Init	(MDR_PORTB, &PORT_InitB);
}
//__________________________________________________________________________________________________________
//ТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТ



//**********************************************************************************************************
//____________________________Инициализация таймера_________________________________________________________
//**********************************************************************************************************
void TimerInit(void)
	{
		TIMER_CntInitTypeDef TIM1Init;
		//Включение тактирования таймера
		RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER1, ENABLE);
		//Установка первого делителя тактовой частоты таймера
		TIMER_BRGInit(MDR_TIMER1, TIMER_HCLKdiv16);
		//Загрузка значений по-умолчанию в структуру TIM1Init
		TIMER_CntStructInit(&TIM1Init);
		TIM1Init.TIMER_Prescaler = 8000; // Второй делитель частоты
		TIM1Init.TIMER_Period = 1000; // Период до обновлния или основание счета
		TIMER_CntInit(MDR_TIMER1, &TIM1Init);
		//Настройка прерывания
		NVIC_EnableIRQ(TIMER1_IRQn); //Включение прерываний от TIMER1
		NVIC_SetPriority(TIMER1_IRQn, 0); //Установка приоритета прерываний 0-15
		//Включение прерывания при равенстве нулю значения TIMER1
		TIMER_ITConfig(MDR_TIMER1, TIMER_STATUS_CNT_ZERO, ENABLE);
		//Запуск таймера
		TIMER_Cmd(MDR_TIMER1, ENABLE);
		//------------------------------------------------------------------------
		//time for test
		TIMER_CntInitTypeDef TIM2Init;
		//Включение тактирования таймера
		RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER2, ENABLE);
		//Установка первого делителя тактовой частоты таймера
		TIMER_BRGInit(MDR_TIMER2, TIMER_HCLKdiv128);
		//Загрузка значений по-умолчанию в структуру TIM1Init
		TIMER_CntStructInit(&TIM2Init);
		TIM2Init.TIMER_Prescaler = 1; // Второй делитель частоты
		TIM2Init.TIMER_Period = 1000; // Период до обновлния или основание счета
		TIMER_CntInit(MDR_TIMER2, &TIM2Init);
		//Настройка прерывания
		NVIC_EnableIRQ(TIMER2_IRQn); //Включение прерываний от TIMER2
		NVIC_SetPriority(TIMER2_IRQn, 1); //Установка приоритета прерываний 0-15
		//Включение прерывания при равенстве нулю значения TIMER1
		TIMER_ITConfig(MDR_TIMER2, TIMER_STATUS_CNT_ZERO, ENABLE);
		//Запуск таймера
		TIMER_Cmd(MDR_TIMER2, ENABLE);
	}	
//__________________________________________________________________________________________________________
//ТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТ