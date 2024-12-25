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

    // set the interrupt
    if (config->interrupt == PORT_INTERRUPT_ENABLED)
    {
        config->port->ISFR |= (1 << config->pin);
    }
    else
    {
        config->port->ISFR &= ~(1 << config->pin);
    }
}
