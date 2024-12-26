
#include "Handler_Data.h"
#include <string.h>
extern Queue* q;

extern uint8_t temp_data;

extern volatile uint8_t *share_value;

void Handler_Data(State_t *State, SrecLine_t *SrecLine) {

	static uint8_t count = 0;
	static uint8_t numAddr = 0;
	static uint8_t Buffer_Data[64] = {0};
	static uint32_t Sum_Data = 0;

	temp_data = dequeue(q);

	switch(*State)
	{
		case SREC_READ_RECORDTYPE:
			count++;
//			temp_data = charToHex(temp_data);
			if (count == 1) {
				if (temp_data != 0x53){
 					*State = SREC_READ_ERROR;
				}
			}

			if ((count == 2)){
				temp_data = charToHex(temp_data);
				SrecLine->u8SrecType = temp_data;
				*State = SREC_READ_BYTECOUNT;
				count = 0;

				if (SrecLine->u8SrecType == 8) numAddr = 2 * 3;
				else if (SrecLine->u8SrecType == 9) numAddr = 2 * 2;
				else if (SrecLine->u8SrecType == 7) numAddr = 2 * 4;
				else numAddr = 2 * (SrecLine->u8SrecType + 1);
			}
			break;

		case SREC_READ_BYTECOUNT:
			count++;
			temp_data = charToHex(temp_data);
			SrecLine->u8ByteCount +=  temp_data * pow(16, 2 - count);

			if (count == 2) {
				*State = SREC_READ_ADDRESS;
				count = 0;
			}
			break;

		case SREC_READ_ADDRESS:
			count++;
			temp_data = charToHex(temp_data);
			SrecLine->u32Address = (SrecLine->u32Address << 4) | temp_data;

			if (count == numAddr) {

				*State = SREC_READ_DATA;
				SrecLine->u8DataLen = SrecLine->u8ByteCount - 1 - (SrecLine->u8SrecType + 1);
				count = 0;

				memset(Buffer_Data, 0, sizeof(Buffer_Data)); // Reset buffer

				if (SrecLine->u8SrecType == 7 || SrecLine->u8SrecType == 8 || SrecLine->u8SrecType == 9) {
					*State = SREC_READ_CHECKSUM;
					break;
				}
			}
			break;

		case SREC_READ_DATA:
			count++;

			temp_data = charToHex(temp_data);
			Buffer_Data[(count - 1) / 2] = (Buffer_Data[(count - 1) / 2] << 4) | temp_data;

//			Sum_Data += Buffer_Data[count - 1];
//			uint8_t count_temp = count;
			if (count > 0 && (count % 2) == 0 ) {

				Sum_Data += Buffer_Data[(count - 1) / 2];
			}
			if (count == (2 * SrecLine->u8DataLen)) {
				*State = SREC_READ_CHECKSUM;
				count = 0;

				break;
			}
			break;

		case SREC_READ_CHECKSUM:
			count++;
			temp_data = charToHex(temp_data);
			SrecLine->u8CheckSum += temp_data * pow(16, 2 - count);

			if (SrecLine->u8SrecType == 9 && count == 2) {

				*State = SREC_DONE;
				*share_value = 0x09; 	// reset share value application
				NVIC_SystemReset();		// Reset system to jump to app

				break;
			}

			if (count == 2) {

				uint32_t Sum_Of_Line = SrecLine->u8ByteCount +
						(SrecLine->u32Address >> 8) +
						(SrecLine->u32Address & 0xFF) +
						Sum_Data;

				uint8_t low_byte = Sum_Of_Line & 0xFF;
				uint8_t checksum = 0xFF - low_byte;
				if (checksum == SrecLine->u8CheckSum) {
					*State = SREC_END_OF_LINE;
					count = 0;
					Sum_Data = 0;
					break;
				}
				else {
					*State = SREC_READ_ERROR;
				}
			}
			break;

		case SREC_END_OF_LINE:

			count++;

			if (count == 2 && (temp_data == 0x0D || temp_data == 0x0A)) {

				// Ghi vào Flash.
				uint32_t i = 0;
				while (i < sizeof(Buffer_Data) / 16) //  4 byte
				{

					uint32_t dataToWrite = (Buffer_Data[i * 4 + 3] << 24) |  // Byte thấp nhất (LSB)
										   (Buffer_Data[i * 4 + 2] << 16) |  // Byte thứ hai
										   (Buffer_Data[i * 4 + 1] << 8)  |  // Byte thứ ba
										   (Buffer_Data[i * 4]);             // Byte cao nhất (MSB)

					// Tính địa chỉ cần ghi
					uint32_t address = SrecLine->u32Address + i * 4; // Địa chỉ tiếp theo


					// Flash_EraseSector(address / 1024);


					FLASH_Write(address, dataToWrite);
					i++;
				}

				*State = SREC_READ_RECORDTYPE;
				memset(SrecLine, 0, sizeof(SrecLine_t));
				count = 0;
			}
			break;

//		case SREC_DONE:
//				// JumtoApp.
//
//			*State = SREC_READ_RECORDTYPE;
//			count = 0;
//			break;

		case SREC_READ_ERROR:
			*State = SREC_READ_RECORDTYPE;
			count = 0;
			break;

		default:
			*State = SREC_READ_RECORDTYPE;
			break;
	}
}

uint32_t charToHex(char CTH) {
	if ('0' <= CTH && CTH <= '9') return CTH - '0';
	else if ('A' <= CTH && CTH <= 'F') return CTH - 'A' + 10;
	return 0;
}
