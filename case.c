#include <stdint.h>
#include <math.h>
#include <string.h> // strlen() and memset()
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

// Middleware
#define LED_GREEN_PORT  PORTD
#define LED_GREEN_PIN   4
#define LED_RED_PORT    PORTD
#define LED_RED_PIN     5

volatile uint16_t LPIT_ISR_Flag = 0;
volatile uint16_t UART0_ISR_Flag = 0;
volatile uint8_t temp_data = 0;
Queue* q; 
int i, j;
typedef struct {
    uint8_t u8SrecType;    // 1, 2, 3, 7, 8, 9
    uint8_t u8ByteCount;
    uint32_t u32Address;
    uint8_t *pData;
    uint8_t u8DataLen;
    uint8_t u8CheckSum;
    uint32_t u32Data;
} SrecLine_t;

SrecLine_t SrecLine;

typedef enum {
    SREC_READ_RECORDTYPE,   // Reading the record type
    SREC_READ_BYTECOUNT,    // Reading the byte count
    SREC_READ_ADDRESS,      // Reading the address
    SREC_READ_DATA,         // Reading the data
    SREC_READ_CHECKSUM,     // Reading the checksum
    HEX_END_OF_LINE,        // End of line (\n or \r)
    HEX_DONE,               // End of file
    SREC_READ_ERROR,        // Error state
} State_t;

State_t State = SREC_READ_RECORDTYPE;

uint32_t charToHex(char CTH) {
    if ('0' <= CTH && CTH <= '9') return CTH - '0';
    else if ('A' <= CTH && CTH <= 'F') return CTH - 'A' + 10;
    else if ('a' <= CTH && CTH <= 'f') return CTH - 'a' + 10;
    return 0;
}

int main(void) {
	
    q = createQueue();

    //-------- Test Data ------------------------------------
    char transmissionString[] = "S113200000600020D520000043210000E5250000E9\nS113200000600020D520000043210000E5250000E9\nS113200000600020D520000043210000E5250000E9\nS113200000600020D520000043210000E5250000E9\nS113200000600020D520000043210000E5250000E9\nS113200000600020D520000043210000E5250000E9\nS113200000600020D520000043210000E5250000E9\n";
    for ( i = 0; i < strlen(transmissionString); i++) {
        enqueue(q, transmissionString[i]);
    }

    displayQueue(q);
    //-------- Test Data ---------------------------------------

    int count = 0;
    uint8_t numAddr = 0;
    uint8_t Buffer_Data[64] = {0};

    while (!isEmpty(q)) {
    	
        uint8_t temp_data = dequeue(q);
   
        switch(State) {
            case SREC_READ_RECORDTYPE:
                count++;
                if (count == 1) {
                    if (temp_data != 'S') {
                        State = SREC_READ_ERROR;
                        break;
                    }
                }
                else if (count == 2) {
                    temp_data = charToHex(temp_data);
                    SrecLine.u8SrecType = temp_data;
                    State = SREC_READ_BYTECOUNT;
                    count = 0;

                    if (SrecLine.u8SrecType == 8) numAddr = 6; // 3 bytes address
                    else if (SrecLine.u8SrecType == 9) numAddr = 4; // 2 bytes address
                    else if (SrecLine.u8SrecType == 7) numAddr = 8; // 4 bytes address
                    else numAddr = 2 * (SrecLine.u8SrecType + 1); // vd: S1 -> 4 nibbles (2 bytes)
                }
                break;

            case SREC_READ_BYTECOUNT:
                count++;
                temp_data = charToHex(temp_data);
                if (count == 1) {
                    SrecLine.u8ByteCount = temp_data * 16; // Higher 
                } 
                else if (count == 2) {
                    SrecLine.u8ByteCount += temp_data; // Lower 
                    State = SREC_READ_ADDRESS;
                    count = 0;
                }
                break;

            case SREC_READ_ADDRESS:
                count++;
                temp_data = charToHex(temp_data);
                SrecLine.u32Address = (SrecLine.u32Address << 4) | temp_data; 

                if (count == numAddr) {
                    State = SREC_READ_DATA;

                    //data length
                    int address_bytes = numAddr / 2; 
                    SrecLine.u8DataLen = SrecLine.u8ByteCount - address_bytes - 1; //  address and checksum
                    count = 0;

                    memset(Buffer_Data, 0, sizeof(Buffer_Data)); // Reset buffer
                }
                break;

            case SREC_READ_DATA:
                count++;
                temp_data = charToHex(temp_data);
                Buffer_Data[(count - 1) / 2] = (Buffer_Data[(count - 1) / 2] << 4) | temp_data; 

                if (count == SrecLine.u8DataLen * 2) { 
                    State = SREC_READ_CHECKSUM;
                    count = 0;
                }
                break;

            case SREC_READ_CHECKSUM:
                count++;
                temp_data = charToHex(temp_data);
                if (count == 1) {
                    SrecLine.u8CheckSum = temp_data * 16; // Higher nibble
                } 
                else if (count == 2) {
                    SrecLine.u8CheckSum += temp_data; // Lower nibble

                  
    //--------------------------------------------------------------------------------------------
                    printf("Type: S%u\n", SrecLine.u8SrecType);
                    printf("Byte Count: %u\n", SrecLine.u8ByteCount);
                    printf("Address: 0x%08X\n", SrecLine.u32Address);
                    printf("Data Length: %u bytes\n", SrecLine.u8DataLen);
                    printf("Data: ");
                    for ( j = 0; j < SrecLine.u8DataLen; j++) {
                        printf("%02X ", Buffer_Data[j]);
                    }
                    printf("\nChecksum: 0x%02X\n", SrecLine.u8CheckSum);
    //------------------------------------------------------------------------------------------
                    State = HEX_END_OF_LINE;
                    count = 0;
                }
                break;

            case HEX_END_OF_LINE:
                if (temp_data == '\n' || temp_data == '\r') {
                    State = SREC_READ_RECORDTYPE; 
                } else if (isEmpty(q)) {
                    State = HEX_DONE;
                }
                break;

            case HEX_DONE:
                break;

            case SREC_READ_ERROR:
                State = SREC_READ_RECORDTYPE;
                count = 0;
                break;

            default:
                State = SREC_READ_RECORDTYPE;
                break;
        }
    }

//    freeQueue(q);

    return 0;
}

