//#include "uart.h"
//
//#define UART_TIMEOUT_VALUE 100000
//#define FIRC_CLOCK 48000000 // Clock nguồn FIRC
//
//void UART_Init(LPUART_Type *base, uart_config_t *config)
//{
//    // Cấu hình nguồn clock FIRC
//    SCG->FIRCDIV &= ~SCG_FIRCDIV_FIRCDIV2_MASK;
//    SCG->FIRCDIV |= SCG_FIRCDIV_FIRCDIV2(1);
//
//    if (!(SCG->FIRCCSR & SCG_FIRCCSR_FIRCEN_MASK))
//    {
//        SCG->FIRCCSR |= SCG_FIRCCSR_FIRCEN_MASK;
//        while (!(SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK));
//    }
//
//    // Cấu hình clock cho UART
//    PCC->CLKCFG[PCC_LPUART0_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
//    PCC->CLKCFG[PCC_LPUART0_INDEX] &= ~PCC_CLKCFG_PCS_MASK;
//    PCC->CLKCFG[PCC_LPUART0_INDEX] |= PCC_CLKCFG_PCS(3);
//    PCC->CLKCFG[PCC_LPUART0_INDEX] |= PCC_CLKCFG_CGC(1);
//
//    // Cấu hình clock cho PORTB
//    PCC->CLKCFG[PCC_PORTB_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
//    PCC->CLKCFG[PCC_PORTB_INDEX] |= PCC_CLKCFG_CGC(1);
//
//    // Cấu hình chân UART (PORTB[0] và PORTB[1])
//    PORTB->PCR[1] = PORT_PCR_MUX(2); // TX
//    PORTB->PCR[0] = PORT_PCR_MUX(2); // RX
//
//    // Tắt bộ phát và thu để cấu hình
//    base->CTRL &= ~(LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);
//
//    // Tính toán thông số baud rate
//    uint32_t osr = 16; // Mặc định OSR = 16
//    uint16_t sbr = config->clockSrc / ((osr + 1) * config->baudRate);
//
//    if (sbr == 0 || sbr > 0x1FFF)
//    {
//        // Báo lỗi nếu giá trị SBR không hợp lệ
//        return;
//    }
//
//    base->BAUD &= ~(LPUART_BAUD_SBR_MASK | LPUART_BAUD_OSR_MASK);
//    base->BAUD |= LPUART_BAUD_SBR(sbr) | LPUART_BAUD_OSR(osr - 1);
//
//    // Kích hoạt bộ phát và thu
//    base->CTRL |= (LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);
//}
//
//uint8_t UART_ReadByte(LPUART_Type *base)
//{
//    uint32_t timeout = UART_TIMEOUT_VALUE;
//    while (!(base->STAT & LPUART_STAT_RDRF_MASK) && --timeout);
//    if (timeout == 0)
//    {
//        return 0xFF; // Lỗi timeout
//    }
//    return base->DATA;
//}
//
//uart_status_t UART_WriteByte(LPUART_Type *base, uint8_t data)
//{
//    uint32_t timeout = UART_TIMEOUT_VALUE;
//    while (!(base->STAT & LPUART_STAT_TC_MASK) && --timeout);
//    if (timeout == 0)
//    {
//        return UART_TIMEOUT;
//    }
//    base->DATA = data;
//    return UART_SUCCESS;
//}
//
//void UART_EnableInterrupts(LPUART_Type *base)
//{
//    base->CTRL |= LPUART_CTRL_RIE_MASK; // Bật ngắt nhận
//}
//
//void UART_DisableInterrupts(LPUART_Type *base)
//{
//    base->CTRL &= ~LPUART_CTRL_RIE_MASK; // Tắt ngắt nhận
//}
//
//void UART_SetBaudRate(LPUART_Type *base, uint32_t clockSrc, uint32_t baudRate)
//{
//    uint32_t osr = 16; // Mặc định OSR = 16
//    uint16_t sbr = clockSrc / ((osr + 1) * baudRate);
//
//    if (sbr == 0 || sbr > 0x1FFF)
//    {
//        // Báo lỗi nếu giá trị SBR không hợp lệ
//        return;
//    }
//
//    base->BAUD &= ~(LPUART_BAUD_SBR_MASK | LPUART_BAUD_OSR_MASK);
//    base->BAUD |= LPUART_BAUD_SBR(sbr) | LPUART_BAUD_OSR(osr - 1);
//}
//
//int main(void)
//{
//    uart_config_t uartConfig = {
//        .baudRate = 9600,
//        .clockSrc = FIRC_CLOCK
//    };
//
//    UART_Init(LPUART0, &uartConfig);
//
//    while (1)
//    {
//        if (LPUART0->STAT & LPUART_STAT_RDRF_MASK)
//        {
//            uint8_t data = UART_ReadByte(LPUART0);
//            if (data != 0xFF) // Kiểm tra không bị lỗi timeout
//            {
//                if (UART_WriteByte(LPUART0, data) == UART_TIMEOUT)
//                {
//
//                	// Xử lý lỗi timeout trong quá trình ghi dữ liệu
//                    break;
//                }
//            }
//        }
//    }
//
//    return 0;
//}
#include "uart.h"
#include <stdbool.h> // Thêm header để sử dụng bool, true, false

#define UART_TIMEOUT_VALUE 100000
#define FIRC_CLOCK 48000000 // Clock nguồn FIRC

volatile uint8_t uartReceivedData;
volatile bool uartDataAvailable = false;

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

    // Bật ngắt nhận dữ liệu
    base->CTRL |= LPUART_CTRL_RIE_MASK;
    NVIC_EnableIRQ(LPUART0_IRQn);
}

void LPUART0_IRQHandler(void)
{
    if (LPUART0->STAT & LPUART_STAT_RDRF_MASK) // Nếu có dữ liệu nhận
    {
        uartReceivedData = LPUART0->DATA; // Đọc dữ liệu
        uartDataAvailable = true;         // Đánh dấu dữ liệu đã sẵn sàng
        LPUART0->STAT |= LPUART_STAT_RDRF_MASK; // Xóa cờ ngắt nhận
    }
}

uart_status_t UART_WriteByte(LPUART_Type *base, uint8_t data)
{
    uint32_t timeout = UART_TIMEOUT_VALUE;
    while (!(base->STAT & LPUART_STAT_TC_MASK) && --timeout);
    if (timeout == 0)
    {
        return UART_TIMEOUT; // Giá trị timeout sửa lại để phù hợp với uart_status_t
    }
    base->DATA = data;
    return UART_SUCCESS;
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
        if (uartDataAvailable) // Kiểm tra nếu dữ liệu nhận đã sẵn sàng
        {
            uartDataAvailable = false; // Reset cờ
            if (UART_WriteByte(LPUART0, uartReceivedData) == UART_TIMEOUT)
            {
                // Xử lý lỗi timeout trong quá trình ghi dữ liệu
                break;
            }
        }
    }

    return 0;
}
