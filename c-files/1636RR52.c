// ***********************************************************************************
// Микроконтроллер: 1986ВЕ92QI
// Файл:  1636RR52.c
// Модуль:  Работа с микросхемой Flash-памяти 1636РР52У1
// Компилятор:  Armcc 5.03.0.76 из комплекта Keil uVision 4.72.1.0 
// ***********************************************************************************
#include "MDR1986VE1T.h"                // Keil::Device:Startup
#include "MDR32F9Qx_rst_clk.h"          // Keil::Drivers:RST_CLK
#include "system_MDR1986VE1T.h"         // Keil::Device:Startup
#include "MDR32F9Qx_port.h"             // Keil::Drivers:PORT
#include <MDR32F9Qx_eeprom.h>
#include "1636RR52.h"
#include "MDR32F9Qx_ssp.h"              // Keil::Drivers:SSP
#include "variant.h"
#include "init.h"

// Ожидание окончания записи байта 
static void Delay_Byte_Program (void);
// Ожидание стирания всей памяти  
static void Delay_Chip_Erase (void);
// Ожидание стирания сектора 
static void Delay_Sector_Erase (void);
uint8_t read_bite[4];
uint8_t out_while = 0x00;

// Инициализация
uint32_t U_1636RR52_Init (void)
{
	uint32_t result;
	
	// Структура для инициализации линий ввода-вывода
  PORT_InitTypeDef PortInitStructure; 
	// Структура для инициализации SPI
  SSP_InitTypeDef SSPInitStructure; 
	
  // Разрешить тактирование SSP и PORT
  RST_CLK_PCLKcmd (RST_CLK_PCLK_SSP2 | RST_CLK_PCLK_PORTC, ENABLE); //RST_CLK_PCLK_SSP1

	
  // Конфигурация выводов SSP2
	PORT_StructInit (&PortInitStructure);
  PortInitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
  PortInitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
  PortInitStructure.PORT_PD_SHM = PORT_PD_SHM_OFF;
  PortInitStructure.PORT_PD = PORT_PD_DRIVER;
  PortInitStructure.PORT_GFEN = PORT_GFEN_OFF;
  PortInitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
  PortInitStructure.PORT_MODE = PORT_MODE_DIGITAL;

  // Деинициализация SSP2
  SSP_DeInit (U_1636RR52_SPI);

  SSP_StructInit (&SSPInitStructure);

	// Задать коэффициент деления частоты PCLK для получения частоты F_SSPCLK
	SSP_BRGInit (U_1636RR52_SPI, U_1636RR52_SSP_BRG);

	
	// SSP_TX - выход MOSI
	PortInitStructure.PORT_OE = PORT_OE_OUT;
  PortInitStructure.PORT_FUNC = U_1636RR52_MOSI_FUNC;
	PortInitStructure.PORT_Pin = U_1636RR52_MOSI_PIN;
	PORT_Init (U_1636RR52_MOSI_PORT, &PortInitStructure);	

  // SSP_RX - вход MISO
	PortInitStructure.PORT_OE = PORT_OE_IN;
  PortInitStructure.PORT_FUNC = U_1636RR52_MISO_FUNC;
	PortInitStructure.PORT_Pin = U_1636RR52_MISO_PIN;
	PORT_Init (U_1636RR52_MISO_PORT, &PortInitStructure);

	// SSP_SCLK - выход SCLK
	PortInitStructure.PORT_OE = PORT_OE_OUT;
  PortInitStructure.PORT_FUNC = U_1636RR52_SCLK_FUNC;
	PortInitStructure.PORT_Pin = U_1636RR52_SCLK_PIN;
	PORT_Init (U_1636RR52_SCLK_PORT, &PortInitStructure);	

	// SSP_FSS - выход SS
	PortInitStructure.PORT_OE = PORT_OE_OUT;
  PortInitStructure.PORT_FUNC = U_1636RR52_SS_FUNC;
	PortInitStructure.PORT_Pin = U_1636RR52_SS_PIN;
	PORT_Init (U_1636RR52_SS_PORT, &PortInitStructure);

	// Конфигурация SSP (Master)
	SSPInitStructure.SSP_Mode = SSP_ModeMaster;          // Режим ведущего (Master) 
	
	// Предделители частоты сигнала SCLK
	// F_SCLK = F_SSPCLK / ( CPSDVR * (1 + SCR) = 10МГц / (10 * (1 + 1)) = 500 кГц 
	SSPInitStructure.SSP_CPSDVSR = U_1636RR52_SSP_CPSDVSR;  // 2 to 254
	SSPInitStructure.SSP_SCR = U_1636RR52_SSP_SCR;          // 0 to 255

	SSPInitStructure.SSP_WordLength = SSP_WordLength8b;  // Размер фрейма 
	
	// Режим 3
	// При этом FSS устанавливается в низкое состояние на время передачи всего блока данных  
	SSPInitStructure.SSP_SPO = SSP_SPO_High;             // Поляризация SCLK
	SSPInitStructure.SSP_SPH = SSP_SPH_2Edge;            // Фаза SCLK (по заднему фронту)
	
	SSPInitStructure.SSP_HardwareFlowControl = SSP_HardwareFlowControl_SSE;  // Аппаратное управление передачей данных (приемо-передатчик пока отключен отключен) 
	SSPInitStructure.SSP_FRF = SSP_FRF_SPI_Motorola; 	   // Формат фрейма (Motorola)
	SSP_Init (U_1636RR52_SPI, &SSPInitStructure);

  // Разрешить работу SSP
  SSP_Cmd (U_1636RR52_SPI, ENABLE);	

  return result;
}

