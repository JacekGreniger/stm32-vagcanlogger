#ifndef KW1281_H
#define KW1281_H

#include "stm32f10x_lib.h"
#include "stdio.h"

#define KW1281_ACK 0x09
#define KW1281_NO_ACK 0x0A
#define KW1281_END_OF_SESSION 0x06
#define KW1281_ASCII 0xF6
#define KW1281_0X02_FRAME 0x02

#define KW1281_ERRORS_DELETE 0x05
#define KW1281_ERRORS_REQ 0x07
#define KW1281_ERRORS_RESP 0xFC

#define KW1281_GROUP_REQ 0x29
#define KW1281_GROUP_RESP 0xE7

#define KW1281_BLOCK_END 0x03

extern vu16 timer1;
extern vu16 timer2;
extern vu16 timerKW;
extern vu8 buttonState;

extern u8  kw1281_max_init_attempts; // maksymalna ilosc prob polaczenia ze sterownikiem
extern u8  kw1281_max_byte_transmit_attempts; //wysylanie komunikatu do ecu, maksymalna liczba prób wyslania bajtu w przypadku nieotrzymania zanegowanej odpowiedzi

extern u8  kw1281_interbyte_delay; //opoznienie przed wyslaniem kolejnych bajtow komunikatu lub opoznienie w wysylaniu zanegowanego potwierdzenia do ecu
extern u8  kw1281_interbyte_delaymax; //maksymalny czas na przyjscie kolejnego bajtu z ecu lub maksymalny czas oczekiwania na przyjscie zanegowanego potwierdzenia od ecu
extern u16 kw1281_intermessage_delay; //opoznienie przed wyslaniem nowego komunikatu
extern u16 kw1281_intermessage_delaymax; //maksymalny czas oczekiwania na przyjscie nowego komunikatu od ecu

typedef struct {
  u8 len;
  u8 cnt;
  u8 title;
  u8 * data;
} KW1281struct_t;

//u8 KW1281BlockCounter;
// nalezy go powiekszyc o 1 przed wyslaniem nowego bloku oraz po otrzymaniu bloku od ECU
// mozna wykorzystac do sprawdzenia czy nie zostala zagubiona jakas ramka

extern u8 KW1281ProtocolVersion[2];

u8 ISO9141Init(u16 * uartSpeed);
u8 KW1281SendBlock(KW1281struct_t * block);
u8 KW1281ReceiveBlock(KW1281struct_t * block);

#endif
