#include "MKE16Z4.h"
#include <stdint.h>
#include "uart.h"
#include "queue.h"
#include "port.h"
#include "Flash.h"
#include "Handler_Data.h"

#define APP_START_ADDRESS  0x4000
#define SHARED_VAR_ADDRESS (0x200017FE)
volatile uint8_t *share_value = (volatile uint8_t *)SHARED_VAR_ADDRESS;

void set_share_val(void);
void BootloaderMode(void);
void JumpToApplication(void);

Queue* q;		// create a new queue value


SrecLine_t g_SrecLine ;
State_t g_State = SREC_READ_RECORDTYPE;
uint8_t temp_data = 0;


int main(void) {

	//set_share_val();
    if (*share_value == 0x01)		// jump to boot mode
    {
    	q = createQueue();		// create a new queue
    	uart_config_t uartConfig = {
    		.baudRate = 9600,
    		.clockSrc = 48000000
    	};

        UART_Init(LPUART0, &uartConfig); 	// setup UART
        UART_EnableInterrupts(LPUART0);		// Turn on UART
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
        *share_value = 0x01;
    }
}

void BootloaderMode(void)
{
	port_config_t led1_config = {
		.port = PORTD,
		.pin = 1,
		.mux = PORT_MUX_GPIO,
		.direction = PORT_DIRECTION_OUTPUT,
		.pull = PORT_PULL_NONE,
		.interrupt = PORT_INTERRUPT_DISABLED,
	};
    port_init(&led1_config);
    turn_on_led(&led1_config);


	while(1) {
		if (!isEmpty(q))
		{
			Handler_Data(&g_State, &g_SrecLine);
		}
	}
	*share_value = 0x09; 	// reset share value application
	NVIC_SystemReset();		// Reset system to jump to app

}

void JumpToApplication(void)
{
    // turn off all IRQ
    __disable_irq();

    // configuring new vector table of application
    SCB->VTOR = APP_START_ADDRESS;

    // getting Reset Handler of application
    void (*app_reset_handler)(void) = (void (*)(void))(*((uint32_t *)(APP_START_ADDRESS + 0x04)));

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