// Передать блок данных
void static SPI_Write_Block (uint16_t* src, uint16_t* dst, uint32_t Count)
{
  uint32_t i;
	volatile uint32_t data;
	
	// Дождаться полного освобождения буфера
	while ((U_1636RR52_SPI->SR & SSP_FLAG_BSY)); //#define U_1636RR52_SPI MDR_SSP2

  // Вычитать все данные из входного буфера, так как там может лежать мусор	
	while ((U_1636RR52_SPI->SR & SSP_FLAG_RNE))
		data = U_1636RR52_SPI->DR;
	
	for (i = 0; i < Count; i++)
	{
		// Дождаться появления свободного места в буфера передатчика
		while (!(U_1636RR52_SPI->SR & SSP_FLAG_TNF));
		
	  // Передать байт
		U_1636RR52_SPI->DR = *src++;
	}

	for (i = 0; i < Count; i++)
	{
		// Дождаться приема байта
		while (!(U_1636RR52_SPI->SR & SSP_FLAG_RNE));
		
	  // Читаем байт, полученный от 1636РР52У
	  *dst++ = U_1636RR52_SPI->DR;		
	}

}
	


// **** Команды *****
//*************************************************************************
// Чтение 4 байт данных, начиная с заданного адреса на частоте до 15 МГц
//*************************************************************************

uint32_t U_1636RR52_Read_Word (uint32_t addr)
{
  TVariant32 result;
  TVariant32 Addr;

	uint16_t src[4];
  uint32_t i;
	volatile uint32_t data;
	
	Addr.DWord = addr;
	
	src[0] = U_1636RR52_CMD_READ_ARRAY_15; //#define U_1636RR52_CMD_READ_ARRAY_15 0x03 - команда 
	src[1] = Addr.Byte[2];
	src[2] = Addr.Byte[1];
	src[3] = Addr.Byte[0];
	
	// Дождаться полного освобождения SPI
	while (((U_1636RR52_SPI->SR) & SSP_FLAG_BSY));
	
  // Вычитать все данные из входного буфера, так как там может лежать мусор	
	while ((U_1636RR52_SPI->SR & SSP_FLAG_RNE))
		data = U_1636RR52_SPI->DR;
	
	
  // Передать 4 байта с кодом команды и адресом
	for (i = 0; i < 4; i++)
		U_1636RR52_SPI->DR = src[i];

  // Передать 4 пустых байта 
	for (i = 0; i < 4; i++)
		U_1636RR52_SPI->DR = 0;

	// Читаем ответные 4 байта, хоть они нам и не нужны
	for (i = 0; i < 4; i++)
	{
		// Дождаться приема байта
		while (!(U_1636RR52_SPI->SR & SSP_FLAG_RNE));
		U_1636RR52_SPI->DR;
	}
	
	// Читаем и сохраняем 4 полезных ответных байта
	for (i = 0; i < 4; i++)
	{
		// Дождаться приема байта
		while (!(U_1636RR52_SPI->SR & SSP_FLAG_RNE));
		result.Byte[i] = U_1636RR52_SPI->DR;
		read_bite[i] = result.Byte[i];
	}

  return result.DWord;	
}

