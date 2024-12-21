/*
 * KE16_Flash.h
 *
 *  Created on: Dec 21, 2024
 *      Author: sh124
 */

/*
 * KE16_Flash.h
 *
 *  Created on: Dec 19, 2024
 *      Author: ADMIN
 */

#ifndef KE16_FLASH_H_
#define KE16_FLASH_H_

#include "MKE16Z4.h"

void FLASH_Write(uint32_t Address, uint32_t Data);
void FLASH_Erase(uint32_t Address);
void Flash_EraseSector(uint32_t SectorNum);
uint32_t Flash_Read(uint32_t Address);

#endif /* KE16_FLASH_H_ */
