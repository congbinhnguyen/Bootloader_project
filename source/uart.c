#include "uart.h"


void UART_Init(LPUART_Type *base, uart_config_t *config) {

	SCG->FIRCDIV &= ~SCG_FIRCDIV_FIRCDIV2_MASK;
	SCG->FIRCDIV |= SCG_FIRCDIV_FIRCDIV2(1);

    // configuring PCC
    PCC->CLKCFG[PCC_LPUART0_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
    PCC->CLKCFG[PCC_LPUART0_INDEX] &= ~PCC_CLKCFG_PCS_MASK;  // Xóa nguồn clock hiện tại
    PCC->CLKCFG[PCC_LPUART0_INDEX] |= PCC_CLKCFG_PCS(3);     // Chọn LPFLL làm nguồn clock
    PCC->CLKCFG[PCC_LPUART0_INDEX] |= PCC_CLKCFG_CGC(1);   // Bật clock cho LPUART0

    // configuring PCC for PORT B
    PCC->CLKCFG[PCC_PORTB_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
    PCC->CLKCFG[PCC_PORTB_INDEX] |= PCC_CLKCFG_CGC(1);

    //PTB1 and PTB0 (LPUART0_TX)
    PORTB->PCR[1] = PORT_PCR_MUX(2);
    PORTB->PCR[0] = PORT_PCR_MUX(2);


    // turn of TE and RE
    base->CTRL &= ~(LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);

    // configuring SBR
    uint32_t osr = 16;
    uint16_t sbr = config->clockSrc / ((osr + 1) * config->baudRate);

    // write BAUD
    base->BAUD &= ~LPUART_BAUD_SBR_MASK;
    base->BAUD |= LPUART_BAUD_SBR(sbr);
    base->BAUD &= ~LPUART_BAUD_OSR_MASK;
    base->BAUD |= LPUART_BAUD_OSR(osr-1);

    base->CTRL |= LPUART_CTRL_RE_MASK;
}


uart_status_t UART_WriteByte(LPUART_Type *base, uint8_t data) {

	base->CTRL |= LPUART_CTRL_TE_MASK;
	base->DATA = data;
	while(!(base->STAT & LPUART_STAT_TC_MASK))
	{

	}
    base->CTRL &= ~LPUART_CTRL_TE_MASK;
    return UART_SUCCESS;
}

uint8_t UART_ReadByte(LPUART_Type *base) {
	base->CTRL |= LPUART_CTRL_RE_MASK;
    while (!(base->STAT & LPUART_STAT_RDRF_MASK));
    base->CTRL &= ~LPUART_CTRL_RE_MASK;
    return base->DATA;
}

void UART_EnableInterrupts(LPUART_Type *base) {
    base->CTRL |= LPUART_CTRL_RIE_MASK;
    NVIC_EnableIRQ(LPUART0_IRQn);
}

void UART_DisableInterrupts(LPUART_Type *base) {
    base->CTRL &= ~LPUART_CTRL_RIE_MASK;
    NVIC_DisableIRQ(LPUART0_IRQn);
}