//*************************************************************************
// Чтение массива данных на частоте до 15 МГц
//*************************************************************************

uint32_t U_1636RR52_Read_Array_15 (uint32_t addr, uint8_t* dst, uint32_t Count)
{
	uint16_t src[4];
  TVariant32 Addr;
  uint32_t i,j;
	volatile uint32_t data;
	uint8_t* Dst;
	
	Dst = dst;
	
	Addr.DWord = addr;
	
	src[0] = U_1636RR52_CMD_READ_ARRAY_15; //#define U_1636RR52_CMD_READ_ARRAY_15 0x03
	src[1] = Addr.Byte[2];
	src[2] = Addr.Byte[1];
	src[3] = Addr.Byte[0];
	
	// Дождаться полного освобождения SPI
	while (((U_1636RR52_SPI->SR) & SSP_FLAG_BSY));

  // Вычитать все данные из входного буфера, так как там может лежать мусор	
	while ((U_1636RR52_SPI->SR & SSP_FLAG_RNE))
		data = U_1636RR52_SPI->DR;	
	
  // Передать 4 байта с кодом команды и адресом
	for (i = 0; i < 4; i++)
		U_1636RR52_SPI->DR = src[i];

	// Читаем ответные 4 байта, хоть они нам и не нужны
	for (i = 0; i < 4; i++)
	{
		// Дождаться приема байта
		while (!(U_1636RR52_SPI->SR & SSP_FLAG_RNE));
		data = U_1636RR52_SPI->DR;
	}
	
	// Передаем пустой байт и принимаем ответный
	for (i = 0, j = 0; i < Count; i++)
	{
		// Дождаться появления свободного места в выходном буфере
		while (!(U_1636RR52_SPI->SR & SSP_FLAG_TNF))
		{
	    // Читаем байт, полученный от 1636РР52У
		  if (U_1636RR52_SPI->SR & SSP_FLAG_RNE)
			{
	     *(Dst++) = U_1636RR52_SPI->DR;					
			 j++;	
			}
		}
		// Передаем пустой байт
		U_1636RR52_SPI->DR = 0;		
		
		// Читаем байт, полученный от 1636РР52У
		if (U_1636RR52_SPI->SR & SSP_FLAG_RNE)
		{
		 *(Dst++) = U_1636RR52_SPI->DR;					
		 j++;	
		}		
	}

  // Читаем остаток еще не принятых байт	
	while (U_1636RR52_SPI->SR & (SSP_FLAG_RNE | SSP_FLAG_BSY))
	{
		if ((U_1636RR52_SPI->SR & SSP_FLAG_RNE) && j < Count)
		{
		 *(Dst++) = U_1636RR52_SPI->DR;					
		 j++;
		}
	}
	
	return j; // Сколько байт прочитали
}


//*************************************************************************
// Чтение массива данных на частоте до 100 МГц
//*************************************************************************

uint32_t U_1636RR52_Read_Array_100 (uint32_t addr, uint8_t* dst, uint32_t Count)
{
	uint16_t src[5];
  TVariant32 Addr;
  uint32_t i,j;
	volatile uint32_t data;
	uint8_t* Dst;
	
	Dst = dst;
	
	Addr.DWord = addr;
	
	src[0] = U_1636RR52_CMD_READ_ARRAY_100;
	src[1] = Addr.Byte[2];
	src[2] = Addr.Byte[1];
	src[3] = Addr.Byte[0];
	src[4] = 0;  // Фиктивный байт
	
	// Дождаться полного освобождения SPI
	while (((U_1636RR52_SPI->SR) & SSP_FLAG_BSY));

  // Вычитать все данные из входного буфера, так как там может лежать мусор	
	while ((U_1636RR52_SPI->SR & SSP_FLAG_RNE))
		data = U_1636RR52_SPI->DR;	
	
  // Передать 4 байта с кодом команды и адресом
	for (i = 0; i < 5; i++)
		U_1636RR52_SPI->DR = src[i];

	// Читаем ответные 4 байта, хоть они нам и не нужны
	for (i = 0; i < 5; i++)
	{
		// Дождаться приема байта
		while (!(U_1636RR52_SPI->SR & SSP_FLAG_RNE));
		data = U_1636RR52_SPI->DR;
	}
	
	// Передаем пустой байт и принимаем ответный
	for (i = 0, j = 0; i < Count; i++)
	{
		// Дождаться появления свободного места в выходном буфере
		while (!(U_1636RR52_SPI->SR & SSP_FLAG_TNF))
		{
	    // Читаем байт, полученный от 1636РР52У
		  if (U_1636RR52_SPI->SR & SSP_FLAG_RNE)
			{
	     *(Dst++) = U_1636RR52_SPI->DR;					
			 j++;	
			}
		}
		// Передаем пустой байт
		U_1636RR52_SPI->DR = 0;		
		
		// Читаем байт, полученный от 1636РР52У
		if (U_1636RR52_SPI->SR & SSP_FLAG_RNE)
		{
		 *(Dst++) = U_1636RR52_SPI->DR;					
		 j++;	
		}		
	}

  // Читаем остаток еще не принятых байт	
	while (U_1636RR52_SPI->SR & (SSP_FLAG_RNE | SSP_FLAG_BSY))
	{
		if ((U_1636RR52_SPI->SR & SSP_FLAG_RNE) && j < Count)
		{
		 *(Dst++) = U_1636RR52_SPI->DR;					
		 j++;
		}
	}
	
	return j; // Сколько байт прочитали
}

