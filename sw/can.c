#include "stm32f10x_lib.h"
#include "filesystem/integer.h"
#include "filesystem/diskio.h"
#include "filesystem/ff.h"
#include "stdio.h"
#include "can.h"
#include "config.h"

u8 canDebug = 0;
u8 extId = 0;
FIL *log_file;

void CAN_UseStdId()
{
  extId = 0;
}


void CAN_UseExtId()
{
  extId = 1;
}

#ifdef DEBUG_LOG_CAN_FRAME
void CAN_Debug(u8 on, FIL *debug_file)
{
  if (on)
  {
    canDebug = 1;
    log_file = debug_file;
  }
  else
  {
    canDebug = 0;
  }
}
#endif

#ifdef DEBUG_LOG_CAN_FRAME
void DebugCanFrame(u8 tx, u8 extFrame, CanMessage_t *msg)
{
  u8 i;
  u8 debugString[60];
  u8 *p = debugString;

  if (canDebug)
  {
    p += sprintf(p, "%s %s id=%03x len=%d payload=", tx?"TX":"RX", extFrame?"EXT":"STD", msg->id, msg->len);
    for (i=0; i<msg->len; i++)
    {
      p += sprintf(p, "%02x ", msg->payload[i]);
    }
    p += sprintf(p, "\n");
  
    f_puts(debugString, log_file);
  }
}
#endif

void CAN_Open() 
{
  CAN_InitTypeDef        CAN_InitStructure;
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN, ENABLE);

  // Configure CAN pin: RX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // Configure CAN pin: TX
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* CAN register init */
  CAN_DeInit();
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM=DISABLE;
  CAN_InitStructure.CAN_ABOM=DISABLE;
  CAN_InitStructure.CAN_AWUM=DISABLE;
  //CAN_InitStructure.CAN_NART=DISABLE;
  CAN_InitStructure.CAN_NART=ENABLE; //no automatic retransmission
  CAN_InitStructure.CAN_RFLM=DISABLE;
  CAN_InitStructure.CAN_TXFP=DISABLE;

  CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;
  
  //CAN block is clocked with Low Speed APB1 clock (PCLK1) = 36MHz
  CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;
  CAN_InitStructure.CAN_BS1=CAN_BS1_9tq; //CAN_BS1_8tq @ 24mhz
  CAN_InitStructure.CAN_BS2=CAN_BS2_8tq; //CAN_BS2_7tq @24mhz
  CAN_InitStructure.CAN_Prescaler=4; //=3  500kbit@24mhz
  
  CAN_Init(&CAN_InitStructure);

  /* CAN filter init */
  CAN_FilterInitStructure.CAN_FilterNumber=0;
  CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment=0;
  CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}


void CAN_Close() 
{
  CAN_DeInit();
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN, DISABLE);
}


//returns 1 if ExtID frame was received
u8 CAN_ReceiveMsg(CanMessage_t * msg)
{
  CanRxMsg RxMessage;

  CAN_Receive(CAN_FIFO0, &RxMessage);

  if (CAN_ID_STD == RxMessage.IDE)
  {
    msg->id = RxMessage.StdId;
    msg->len = RxMessage.DLC;
    memmove(msg->payload, RxMessage.Data, 8);
#ifdef DEBUG_LOG_CAN_FRAME
    DebugCanFrame(0, 0, msg);
#endif
    return 0;
  }
  else //CAN_ID_EXT
  {
    msg->id = RxMessage.ExtId;
    msg->len = RxMessage.DLC;
    memmove(msg->payload, RxMessage.Data, 8);
#ifdef DEBUG_LOG_CAN_FRAME
    DebugCanFrame(0, 1, msg);
#endif
    return 1;
  }
}


u8 CAN_SendMsg(CanMessage_t * msg) 
{
  CanTxMsg TxMessage;
  u8 TransmitMailbox, i;

  if (0 == extId)
  {
    TxMessage.StdId = msg->id;
    TxMessage.ExtId = 0;
    TxMessage.IDE = CAN_ID_STD;
  }
  else
  {
    TxMessage.StdId = 0;
    TxMessage.ExtId = msg->id;
    TxMessage.IDE = CAN_ID_EXT;
  }

  TxMessage.RTR = CAN_RTR_DATA;
  TxMessage.DLC = msg->len;

  memmove(TxMessage.Data, msg->payload, 8);

  TransmitMailbox=CAN_Transmit(&TxMessage);
  if (CAN_NO_MB == TransmitMailbox) {
    return 0;
  }

  i = 0;
  while((CAN_TransmitStatus(TransmitMailbox) != CANTXOK) && (i != 0xFF))
  {
    i++;
    delay(1000);
  }

  if (i != 0xff)
  {
#ifdef DEBUG_LOG_CAN_FRAME
    DebugCanFrame(1, extId, msg);
#endif
    return 255;
  }
  else
  {
    return 0; //can transmit fails
  }
}

void CAN_SetFilter0(u32 id)
{
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  /* CAN filter init */
  CAN_FilterInitStructure.CAN_FilterNumber=0;
  CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdList; //CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
  
  CAN_FilterInitStructure.CAN_FilterIdHigh=(id & 0x07ff) << 5L; 
  CAN_FilterInitStructure.CAN_FilterIdLow=0x0000; //IDE=0
  
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=(((id+1) & 0x3E000) >> 13L); 
  CAN_FilterInitStructure.CAN_FilterMaskIdLow= (((id+1) & 0x01fff) << 3L) | 0x4; //IDE=1
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment=0;
  CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}


void CAN_ResetFilter0()
{
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  /* CAN filter init */
  CAN_FilterInitStructure.CAN_FilterNumber=0;
  CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
  
  CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow=0x0000; 
  
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow= 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment=0;
  CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}


void CAN_FlushReceiveFifo()
{
  CanRxMsg RxMessage;

  while (CAN_MessagePending(CAN_FIFO0))
  {
    CAN_Receive(CAN_FIFO0, &RxMessage);
  }
}
