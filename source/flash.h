/*
 * flash.h
 *
 *  Created on: Dec 25, 2024
 *      Author: Administrator
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "MKE16Z4.h"

void FLASH_Write(uint32_t Address, uint32_t Data);
void FLASH_Erase(uint32_t Address);
void Flash_EraseSector(uint32_t SectorNum);
uint32_t Flash_Read(uint32_t Address);

#endif /* FLASH_H_ */