//*************************************************************************
// Стирание сектора 
//*************************************************************************

uint32_t U_1636RR52_Sector_Erase (uint32_t Sector)
{
	uint32_t result;
	
	uint16_t src[U_1636RR52_CMD_L_SECTOR_ERASE], dst[U_1636RR52_CMD_L_SECTOR_ERASE];
	
  TVariant32 Addr;
	
	Addr.DWord = Sector;
	
	src[0] = U_1636RR52_CMD_SECTOR_ERASE; //#define U_1636RR52_CMD_SECTOR_ERASE 0xD8
	src[1] = Addr.Byte[2];
	src[2] = Addr.Byte[1];
	src[3] = Addr.Byte[0];
	
	// Разрешить запись
  U_1636RR52_Write_Enable ();	
	
  // Передать блок данных
  SPI_Write_Block (src, dst, U_1636RR52_CMD_L_SECTOR_ERASE);
	
	// Время стирания сектора, мс:  55
  // Ожидание стирания сектора 
  Delay_Sector_Erase ();
	
  return result;
}
//*************************************************************************
// Стирание всей памяти  
//*************************************************************************
uint32_t U_1636RR52_Chip_Erase (void)
{
	uint32_t result;

	// Разрешить запись
  U_1636RR52_Write_Enable ();	
	
	// Передать байт c командой
	U_1636RR52_SPI->DR = U_1636RR52_CMD_CHIP_ERASE;
	
	// Время операции стирания микросхемы, мс:  110 
	// Ожидание стирания всей памяти  
	Delay_Chip_Erase ();
	
	
  return result;
}

//*************************************************************************
// Программирование байта 
//*************************************************************************

void U_1636RR52_Byte_Program (uint32_t addr, uint8_t Value)
{
	uint16_t src[U_1636RR52_CMD_L_BYTE_PROGRAM], dst[U_1636RR52_CMD_L_BYTE_PROGRAM];
	
  TVariant32 Addr;
	
	Addr.DWord = addr;
	
	src[0] = U_1636RR52_CMD_BYTE_PROGRAM; //#define U_1636RR52_CMD_BYTE_PROGRAM 0x02
	src[1] = Addr.Byte[2];
	src[2] = Addr.Byte[1];
	src[3] = Addr.Byte[0];
	src[4] = Value;
	
	// Разрешить запись
  U_1636RR52_Write_Enable ();	
	
  // Передать блок данных
  SPI_Write_Block (src, dst, 5);//Value); //U_1636RR52_CMD_L_BYTE_PROGRAM);
	
	// Время операции программирования байта, мкс:  45 
	// Ожидание окончания записи байта 
	Delay_Byte_Program ();
}

//*************************************************************************
// Программирование блока данных 
//*************************************************************************

