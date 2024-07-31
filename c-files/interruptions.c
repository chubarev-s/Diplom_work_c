#include "interruptions.h"
#include "MDR32F9Qx_timer.h"            // Keil::Drivers:TIMER
#include "MDR1986VE1T.h"                // Keil::Device:Startup
#include "MDR32F9Qx_rst_clk.h"          // Keil::Drivers:RST_CLK
#include "MDR32F9Qx_can.h"              // Keil::Drivers:CAN
#include "system_MDR1986VE1T.h"         // Keil::Device:Startup
#include "MDR32F9Qx_port.h"             // Keil::Drivers:PORT
#include "init.h"
#include "1636RR52.h"

#define LED_ON		PORT_SetBits 
#define LED_OFF		PORT_ResetBits
#define LED_1			MDR_PORTD, PORT_Pin_7
#define LED_2			MDR_PORTD, PORT_Pin_8
#define LED_3			MDR_PORTD, PORT_Pin_9
#define LED_4			MDR_PORTD, PORT_Pin_10
#define LED_5			MDR_PORTD, PORT_Pin_11
#define LED_6			MDR_PORTD, PORT_Pin_12
#define LED_7			MDR_PORTD, PORT_Pin_13
#define LED_8			MDR_PORTD, PORT_Pin_14
#define LED_ALL		MDR_PORTD, PORT_Pin_7|PORT_Pin_8|PORT_Pin_9|PORT_Pin_10|PORT_Pin_11|PORT_Pin_12|PORT_Pin_13|PORT_Pin_14
//_______variables___________
	
uint8_t narab_mas[4];
uint8_t test = 0x00;
uint8_t rezerv_exit = 0x00;
uint8_t led = 0x00;
uint32_t time_out = 0x00;
uint32_t test_word = 0x00;
uint32_t test_word_2 = 0x00;
uint32_t delay_ms = 0x00;


//*****************************************************************************
//__________________ПРЕРЫВАНИЯ ТАЙМЕРА ДЛЯ СЧЁТА НАРАБОТКИ_____________________
//*****************************************************************************
void TIMER1_IRQHandler() 
	{
		//Проверка что причина прерывания – обновление таймера 
		if(TIMER_GetITStatus(MDR_TIMER1, TIMER_STATUS_CNT_ZERO))
			{
				narabotka++;
				rezerv_exit++;
				/*Мигаем светодиодом обозначая работу устройства*/		
				if(narabotka % 2 == 0){
				LED_ON(MDR_PORTA, PORT_Pin_3);
				}
				else {
				LED_OFF(MDR_PORTA, PORT_Pin_3);
				
				}
				time_out++;
				//Очистка флага прерывания (это необходимо делать в конце)
				TIMER_ClearITPendingBit(MDR_TIMER1, TIMER_STATUS_CNT_ZERO);
			}
	}
//______________________________________________________________________________
//ТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТ

//*****************************************************************************
//__________________ПРЕРЫВАНИЯ ТАЙМЕРА ДЛЯ TESTa_____________________
//*****************************************************************************
void TIMER2_IRQHandler() 
	{
		//Проверка что причина прерывания – обновление таймера 
		if(TIMER_GetITStatus(MDR_TIMER2, TIMER_STATUS_CNT_ZERO))
			{

				delay_ms++;
				//Очистка флага прерывания (это необходимо делать в конце)
				TIMER_ClearITPendingBit(MDR_TIMER2, TIMER_STATUS_CNT_ZERO);
			}
	}
//______________________________________________________________________________
//ТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТТ
	
	






















