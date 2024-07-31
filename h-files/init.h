#ifndef __INIT_H
#define __INIT_H

#include "MDR32F9Qx_eth.h"

#define BI_ver 0x01 // Версия ПО БИ
//---------------------------------------
//poka chto dlya testa ID_PP zadaem tak:
#define ID_letters_1	0xE1
#define ID_letters_2	0xE2 
#define ID_letters_3	0xE3 
#define ID_num_1			0x01
#define ID_num_2			0x02
#define ID_num_3			0x03
//---------------------------------------
#define PLL_CPU_MULL 15 	//задаём делитель частоты, для 8МГц осц. получается 8*16(с нуля отсчёт 15)=128МГц
void PortE_init(void);
void PortD_init(void);
void PortB_init(void);
void MAC_ID(void);
void Clock_Init(void);
void Ethernet_Init(void);
void Ethernet_Start(void);
void Ethernet_ProcessLoop(void);
void speed_can(void);
extern uint8_t MAC_SRC [6];
void CAN_Transmition(void);
void CAN_receive(void);
void TimerInit(void);
void Buf_Clear(void);
void otpravka_eth(void);
void exit_save(void);
void save_narabotka(uint32_t data);
void save_can(uint32_t memory, uint32_t data);
//void Delay_Byte_Program ();


extern uint32_t chek_data;

extern uint8_t narab_mas[4];

extern uint32_t narabotka;

extern uint8_t data_mas[4];
extern uint32_t flash_adr;

extern uint8_t Eth_RX_save[100];
extern uint8_t Eth_TX_save[100];

void ETH_transmite_save(uint8_t TX_eth_pam[]);
void ETH_recive_save(uint8_t RX_eth_pam[]);
void save_all_data(void);
void Read_blok_nar(void);
extern uint8_t rezerv_exit; 
extern uint8_t TX_eth_pam[70];
extern uint8_t RX_eth_pam[70];
extern uint8_t LEDS_STOP;
extern uint32_t time_out;

extern uint32_t delay_ms;


#define BUF_0						MDR_CAN2->BUF_CON[0]
#define BUF_1						MDR_CAN2->BUF_CON[1]
#define BUF_2						MDR_CAN2->BUF_CON[2]
#define BUF_3						MDR_CAN2->BUF_CON[3]
#define BUF_4						MDR_CAN2->BUF_CON[4]
#define BUF_5						MDR_CAN2->BUF_CON[5]
#define BUF_6						MDR_CAN2->BUF_CON[6]
#define BUF_7						MDR_CAN2->BUF_CON[7]
#define BUF_8						MDR_CAN2->BUF_CON[8]
#define BUF_9						MDR_CAN2->BUF_CON[9]
#define BUF_10					MDR_CAN2->BUF_CON[10]
#define BUF_11					MDR_CAN2->BUF_CON[11]
#define BUF_12					MDR_CAN2->BUF_CON[12]
#define BUF_13					MDR_CAN2->BUF_CON[13]
#define BUF_14					MDR_CAN2->BUF_CON[14]
#define BUF_15					MDR_CAN2->BUF_CON[15]
#define BUF_16					MDR_CAN2->BUF_CON[16]
#define BUF_17					MDR_CAN2->BUF_CON[17]
#define BUF_18					MDR_CAN2->BUF_CON[18]
#define BUF_19					MDR_CAN2->BUF_CON[19]
#define BUF_20					MDR_CAN2->BUF_CON[20]
#define BUF_21					MDR_CAN2->BUF_CON[21]
#define BUF_22					MDR_CAN2->BUF_CON[22]
#define BUF_23					MDR_CAN2->BUF_CON[23]
#define BUF_24					MDR_CAN2->BUF_CON[24]
#define BUF_25					MDR_CAN2->BUF_CON[25]
#define BUF_26					MDR_CAN2->BUF_CON[26]
#define BUF_27					MDR_CAN2->BUF_CON[27]
#define BUF_28					MDR_CAN2->BUF_CON[28]
#define BUF_29					MDR_CAN2->BUF_CON[29]
#define BUF_30					MDR_CAN2->BUF_CON[30]
#define BUF_31					MDR_CAN2->BUF_CON[31]





#endif //__INIT_H