/*
 * UART_FLASH.c
 *
 *  Created on: Dec 21, 2024
 *      Author: sh124
 */
#include "uart.h"
#include "KE16_Flash.h"
#include "uart.h"

#define UART_TIMEOUT_VALUE 100000
#define FIRC_CLOCK 48000000 // Clock nguồn FIRC

#define FTFA_FSTAT_CLEAR_ERROR          0x30U
#define START_COMMAND                   0x80U
#define CMD_PROGRAM_LONGWORD            0x06U
#define CMD_ERASE_SECTOR               	0x09U

void UART_Init(LPUART_Type *base, uart_config_t *config)
{
    // Cấu hình nguồn clock FIRC
    SCG->FIRCDIV &= ~SCG_FIRCDIV_FIRCDIV2_MASK;
    SCG->FIRCDIV |= SCG_FIRCDIV_FIRCDIV2(1);

    if (!(SCG->FIRCCSR & SCG_FIRCCSR_FIRCEN_MASK))
    {
        SCG->FIRCCSR |= SCG_FIRCCSR_FIRCEN_MASK;
        while (!(SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK));
    }

    // Cấu hình clock cho UART
    PCC->CLKCFG[PCC_LPUART0_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
    PCC->CLKCFG[PCC_LPUART0_INDEX] &= ~PCC_CLKCFG_PCS_MASK;
    PCC->CLKCFG[PCC_LPUART0_INDEX] |= PCC_CLKCFG_PCS(3);
    PCC->CLKCFG[PCC_LPUART0_INDEX] |= PCC_CLKCFG_CGC(1);

    // Cấu hình clock cho PORTB
    PCC->CLKCFG[PCC_PORTB_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
    PCC->CLKCFG[PCC_PORTB_INDEX] |= PCC_CLKCFG_CGC(1);

    // Cấu hình chân UART (PORTB[0] và PORTB[1])
    PORTB->PCR[1] = PORT_PCR_MUX(2); // TX
    PORTB->PCR[0] = PORT_PCR_MUX(2); // RX

    // Tắt bộ phát và thu để cấu hình
    base->CTRL &= ~(LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);

    // Tính toán thông số baud rate
    uint32_t osr = 16; // Mặc định OSR = 16
    uint16_t sbr = config->clockSrc / ((osr + 1) * config->baudRate);

    if (sbr == 0 || sbr > 0x1FFF)
    {
        // Báo lỗi nếu giá trị SBR không hợp lệ
        return;
    }

    base->BAUD &= ~(LPUART_BAUD_SBR_MASK | LPUART_BAUD_OSR_MASK);
    base->BAUD |= LPUART_BAUD_SBR(sbr) | LPUART_BAUD_OSR(osr - 1);

    // Kích hoạt bộ phát và thu
    base->CTRL |= (LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);
}

uint8_t UART_ReadByte(LPUART_Type *base)
{
    uint32_t timeout = UART_TIMEOUT_VALUE;
    while (!(base->STAT & LPUART_STAT_RDRF_MASK) && --timeout);
    if (timeout == 0)
    {
        return 0xFF; // Lỗi timeout
    }
    return base->DATA;
}

uart_status_t UART_WriteByte(LPUART_Type *base, uint8_t data)
{
    uint32_t timeout = UART_TIMEOUT_VALUE;
    while (!(base->STAT & LPUART_STAT_TC_MASK) && --timeout);
    if (timeout == 0)
    {
        return UART_TIMEOUT;
    }
    base->DATA = data;
    return UART_SUCCESS;
}

void UART_EnableInterrupts(LPUART_Type *base)
{
    base->CTRL |= LPUART_CTRL_RIE_MASK; // Bật ngắt nhận
}

void UART_DisableInterrupts(LPUART_Type *base)
{
    base->CTRL &= ~LPUART_CTRL_RIE_MASK; // Tắt ngắt nhận
}

void UART_SetBaudRate(LPUART_Type *base, uint32_t clockSrc, uint32_t baudRate)
{
    uint32_t osr = 16; // Mặc định OSR = 16
    uint16_t sbr = clockSrc / ((osr + 1) * baudRate);

    if (sbr == 0 || sbr > 0x1FFF)
    {
        // Báo lỗi nếu giá trị SBR không hợp lệ
        return;
    }

    base->BAUD &= ~(LPUART_BAUD_SBR_MASK | LPUART_BAUD_OSR_MASK);
    base->BAUD |= LPUART_BAUD_SBR(sbr) | LPUART_BAUD_OSR(osr - 1);
}
void FLASH_Write(uint32_t Address, uint32_t Data)
{
    /* Wait Previous Command Complete? */
    while ((FTFA->FSTAT & FTFA_FSTAT_CCIF_MASK) == 0);

    /* Check Error from Previous Command */
    if ( ((FTFA->FSTAT & FTFA_FSTAT_ACCERR_MASK) == FTFA_FSTAT_ACCERR_MASK) \
        || ((FTFA->FSTAT & FTFA_FSTAT_FPVIOL_MASK) == FTFA_FSTAT_FPVIOL_MASK) )
    {
        FTFA->FSTAT = FTFA_FSTAT_CLEAR_ERROR;
    }

    /*write command and parameter to FCCOB */
    FTFA->FCCOB0 = CMD_PROGRAM_LONGWORD;
    /*set address*/
    FTFA->FCCOB1 = (uint8_t) (Address >>16);
    FTFA->FCCOB2 = (uint8_t) (Address >>8);
    FTFA->FCCOB3 = (uint8_t) (Address);

    /*set data*/
    FTFA->FCCOB4 = (uint8_t) (Data >> 24);
    FTFA->FCCOB5 = (uint8_t) (Data >> 16);
    FTFA->FCCOB6 = (uint8_t) (Data >> 8);
    FTFA->FCCOB7 = (uint8_t) (Data);

    /*clear CCIF Flag to launch command */
    FTFA->FSTAT = START_COMMAND;
}

void FLASH_Erase(uint32_t Address)
{
	if(Address < 0x800UL)
	{
		return;
	}

   /* wait previous command complate*/
  while ((FTFA->FSTAT & FTFA_FSTAT_CCIF_MASK) == 0);

  /*write command and parameter to FCCOB0*/
  FTFA->FCCOB0 = CMD_ERASE_SECTOR ;
  /*set address*/
  FTFA->FCCOB1 = (uint8_t) (Address >>16);
  FTFA->FCCOB2 = (uint8_t) (Address >>8);
  FTFA->FCCOB3 = (uint8_t) (Address);

  //clear Clear CCIF;
  FTFA->FSTAT = START_COMMAND;
}

void Flash_EraseSector(uint32_t SectorNum)
{
	if(0 == SectorNum || 1 == SectorNum)
	{
		return;
	}

	uint32_t Address = SectorNum * 1024;
	Flash_Erase(Address);
}

uint32_t Flash_Read(uint32_t Address)
{
	return *(uint32_t*)Address;
}
int main(void)
{

    uart_config_t uartConfig = {
        .baudRate = 9600,
        .clockSrc = FIRC_CLOCK
    };

    UART_Init(LPUART0, &uartConfig);

    while (1)
    {
        if (LPUART0->STAT & LPUART_STAT_RDRF_MASK)
        {
            uint8_t data = UART_ReadByte(LPUART0);
            if (data != 0xFF) // ktra lỗi timeout
            {
                if (UART_WriteByte(LPUART0, data) == UART_TIMEOUT)
                {

                    break;
                }
            }
        }
    }

    return 0;
}
