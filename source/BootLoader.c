#include <stdint.h>
#include <math.h>
#include "queue.h"
#include "Handler_Data.h"
#include "MKE16Z4.h"
#include "UART.h"

Queue* q;

SrecLine_t g_SrecLine ;
State_t g_State = SREC_READ_RECORDTYPE;

uint8_t temp_data = 0;

int main(void) {

	uart_config_t uartConfig = {
		.baudRate = 9600,
		.clockSrc = 48000000
	};

	UART_Init(LPUART0, &uartConfig);
	q = createQueue();

	UART_EnableInterrupts(LPUART0);


	while(1) {
		if (!isEmpty(q)) {
			Handler_Data(&g_State, &g_SrecLine);
		}

	}
}

void LPUART0_IRQHandler(void) {

//	temp_data = UART_ReadByte(LPUART0);
//	while(1);
	uint8_t data = LPUART0->DATA;
	enqueue(q, data);
}
