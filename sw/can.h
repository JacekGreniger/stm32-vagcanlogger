#ifndef __CAN_FUNCTIONS_H__
#define __CAN_FUNCTIONS_H__

#include "stm32f10x_map.h"
#include "filesystem/integer.h"
#include "filesystem/diskio.h"
#include "filesystem/ff.h"

typedef struct
{
  u16 id;
  u8  len;
  u8  payload[8];
} CanMessage_t;

void CAN_Open();
void CAN_Close();
u8 CAN_ReceiveMsg(CanMessage_t * msg);
u8 CAN_SendMsg(CanMessage_t * msg);
void CAN_SetFilter0(u32 id);
void CAN_ResetFilter0();
void CAN_FlushReceiveFifo();
void CAN_Debug(u8 on, FIL *debug_file);
void CAN_UseStdId();
void CAN_UseExtId();

#endif
