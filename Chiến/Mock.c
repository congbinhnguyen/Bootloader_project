#include <stdint.h>
#include <math.h>

#include "MKE16Z4.h"
// Middleware
#define LED_GREEN_PORT 	PORTD
#define LED_GREEN_PIN	4
#define LED_RED_PORT	PORTD
#define	LED_RED_PIN		5



volatile uint16_t LPIT_ISR_Flag = 0;
volatile uint16_t UART0_ISR_Flag = 0;
volatile uint8_t temp_data = 0;

void UART_Init() {
	// 1. Enable Clock Peripheral UART
	/*Config fast IRC Clock Devide */
	SCG->FIRCDIV &= ~SCG_FIRCDIV_FIRCDIV2_MASK;
	SCG->FIRCDIV |= SCG_FIRCDIV_FIRCDIV2(1);

    if (!(SCG->FIRCCSR & SCG_FIRCCSR_FIRCEN_MASK))
    {
        SCG->FIRCCSR |= SCG_FIRCCSR_FIRCEN_MASK;
        while (!(SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK));
    }

	/*Config Clock Source Select */
	PCC->CLKCFG[PCC_LPUART0_INDEX] &= ~PCC_CLKCFG_PCS_MASK;
	PCC->CLKCFG[PCC_LPUART0_INDEX] |= PCC_CLKCFG_PCS(3);

	/*Enable Clock UART */
	PCC->CLKCFG[PCC_LPUART0_INDEX] |= PCC_CLKCFG_CGC(1);


	//2. Config Mode TX/RX Pin
	/*Enclock PortB / PIN0-RX / PIN1-TX */
	PCC->CLKCFG[PCC_PORTB_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
	PCC->CLKCFG[PCC_PORTB_INDEX] |= PCC_CLKCFG_CGC(1);

	/*Config PIN0-RX */
	PORTB->PCR[0] |= PORT_PCR_MUX(2);

	/*Config PIN1-TX */
	PORTB->PCR[1] |= PORT_PCR_MUX(2);

	//3. Config UART
	/*Reset all internal logic and registers, except the Global Register */

	/* Disable LPUART TX RX before setting. */
	LPUART0->CTRL &= ~(LPUART_CTRL_RE_MASK | LPUART_CTRL_TE_MASK);

	/*Config Word Length*/
	LPUART0->CTRL &= ~(LPUART_CTRL_M_MASK);

    /* Enable tx/rx FIFO */
	LPUART0->FIFO |= (LPUART_FIFO_TXFE_MASK | LPUART_FIFO_RXFE_MASK);

	/*Congif Baudrate*/
	uint32_t osr = 16; // Mặc định OSR = 16
	uint32_t sbr = 48000000 / ((16 + 1) * 9600);
	LPUART0->BAUD &= ~(LPUART_BAUD_SBR_MASK);
	LPUART0->BAUD |= LPUART_BAUD_SBR(sbr);

	LPUART0->BAUD |= LPUART_BAUD_OSR(osr - 1);
	/* Config Interrupt */
	LPUART0->CTRL |= (LPUART_CTRL_RIE(1));
	NVIC_EnableIRQ(LPUART0_IRQn);

	LPUART0->CTRL |= LPUART_CTRL_RE_MASK;
}


void UART_WriteByte(LPUART_Type *base, uint8_t data)
{

    //	uint32_t timeout;
    base->CTRL |= LPUART_CTRL_TE_MASK;
    base->DATA = data;
    while (!(base->STAT & LPUART_STAT_TC_MASK))
    {
    }
    base->CTRL &= ~LPUART_CTRL_TE_MASK;
}

uint8_t UART_ReadByte(LPUART_Type *base)
{
    // Chờ trạng thái RDRF (Receive Data Register Full)
    base->CTRL |= LPUART_CTRL_RE_MASK;

    while (!(base->STAT & LPUART_STAT_RDRF_MASK));

    base->CTRL &= ~LPUART_CTRL_RE_MASK;
    return base->DATA;
}
// Application - Middler-ware - MCAL

typedef struct  {
	uint8_t u8SrecType;		// 0x00 -> 0x09
	uint8_t u8ByteCount;
	uint32_t u32Address;
	uint8_t *pData;
	uint8_t u8DataLen;
	uint8_t u8CheckSum;
	uint32_t u32Data;
} SrecLine_t;

SrecLine_t SrecLine;

typedef enum {

	SREC_READ_RECORDTYPE,
	SREC_READ_BYTECOUNT,
	SREC_READ_ADDRESS,
	SREC_READ_DATA,
	SREC_READ_CHECKSUM,
	HEX_END_OF_LINE,
	HEX_DONE,
	SREC_READ_ERROR,
} State_t;

State_t State = SREC_READ_RECORDTYPE;

uint32_t charToHex(char CTH) {
	if ('0' <= CTH && CTH <= '9') return CTH - '0';
	else if ('A' <= CTH && CTH <= 'F') return CTH - 'A' + 10;
	return 0;
}
int main(void) {

//	// TIMER - LPIT
//	//	- CLOCK
//	// 	- Generate Periodic Interrupt
//	//	- Start: 3... Software, Trigger form anther module,
//
//	ADC_Init();
//
//	LPIT_Init();

//	TRGMUX0->TRGCFG[TRGMUX_ADC0_INDEX] |= TRGMUX_TRGCFG_SEL0(7);
//
//	// Hardware Trigger 	LPIT Trigger ADC -> ADC Start
//	SIM->ADCOPT |= SIM_ADCOPT_ADC0TRGSEL(1);	// TRGMUX OUTPUT
//	SIM->ADCOPT |= SIM_ADCOPT_ADC0PRETRGSEL(1U);
//	ADC_Init();

//	LPIT_StartTimer();

	PCC->CLKCFG[PCC_PORTB_INDEX] |= (1 << 30);
	PORTB->PCR[4] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[4] |= PORT_PCR_MUX(1);

	GPIOB->PDDR |= (1 << 4);		// Set Output
	GPIOB->PSOR |= (1 << 4);		// Set bit 4 - 1
//
//	uint16_t ADC_Value;

	UART_Init();

	int count = 0;
	uint8_t numAddr = 0;
	uint8_t Buffer_Data[64] = {0};
	while (1) {

		if (UART0_ISR_Flag == 1) {
			switch(State)
			{
				case SREC_READ_RECORDTYPE:
					count++;
//					temp_data = charToHex(temp_data);
					if (count == 1) {
						if (temp_data != 0x53){
							State = SREC_READ_ERROR;
						}
					}

					if (count == 2 ){
						temp_data = charToHex(temp_data);
						SrecLine.u8SrecType = temp_data;
						State = SREC_READ_BYTECOUNT;
						count = 0;

						if (SrecLine.u8SrecType == 8) numAddr = 2 * 3;
						else if (SrecLine.u8SrecType == 9) numAddr = 2 * 2;
						else if (SrecLine.u8SrecType == 7) numAddr = 2 * 4;
						else numAddr = 2 * (SrecLine.u8SrecType + 1);
					}

					break;
				case SREC_READ_BYTECOUNT:
					count++;
					temp_data = charToHex(temp_data);
					SrecLine.u8ByteCount +=  temp_data * pow(16, 2 - count);

					if (count == 2) {
						State = SREC_READ_ADDRESS;
						count = 0;
					}
					break;

				case SREC_READ_ADDRESS:
					count++;
					temp_data = charToHex(temp_data);
					SrecLine.u32Address +=  temp_data * pow(16, numAddr - count);

					if (count == numAddr) {

						State = SREC_READ_DATA;
						SrecLine.u8DataLen = SrecLine.u8ByteCount - 1 - (SrecLine.u8SrecType + 1);
						count = 0;
						break;
					}
					break;
				case SREC_READ_DATA:
					count++;
					if (SrecLine.u8SrecType == 7 || SrecLine.u8SrecType == 8 || SrecLine.u8SrecType == 7) {
						State = SREC_READ_CHECKSUM;
						break;
					}

					temp_data = charToHex(temp_data);
					Buffer_Data[count - 1] = temp_data;
					if (count == (2*SrecLine.u8DataLen)) {

						State = SREC_READ_CHECKSUM;
						count = 0;
						break;
					}

					break;

				case SREC_READ_CHECKSUM:
					count++;
					temp_data = charToHex(temp_data);
					SrecLine.u8CheckSum += temp_data * pow(16, 2 - count);
					if (count == 2) {

						State = SREC_READ_RECORDTYPE;
						count = 0;
						break;
					}
					break;

				case SREC_READ_ERROR:
					State = SREC_READ_RECORDTYPE;
					count = 0;
					break;

				default:
					State = SREC_READ_RECORDTYPE;
					break;
			}
			// Toggle Led
			GPIOB->PTOR |= (1 << LED_GREEN_PIN);

			UART0_ISR_Flag = 0;
		}

//		while ((ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0);

			// Set bit 4 - 1

//
//		if (LPIT_ISR_Flag == 1) {
//
//			// Toggle Led
//			GPIOD->PTOR |= (1 << LED_GREEN_PIN);
//
//			LPIT_ISR_Flag = 0;
//		}
//		while ((ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0);
//
//		ADC_Value *= 2;
//		GPIOB->PSOR |= (1 << 4);		// Set bit 4 - 1
	}
}

void LPUART0_IRQHandler(void) {

//	temp_data = UART_ReadByte(LPUART0);
//	while(1);
	temp_data = (uint8_t)LPUART0->DATA;
	UART0_ISR_Flag = 1;
}

void LPIT0_IRQHandler(void) {

	// Clear Interrupt
	uint32_t temp = ADC0->SC1[0];		//Read
	temp &= ~ADC_SC1_ADCH_MASK;			// Modify
	temp |= (ADC_SC1_ADCH(26));

	ADC0->SC1[0] = temp;			// Write

	// Start Read Sensor
}

void ADC0_IRQHandler(void) {
//	while ((ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0);

	LPIT_ISR_Flag = ADC0->R[0];
	GPIOB->PCOR |= (1 << 4);		// Set bit 4 - 1

//	GPIO_PortToggle(GPIOD, LED_GREEN_PIN);
}
