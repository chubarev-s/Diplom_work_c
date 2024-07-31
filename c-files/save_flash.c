#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_eeprom.h>
#include "MDR1986VE1T.h"  
#include "MDR32F9Qx_eth.h"
#include "MDR32F9Qx_can.h"              // Keil::Drivers:CAN
#include "init.h"
#include "1636RR52.h"
uint8_t i;
uint16_t memory;
uint32_t memory_save;
uint8_t nar_BI[4];

//***********************************************************************************************
//****************** Функции для сохранения во флеш *********************************************
//***********************************************************************************************

void save_all_data()
{
	//Save_eth:
	//RX
	memory_save = 0x036;
	for(int i = 0; i < 101; i++)
	{
		U_1636RR52_Byte_Program(memory_save, RX_eth_pam[i]);
		memory_save++;
	}
	//TX
	memory_save = 0x013D;
	for(int i = 0; i < 101; i++)
	{
		U_1636RR52_Byte_Program(memory_save, TX_eth_pam[i]);
		memory_save++;
	}
}


void save_narabotka(uint32_t data_save_n)
{
			memory = 0x000000;
			U_1636RR52_Byte_Program(memory, (data_save_n >> 24));
			nar_BI[0] = (data_save_n >> 24);
			memory++;
			U_1636RR52_Byte_Program(memory, (data_save_n >> 16));
			nar_BI[1] = (data_save_n >> 16);
			memory++;
			U_1636RR52_Byte_Program(memory, (data_save_n >> 8));
			nar_BI[2] = (data_save_n >> 8);
			memory++;
			U_1636RR52_Byte_Program(memory, data_save_n);
			nar_BI[3] = data_save_n;

			
}

//***********************************************************************************************
void save_can(uint32_t memory, uint32_t data)
{
		uint8_t data_s[4];
	data_s[0] = (data >> 24);
	data_s[1] = (data >> 16);
	data_s[2] = (data >> 8);
	data_s[3] = data;
	for (int i = 0; i<4; i++)
	{
		U_1636RR52_Byte_Program(memory, data_s[i]);
		memory++;
	}
}

//чтение наработки
void read_nar()
{
			nar_BI[0] = U_1636RR52_Read_Word(0x000000);
			nar_BI[1] = U_1636RR52_Read_Word(0x000001);
			nar_BI[2] = U_1636RR52_Read_Word(0x000002);
			nar_BI[3] = U_1636RR52_Read_Word(0x000003);
}


