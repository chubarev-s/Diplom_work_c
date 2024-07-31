// ***********************************************************************************
// Микроконтроллер: 1986ВЕ92QI
// Файл:  1636RR52.h
// Модуль:  Работа с микросхемой Flash-памяти 1636РР52У1
// Компилятор:  Armcc 5.03.0.76 из комплекта Keil uVision 4.72.1.0 
// ***********************************************************************************

#ifndef __U_1636RR52
 #define __U_1636RR52

#include "MDR1986VE1T.h" 
#include "MDR32F9Qx_port.h" 


// **** Настройки ****

// Контроллер SPI
#define U_1636RR52_SPI MDR_SSP2 //MDR_SSP1

// MOSI
#define U_1636RR52_MOSI_PORT MDR_PORTC
#define U_1636RR52_MOSI_PIN  PORT_Pin_9 //5
#define U_1636RR52_MOSI_FUNC PORT_FUNC_MAIN;

// MISO
#define U_1636RR52_MISO_PORT MDR_PORTC
#define U_1636RR52_MISO_PIN  PORT_Pin_10	//6
#define U_1636RR52_MISO_FUNC PORT_FUNC_MAIN;
	
// SCLK
#define U_1636RR52_SCLK_PORT MDR_PORTC
#define U_1636RR52_SCLK_PIN  PORT_Pin_11 //7
#define U_1636RR52_SCLK_FUNC PORT_FUNC_MAIN;

// SS
#define U_1636RR52_SS_PORT MDR_PORTC
#define U_1636RR52_SS_PIN  PORT_Pin_12	//8
#define U_1636RR52_SS_FUNC PORT_FUNC_MAIN;

// Частота SCLK
// F_SCLK = PCLK / SSP_BRG / (CPSDVR * (1 + SCR))
// PCLK - частота тактирования периферии 
#define U_1636RR52_SSP_BRG SSP_HCLKdiv16 //8  // 1,2,4,8,16,32,64,128 F_SSCLK
#define U_1636RR52_SSP_CPSDVSR 4 //8         // 2 to 254 
#define U_1636RR52_SSP_SCR 2             // 0..255


/*
--------------------------------------------------------------------------------------
Команда               Код команды    Частота     Байт   Байт      Байт      Всего 
                                                 адреса фиктивных данных    байт
--------------------------------------------------------------------------------------
Read Array            03h 0000 0011  до 15 МГц   3      0         1       * 5 и выше
                      0Bh 0000 1011  до 100 МГц  3      1         1         6 и выше
--------------------------------------------------------------------------------------
Sector Erase          D8h 1101 1000  до 100 МГц  3      0         0       * 4
--------------------------------------------------------------------------------------
Chip Erase            60h 0110 0000  до 100 МГц  0      0         0       * 1
--------------------------------------------------------------------------------------
Byte Program          02h 0000 0010  до 100 МГц  3      0         1       * 5
--------------------------------------------------------------------------------------
Write Enable          06h 0000 0110  до 100 МГц  0      0         0       * 1
--------------------------------------------------------------------------------------
Write Disable         04h 0000 0100  до 100 МГц  0      0         0       * 1
--------------------------------------------------------------------------------------
Protect Sector        36h 0011 0110  до 100 МГц  3      0         0         4
--------------------------------------------------------------------------------------
Unprotect Sector      39h 0011 1001  до 100 МГц  3      0         0         4
--------------------------------------------------------------------------------------
Read Sector 
Protection Register   3Ch 0011 1100  до 100 МГц  3      0         1         5
--------------------------------------------------------------------------------------
Read Status Register  05h 0000 0101  до 100 МГц  0      0         1       * 2
--------------------------------------------------------------------------------------
Write Status Register 01h 0000 0001  до 100 МГц  0      0         1         2
--------------------------------------------------------------------------------------
Reset                 F0h 1111 0000  до 100 МГц  0      0         1       * 2
--------------------------------------------------------------------------------------
Read ID устройства и 
производителя         9Fh 1001 1111  до 100 МГц  0      0         2       * 3
--------------------------------------------------------------------------------------
*/

