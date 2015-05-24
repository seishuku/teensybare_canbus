#ifndef __CAN_H__
#define __CAN_H__

#define CAN_MESSAGE_ID_EXT			0x80000000

typedef struct
{
	uint32_t MessageID;
	uint8_t Data[8], Length;
} CAN_Frame_t;

void CAN_Init(void);
int CAN_ReadFrame(CAN_Frame_t *Frame);
int CAN_SendFrame(CAN_Frame_t Frame);

#endif
