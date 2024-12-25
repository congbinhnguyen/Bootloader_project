#include "MKE16Z4.h"
#include <stdint.h>
#include "uart.h"
#include "queue.h"

#define APP_START_ADDRESS  0x2000
#define SHARED_VAR_ADDRESS (0x200017F0)
volatile uint32_t *share_value = (volatile uint32_t *)SHARED_VAR_ADDRESS;

void set_share_val(void);
void BootloaderMode(void);
void JumpToApplication(void);

Queue* q;		// create a new queue value

int main(void) {

	q = createQueue();		// create a new queue

	uart_config_t uartConfig = {
		.baudRate = 9600,
		.clockSrc = 48000000
	};

    UART_Init(LPUART0, &uartConfig); 	// setup UART
    UART_EnableInterrupts(LPUART0);		// Turn on UART
	set_share_val();

    if (*share_value == 0x01)
    {
        BootloaderMode();				// boot mode
    }
    else
    {
        JumpToApplication();			// app mode
    }
}

void set_share_val(void)
{
    if (*share_value != 0x01)
    {
        *share_value = 0x00;
    }
}

void BootloaderMode(void)
{
	// Data_filtering and write data in flash
	//.......

	// if done or EOF -> JumpToApplication
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
	uint8_t data = LPUART0->DATA;
	enqueue(q, data);
}
