#include "MKE16Z4.h"
#include <stdint.h>
#include "uart.h"
#include "queue.h"
#include "port.h"
#include "Flash.h"
#include "Handler_Data.h"
#include <core_cm0plus.h>


//#define T_SYSTICK (2000.0f / 1000)
#define APP_START_ADDRESS  0x4000
#define END_ADDRESS   0x1087F
#define SECTOR_SIZE   1024 // 1 KB
#define SHARED_VAR_ADDRESS (0x200017FE)
volatile uint8_t *share_value = (volatile uint8_t *)SHARED_VAR_ADDRESS;

void BootloaderMode(void);
void JumpToApplication(void);
uint8_t isAppValid(void);

Queue* q; // create a new queue value
SrecLine_t g_SrecLine;
State_t g_State = SREC_READ_RECORDTYPE;
uint8_t temp_data = 0;
uint8_t timer = 0;


int main(void) {
	//__enable_irq();

	//*share_value = 0x01;
    if (!isAppValid() || (*share_value == 0x01))
    {
    	q = createQueue();
		uart_config_t uartConfig = {
			.baudRate = 9600,
			.clockSrc = 48000000
		};

		UART_Init(LPUART0, &uartConfig);
		UART_EnableInterrupts(LPUART0);
		BootloaderMode();

    }
    else
    {
        JumpToApplication();
    }
}

uint8_t isAppValid(void) {
    uint8_t *appMemory = (uint8_t *)APP_START_ADDRESS;

    for (int i = 0; i < 16; i++) {
        if (appMemory[i] != 0xFF) {
            return 1;
        }
    }
    return 0;
}


void BootloaderMode(void) {
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

    for (uint32_t address = APP_START_ADDRESS; address <= END_ADDRESS; address += SECTOR_SIZE)
    {
			uint32_t sectorNum = address / SECTOR_SIZE;
			Flash_EraseSector(sectorNum);
	}

    while (1)
    {
        if (!isEmpty(q))
        {
            Handler_Data(&g_State, &g_SrecLine);
        }
    }

//    *share_value = 0x01;   // Reset giá trị chia sẻ
//    NVIC_SystemReset();    // Reset hệ thống để chuyển sang ứng dụng
}

void JumpToApplication(void) {
    // Tắt tất cả ngắt
    __disable_irq();

    // Cấu hình vector table của ứng dụng
    SCB->VTOR = APP_START_ADDRESS;

    // Lấy hàm reset handler của ứng dụng
    void (*app_reset_handler)(void) = (void (*)(void))(*((uint32_t *)(APP_START_ADDRESS + 0x04)));

    // Cấu hình lại MSP (Main Stack Pointer)
    __set_MSP(*((uint32_t *)APP_START_ADDRESS));

    // Nhảy vào ứng dụng
    app_reset_handler();
}

void LPUART0_IRQHandler(void) {
    uint8_t data = LPUART0->DATA;
    enqueue(q, data);
    //timer++;
}

//void SysTick_Handler(void)
//{
//	*share_value = 0x09; 	// reset share value application
//	NVIC_SystemReset();		// Reset system to jump to app
//}
