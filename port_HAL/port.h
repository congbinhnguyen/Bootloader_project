/*
 * port.h
 *
 *  Created on: Dec 17, 2024
 *      Author: binhc
 */

#ifndef PORT_H_
#define PORT_H_

#include "MKE16Z4.h"
#include <stdint.h>

// define the feature of the port
typedef enum {
    PORT_MUX_DISABLED = 0, // default
    PORT_MUX_GPIO     = 1,
} port_mux_t;


// input/output
typedef enum {
    PORT_DIRECTION_INPUT  = 0,
    PORT_DIRECTION_OUTPUT = 1,
} port_direction_t;


// define the pull-up/pull-down
typedef enum {
    PORT_PULL_NONE = 0, // no pull
    PORT_PULL_UP   = 1, // Pull-up
    PORT_PULL_DOWN = 2, // Pull-down
} port_pull_t;


// interrupt
typedef enum {
    PORT_INTERRUPT_DISABLED = 0,
    PORT_INTERRUPT_ENABLED  = 1,
} port_interrupt_t;


typedef struct {
    PORT_Type * port;
    uint32_t pin;
    port_mux_t mux;
    port_direction_t direction;
    port_pull_t pull;
    port_interrupt_t interrupt;
}port_config_t;



// error code
typedef enum {
    PORT_SUCCESS = 0,
    PORT_ERROR   = 1,
} port_error_t;


// function prototype
void port_init(port_config_t *config);



#endif