// **** Коды команд ****
// Read Array - чтение массива данных на частоте до 15 МГц
#define U_1636RR52_CMD_READ_ARRAY_15 0x03
// Read Array - чтение массива данных на частоте до 100 МГц
#define U_1636RR52_CMD_READ_ARRAY_100 0x0B
// Sector Erase  - Стирание сектора 
#define U_1636RR52_CMD_SECTOR_ERASE 0xD8
#define U_1636RR52_CMD_L_SECTOR_ERASE 4
// Chip Erase  - Стирание всей памяти  
#define U_1636RR52_CMD_CHIP_ERASE 0x60
// Byte Program - Программирование байта 
#define U_1636RR52_CMD_BYTE_PROGRAM 0x02
#define U_1636RR52_CMD_L_BYTE_PROGRAM 5
// Write Enable - Разрешение записи
#define U_1636RR52_CMD_WRITE_ENABLE 0x06
#define U_1636RR52_CMD_L_WRITE_ENABLE 1
// Write Disable - Запрет записи 
#define U_1636RR52_CMD_WRITE_DISABLE 0x04
#define U_1636RR52_CMD_L_WRITE_DISABLE 1
// Protect Sector - Защита сектора 
#define U_1636RR52_CMD_PROTECT_SECTOR 0x36
#define U_1636RR52_CMD_L_PROTECT_SECTOR 4
// Unprotect Sector - Снятие защиты сектора 
#define U_1636RR52_CMD_UNPROTECT_SECTOR 0x39
#define U_1636RR52_CMD_L_UNPROTECT_SECTOR 4
// Read Sector Protection Register - Чтение регистра защиты сектора 
#define U_1636RR52_CMD_READ_SECTOR_PROTECTION_REGISTER 0x3C
#define U_1636RR52_CMD_L_READ_SECTOR_PROTECTION_REGISTER 5
// Read Status Register - Чтение регистра статуса
#define U_1636RR52_CMD_READ_STATUS_REGISTER 0x05
#define U_1636RR52_CMD_L_READ_STATUS_REGISTER 2
// Write Status Register - Запись регистра статуса 
#define U_1636RR52_CMD_WRITE_STATUS_REGISTER 0x01
#define U_1636RR52_CMD_L_WRITE_STATUS_REGISTER 2
// Reset - Сброс  
#define U_1636RR52_CMD_RESET  0xF0
#define U_1636RR52_CMD_L_RESET  2
// Read ID - Чтение ID кодов производителя и микросхемы
#define U_1636RR52_CMD_READ_ID 0x9F
#define U_1636RR52_CMD_L_READ_ID 3


// **** Биты регистра статуса ****

// 0 Бит состояния готовности устройства (R)
//  0 - микросхема готова 
//  1 - микросхема занята  внутренней операцией
#define U_1636RR52_SR_RDY_BSY 0x01 

// 1 Бит состояния доступа в микросхему (R)
//  0 - микросхема  не доступна  для записи (по умолчанию) 
//  1 - микросхема доступна для записи 
#define U_1636RR52_SR_WEL  0x02 

// 3:2 Состояние защищенности секторов  (R)
//  00 - Все сектора не защищены (состояние всех регистров защиты сектора логический ноль)  
//  01 - Некоторые сектора защищены. Чтение индивидуальных регистров защиты секторов позволяет определить, какие сектора защищены 
//  10 - Зарезервированы для будущего использования
//  11 - се сектора защищены (состояние всех регистров защиты секторов логическая единица – по умолчанию) 
#define U_1636RR52_SR_SWP  0x0C 

// 5 Ошибка стирания/записи  (R)
//  0 - Операция стирания или записи завершилась успешно 
//  1 - Обнаружена ошибка во время операции записи или стирания 
#define U_1636RR52_SR_EPE  0x20 

// 6 Разрешение сброса (R/W)
//  0 - Команда сброса запрещена (по умолчанию) 
//  1 - Команда сброса разрешена 
#define U_1636RR52_SR_RSTE 0x40 

// 7 Блокировка регистров защиты сектора   (R/W)
//  0 - Регистры защиты сектора не заблокированы (по умолчанию)  
//  1 - Регистры защиты сектора заблокированы 
#define U_1636RR52_SR_SPRL 0x80 

//test
extern uint8_t read_bite[4];

// Инициализация
uint32_t U_1636RR52_Init (void);

// **** Команды *****
// Чтение 4 байт данных, начиная с заданного адреса, на частоте до 15 МГц
uint32_t U_1636RR52_Read_Word (uint32_t addr);
// Чтение массива данных на частоте до 15 МГц
uint32_t U_1636RR52_Read_Array_15 (uint32_t addr, uint8_t* dst, uint32_t Count);
// Чтение массива данных на частоте до 100 МГц
uint32_t U_1636RR52_Read_Array_100 (uint32_t addr, uint8_t* dst, uint32_t Count);
// Стирание сектора 
uint32_t U_1636RR52_Sector_Erase (uint32_t Sector);
// Стирание всей памяти  
uint32_t U_1636RR52_Chip_Erase (void);
// Программирование байта 
void U_1636RR52_Byte_Program (uint32_t addr, uint8_t value);
// Программирование блока данных 
void U_1636RR52_Block_Program (uint32_t addr, uint8_t* Src, uint32_t Count);
// Разрешение записи 
void U_1636RR52_Write_Enable (void);
// Запрет записи  
void U_1636RR52_Write_Disable (void);
// Защита сектора
uint32_t U_1636RR52_Protect_Sector (uint32_t addr);
// Снятие защиты сектора
uint32_t U_1636RR52_Unprotect_Sector (uint32_t addr);
// Чтение регистра защиты сектора 
uint32_t U_1636RR52_Read_Sector_Protection_Register (uint32_t addr);
// Чтение регистра статуса
uint32_t U_1636RR52_Read_Status_Register (void);
// Запись регистра статуса 
void U_1636RR52_Write_Status_Register (uint8_t status);
// Сброс
void U_1636RR52_Reset (void);
// Чтение ID кодов производителя и микросхемы
uint32_t U_1636RR52_Read_ID (void);

// Test памяти
void test_flash (void);

#endif 


