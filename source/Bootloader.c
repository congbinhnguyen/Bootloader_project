#include "MKE16Z4.h"
#include "uart.h"
#include "queue.h"
#define APP_START_ADDRESS  0x2000
#define SHARED_VAR_ADDRESS (0x200017F0)
volatile uint32_t *shared_var = (volatile uint32_t *)SHARED_VAR_ADDRESS;

void set_share_val(void);
void BootloaderMode(void);
void JumpToApplication(void);


uart_config_t uartConfig = {
	.baudRate = 9600,
	.clockSrc = 48000000 // Clock nguồn
};





int main(void) {


	set_share_val();

    if (*shared_var == 0x01)
    {
        BootloaderMode();
    }
    else
    {
        JumpToApplication();
    }

    while (1);
}


void set_share_val(void)
{
    if (*shared_var != 0x01)
    {
        *shared_var = 0x00;
    }
}

void BootloaderMode(void)
{
    UART_Init(LPUART0, &uartConfig);
    UART_EnableInterrupts(LPUART0);

    // Data filtering


    // Data writting
}

void JumpToApplication(void)
{
    // turn off all IRQ
    __disable_irq();

    // configuring new vector table of application
    SCB->VTOR = APP_START_ADDRESS;

    // getting Reset Handler of application
    void (*app_reset_handler)(void) = (void (*)(void))(*((uint32_t *)(APP_START_ADDRESS + 4)));

    // configuring again MSP (Main Stack Pointer)
    __set_MSP(*((uint32_t *)APP_START_ADDRESS));

    // jump to application
    app_reset_handler();
}

void LPUART0_IRQHandler(void)
{
	while(1);
}
