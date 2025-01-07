/*
 * Handler_Data.h
 *
 *  Created on: Dec 25, 2024
 *      Author: Administrator
 */

#ifndef HANDLER_DATA_H_
#define HANDLER_DATA_H_

#include <stdint.h>
#include "Handler_Data.h"
#include "queue.h"
#include "math.h"
#include "Flash.h"

typedef struct  {
	uint8_t u8SrecType;		// 0x00 -> 0x09
	uint8_t u8ByteCount;
	uint32_t u32Address;
	uint8_t *pData;
	uint8_t u8DataLen;
	uint8_t u8CheckSum;
} SrecLine_t;


typedef enum {

	//SREC_READ_START,
	SREC_READ_RECORDTYPE,
	SREC_IGNORE_S0,
	SREC_READ_BYTECOUNT,
	SREC_READ_ADDRESS,
	SREC_READ_DATA,
	SREC_READ_CHECKSUM,
	SREC_END_OF_LINE,
	SREC_READ_ERROR,
} State_t;


void Handler_Data(State_t *State, SrecLine_t *SrecLine);
uint32_t charToHex(char CTH);

#endif /* HANDLER_DATA_H_ */
