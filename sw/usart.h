#ifndef USART_H
#define USART_H

#include "stm32f10x_lib.h"

void USART1_Init(u32 speed);
void USART1_Deinit();
u8 USART1_DataAvailable();
u8 USART1_GetData();
void USART1_PutData(u8 ch);

void USART2_Init(u32 speed);
void USART2_Deinit();
u8 USART2_DataAvailable();
u8 USART2_GetData();
void USART2_PutData(u8 ch);

#endif