void U_1636RR52_Block_Program (uint32_t addr, uint8_t* Src, uint32_t Count)
{
	uint16_t src[U_1636RR52_CMD_L_BYTE_PROGRAM], dst[U_1636RR52_CMD_L_BYTE_PROGRAM];
  TVariant32 Addr;
	uint32_t i;

  Addr.DWord = addr;
	
	src[0] = U_1636RR52_CMD_BYTE_PROGRAM; //#define U_1636RR52_CMD_BYTE_PROGRAM 0x02

  // Записать все байты блока
	for (i = 0; i < Count; i++, Addr.DWord++)
	{
		src[1] = Addr.Byte[2];
		src[2] = Addr.Byte[1];
		src[3] = Addr.Byte[0];
		src[4] = Src[i];
		
		// Разрешить запись
		U_1636RR52_Write_Enable ();	
		
		// Передать блок данных
		SPI_Write_Block (src, dst, U_1636RR52_CMD_L_BYTE_PROGRAM);
		
		// Время операции программирования байта, мкс:  45 
		// Ожидание окончания записи байта 
		Delay_Byte_Program ();
	}
}


//*************************************************************************
// Разрешение записи 
//*************************************************************************

void U_1636RR52_Write_Enable (void)
{
	// Дождаться полного окончания предыдущих операций
	while ((U_1636RR52_SPI->SR & SSP_FLAG_BSY));

	// Передать байт
	U_1636RR52_SPI->DR = U_1636RR52_CMD_WRITE_ENABLE; //#define U_1636RR52_CMD_WRITE_ENABLE 0x06

	// Дождаться полного окончания операции
	while ((U_1636RR52_SPI->SR & SSP_FLAG_BSY));
	
	U_1636RR52_SPI->DR; // Хоть результат чтения и не нужен, но прочитать всё равно надо
}

// Запрет записи  
void U_1636RR52_Write_Disable (void)
{
	// Дождаться полного окончания предыдущих операций
	while ((U_1636RR52_SPI->SR & SSP_FLAG_BSY));

	// Передать байт
	U_1636RR52_SPI->DR = U_1636RR52_CMD_WRITE_DISABLE;

	// Дождаться полного окончания операции
	while ((U_1636RR52_SPI->SR & SSP_FLAG_BSY));
	
	U_1636RR52_SPI->DR; // Хоть результат чтения и не нужен, но прочитать всё равно надо	
	
}

//*************************************************************************
// Защита сектора
//*************************************************************************

uint32_t U_1636RR52_Protect_Sector (uint32_t addr)
{
  TVariant32 result;

	uint16_t src[U_1636RR52_CMD_L_PROTECT_SECTOR], dst[U_1636RR52_CMD_L_PROTECT_SECTOR];
	
  TVariant32 Addr;
	
	Addr.DWord = addr;
	
	src[0] = U_1636RR52_CMD_PROTECT_SECTOR;
	src[1] = Addr.Byte[2];
	src[2] = Addr.Byte[1];
	src[3] = Addr.Byte[0];

	// Разрешить запись
  U_1636RR52_Write_Enable ();		
	
  // Передать блок данных
  SPI_Write_Block (src, dst, U_1636RR52_CMD_L_PROTECT_SECTOR);
	
  return result.DWord;
}

//*************************************************************************
// Снятие защиты сектора
//*************************************************************************

uint32_t U_1636RR52_Unprotect_Sector (uint32_t addr)
{
  TVariant32 result;

	uint16_t src[U_1636RR52_CMD_L_UNPROTECT_SECTOR], dst[U_1636RR52_CMD_L_UNPROTECT_SECTOR];
	
  TVariant32 Addr;
	
	Addr.DWord = addr;
	
	src[0] = U_1636RR52_CMD_UNPROTECT_SECTOR; //#define U_1636RR52_CMD_UNPROTECT_SECTOR 0x39
	src[1] = Addr.Byte[2];
	src[2] = Addr.Byte[1];
	src[3] = Addr.Byte[0];

	// Разрешить запись
  U_1636RR52_Write_Enable ();	
	
  // Передать блок данных
  SPI_Write_Block (src, dst, U_1636RR52_CMD_L_UNPROTECT_SECTOR);
	
  return result.DWord;
}

//*************************************************************************
// Чтение регистра защиты сектора 
//*************************************************************************

uint32_t U_1636RR52_Read_Sector_Protection_Register (uint32_t addr)
{
  TVariant32 result;

	uint16_t src[U_1636RR52_CMD_L_READ_SECTOR_PROTECTION_REGISTER], dst[U_1636RR52_CMD_L_READ_SECTOR_PROTECTION_REGISTER];
	
  TVariant32 Addr;
	
	Addr.DWord = addr;
	
	src[0] = U_1636RR52_CMD_READ_SECTOR_PROTECTION_REGISTER;
	src[1] = Addr.Byte[2];
	src[2] = Addr.Byte[1];
	src[3] = Addr.Byte[0];
	src[4] = 0;
	
  // Передать блок данных
  SPI_Write_Block (src, dst, U_1636RR52_CMD_L_READ_SECTOR_PROTECTION_REGISTER);
	
	result.DWord = 0;
  result.Byte[0] = dst[4];	
	
  return result.DWord;
}

