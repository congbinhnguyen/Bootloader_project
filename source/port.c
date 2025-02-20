#include "port.h"

void port_init(port_config_t *config)
{
    // enable the clock
    switch ((uint32_t)config->port)
    {
    case (uint32_t)PORTA:
        PCC->CLKCFG[PCC_PORTA_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
        PCC->CLKCFG[PCC_PORTA_INDEX] |= PCC_CLKCFG_CGC(1);
        break;

    case (uint32_t)PORTB:
        PCC->CLKCFG[PCC_PORTB_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
        PCC->CLKCFG[PCC_PORTB_INDEX] |= PCC_CLKCFG_CGC(1);
        break;

    case (uint32_t)PORTC:
        PCC->CLKCFG[PCC_PORTC_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
        PCC->CLKCFG[PCC_PORTC_INDEX] |= PCC_CLKCFG_CGC(1);
        break;

    case (uint32_t)PORTD:
        PCC->CLKCFG[PCC_PORTD_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
        PCC->CLKCFG[PCC_PORTD_INDEX] |= PCC_CLKCFG_CGC(1);
        break;

    case (uint32_t)PORTE:
        PCC->CLKCFG[PCC_PORTE_INDEX] &= ~PCC_CLKCFG_CGC_MASK;
        PCC->CLKCFG[PCC_PORTE_INDEX] |= PCC_CLKCFG_CGC(1);
        break;
    }

    // set the mux
    config->port->PCR[config->pin] &= ~PORT_PCR_MUX_MASK;
    config->port->PCR[config->pin] |= PORT_PCR_MUX(config->mux);

    // set the direction
    if (config->direction == PORT_DIRECTION_OUTPUT)
    {
        switch ((uint32_t)config->port)
        {

        case (uint32_t)PORTA:
            GPIOA->PDDR &= ~GPIO_PDDR_PDD_MASK;
            GPIOA->PDDR |= (1 << config->pin);
            break;

        case (uint32_t)PORTB:
            GPIOB->PDDR &= ~GPIO_PDDR_PDD_MASK;
            GPIOB->PDDR |= (1 << config->pin);
            break;

        case (uint32_t)PORTC:
            GPIOC->PDDR &= ~GPIO_PDDR_PDD_MASK;
            GPIOC->PDDR |= (1 << config->pin);
            break;

        case (uint32_t)PORTD:
            GPIOD->PDDR &= ~GPIO_PDDR_PDD_MASK;
            GPIOD->PDDR |= (1 << config->pin);
            break;

        case (uint32_t)PORTE:
            GPIOE->PDDR &= ~GPIO_PDDR_PDD_MASK;
            GPIOE->PDDR |= (1 << config->pin);
            break;
        }
    }
    else
    {
        switch ((uint32_t)config->port)
        {

        case (uint32_t)PORTA:
            GPIOA->PDDR &= (0 << config->pin);
            break;

        case (uint32_t)PORTB:
            GPIOB->PDDR &= (0 << config->pin);
            break;

        case (uint32_t)PORTC:
            GPIOC->PDDR &= (0 << config->pin);
            break;

        case (uint32_t)PORTD:
            GPIOD->PDDR &= (0 << config->pin);
            break;

        case (uint32_t)PORTE:
            GPIOE->PDDR &= (0 << config->pin);
            break;
        }
    }

    // set the pull
    config->port->PCR[config->pin] &= ~PORT_PCR_PE_MASK;
    config->port->PCR[config->pin] |= PORT_PCR_PE(config->pull);


    // set interrupt
    if (config->interrupt == PORT_INTERRUPT_ENABLED)
    {
        // Clear any pending interrupt flags for this pin
        config->port->ISFR |= (1 << config->pin);

        // Enable interrupt for the specific pin and set edge type
        config->port->PCR[config->pin] &= ~PORT_PCR_IRQC_MASK; // Clear existing settings
        config->port->PCR[config->pin] |= PORT_PCR_IRQC(0b1010); // Configure for falling edge

        // Enable NVIC interrupt for the corresponding PORT
        NVIC_EnableIRQ(PORTBCD_IRQn);
    }
    else
    {
        config->port->PCR[config->pin] &= ~PORT_PCR_IRQC_MASK; // Disable interrupt
        config->port->ISFR &= ~(1 << config->pin);
    }
}

void turn_off_led(port_config_t *config)
{
	switch ((uint32_t)config->port)
	{

	case (uint32_t)PORTA:
		GPIOA->PDOR &= ~GPIO_PDOR_PDO_MASK;
		GPIOA->PDOR |= (1 << config->pin);
		break;

	case (uint32_t)PORTB:
		GPIOB->PDOR &= ~GPIO_PDOR_PDO_MASK;
		GPIOB->PDOR |= (1 << config->pin);
		break;

	case (uint32_t)PORTC:
		GPIOC->PDOR &= ~GPIO_PDOR_PDO_MASK;
		GPIOC->PDOR |= (1 << config->pin);
		break;

	case (uint32_t)PORTD:
		GPIOD->PDOR &= ~GPIO_PDOR_PDO_MASK;
		GPIOD->PDOR |= (1 << config->pin);
		break;

	case (uint32_t)PORTE:
		GPIOE->PDOR &= ~GPIO_PDOR_PDO_MASK;
		GPIOE->PDOR |= (1 << config->pin);
		break;
	}
}
void turn_on_led(port_config_t *config)
{
	switch ((uint32_t)config->port)
	{

	case (uint32_t)PORTA:
		GPIOA->PDOR &= ~(1 << config->pin);
		break;

	case (uint32_t)PORTB:
		GPIOB->PDOR &= ~(1 << config->pin);
		break;

	case (uint32_t)PORTC:
		GPIOC->PDOR &= ~(1 << config->pin);
		break;

	case (uint32_t)PORTD:
		GPIOD->PDOR &= ~(1 << config->pin);
		break;

	case (uint32_t)PORTE:
		GPIOE->PDOR &= ~(1 << config->pin);
		break;
	}
}
