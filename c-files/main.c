#include "MDR32F9Qx_rst_clk.h"          // Keil::Drivers:RST_CLK
#include "MDR32F9Qx_port.h"             // Keil::Drivers:PORT
#include "init.h"
#include "1636RR52.h"



uint8_t LEDS_STOP=0x00, nar_arr[4];
uint32_t narabotka = 0x0000000000;


/* Инициализация переферии и запуск бесконечного цикла ожидания приёма пакетов eth и их выполнение */
int main()
{
  //°•°•°•°•°•°• АКТИВАЦИЯ ПЕРИФИРИИ °•°•°•°•°•°•

	RST_CLK_DeInit();											//очищаем тактирование	
	Clock_Init();													//Инициализация тактирования
				for(uint32_t i_c=0; i_c < 10000; i_c++){}	
	PortB_init(); 												//инициализация порта для определения перемычек
						for(uint32_t i_c=0; i_c < 10000; i_c++){}	
	MAC_ID();															//Считывание перемычек и присваивание MAC-адреса
								for(uint32_t i_c=0; i_c < 10000; i_c++){}	
	PortD_init();													//Светодиоды, используется для отработки на отдладочной плате
										for(uint32_t i_c=0; i_c < 10000; i_c++){}	
	PortE_init();													//Инициализация CAN
												for(uint32_t i_c=0; i_c < 10000; i_c++){}	
	speed_can();													//Инициализация и задание скорости работы CAN
														for(uint32_t i_c=0; i_c < 10000; i_c++){}	
	U_1636RR52_Init();										//Инициализация SPI
																for(uint32_t i_c=0; i_c < 10000; i_c++){}	
	TimerInit();													//Инициализация таймера
																		for(uint32_t i_c=0; i_c < 10000; i_c++){}	

																			
	//Раз SPI и флешка работает, то можем считать наработку:
	//Считываем наработку из памяти:
	narab_mas[0] = U_1636RR52_Read_Word(0x00000);
	narab_mas[1] = U_1636RR52_Read_Word(0x00001);
	narab_mas[2] = U_1636RR52_Read_Word(0x00002);
	narab_mas[3] = U_1636RR52_Read_Word(0x00003);
	narabotka =	((narab_mas[0]) << 24) + ((narab_mas[1]) << 16) + ((narab_mas[2]) << 8) + narab_mas[3]; //Считываем наработку из flash
	if(narabotka == 0xFFFFFFFF)
		{
			narabotka = 0x00000000;
		}
	

	//Инициализация Ethernet
	Ethernet_Init();											
  //Запуск блока Ethernet
	Ethernet_Start();											
	
	PORT_SetBits(MDR_PORTA, PORT_Pin_2);
	PORT_ResetBits(MDR_PORTA, PORT_Pin_1);
	PORT_ResetBits(MDR_PORTA, PORT_Pin_0);				
	//°•°•°•°•°•°• ЗАПУСК АНАЛИЗА СООБЩЕНИЙ ПО ETHERNET И ИХ ВЫПОЛНЕНИЕ °•°•°•°•°•°•
	Ethernet_ProcessLoop();								//Бесконечный цикл обработки Ethernet пакетов 
}