//*************************************************************************
// Чтение регистра статуса
//*************************************************************************

uint32_t U_1636RR52_Read_Status_Register (void)
{
  TVariant32 result;

	uint16_t src[U_1636RR52_CMD_L_READ_STATUS_REGISTER], dst[U_1636RR52_CMD_L_READ_STATUS_REGISTER];
	
	src[0] = U_1636RR52_CMD_READ_STATUS_REGISTER;
	
  // Передать блок данных
  SPI_Write_Block (src, dst, U_1636RR52_CMD_L_READ_STATUS_REGISTER);

	result.DWord = 0;
  result.Byte[0] = dst[1];	
	
  return result.DWord;
}

//*************************************************************************
// Запись регистра статуса 
// status - комбинация битов U_1636RR52_SR_RSTE и U_1636RR52_SR_SPRL
//*************************************************************************

void U_1636RR52_Write_Status_Register (uint8_t status)
{
	uint16_t src[U_1636RR52_CMD_L_WRITE_STATUS_REGISTER], dst[U_1636RR52_CMD_L_WRITE_STATUS_REGISTER];
	
	src[0] = U_1636RR52_CMD_WRITE_STATUS_REGISTER;
	src[1] = status & 0xC0; // Можно устанавливать только два старших бита
	
	// Разрешить запись
  U_1636RR52_Write_Enable ();		
	
  // Передать блок данных
  SPI_Write_Block (src, dst, U_1636RR52_CMD_L_WRITE_STATUS_REGISTER);

}

//*************************************************************************
// Сброс
//*************************************************************************

void U_1636RR52_Reset (void)
{
	uint16_t src[U_1636RR52_CMD_L_RESET], dst[U_1636RR52_CMD_L_RESET];
	
	src[0] = U_1636RR52_CMD_RESET;
	src[1] = 0xD0; // Подтверждающий байт
	
  // Передать блок данных
  SPI_Write_Block (src, dst, U_1636RR52_CMD_L_RESET);
}

// Чтение ID кодов производителя и микросхемы
uint32_t U_1636RR52_Read_ID (void)
{
  TVariant32 result;

	uint16_t src[U_1636RR52_CMD_L_READ_ID], dst[U_1636RR52_CMD_L_READ_ID];
	
	src[0] = U_1636RR52_CMD_READ_ID;
	src[1] = 0x00;
	src[2] = 0x00;
	
  // Передать блок данных
  SPI_Write_Block (src, dst, U_1636RR52_CMD_L_READ_ID);

	result.DWord = 0;
  result.Byte[0] = dst[1];	// Код производителя 0x01
  result.Byte[1] = dst[2];	// Код микросхемы 0xC8
	
  return result.DWord;
}

//*************************************************************************
//*************************************************************************

// Ожидание окончания записи байта 
static void Delay_Byte_Program (void)
{
	// Время операции программирования байта, мкс:  45 
	/*out_while = 0;
	delay_ms = 0;
	while(out_while < 1){
		if(delay_ms > 45){
			out_while = 1;
		}
	}*/
	volatile uint32_t t;
	
  for (t = 1300; t ; --t);	// ~57 мкс

  // !!! Можно переделать по-своему  
	// ...
}


// Ожидание стирания всей памяти  
static void Delay_Chip_Erase (void)
{
	// Время операции стирания микросхемы, мс:  110 
 // os_dly_wait (110);
	out_while = 0;
	delay_ms = 0;
	//while(out_while < 1){
	//	if(delay_ms > 110) {
	//		out_while = 1;
	//	}
	//}

  // !!! Можно переделать по-своему  
	// ...
}

// Ожидание стирания сектора 
static void Delay_Sector_Erase (void)
{
	// Время стирания сектора, мс:  55
 // os_dly_wait (55);
/*	out_while = 0;
	delay_ms = 0;
	while(out_while < 1){
		if(delay_ms > 55){
			out_while = 1;
		}
	}*/
	/*int time = 0;
	while(time <1000){
	time++;
	}*/
  // !!! Можно переделать по-своему  
	// ...
}

