#include "led.h"
#include "config.h"

#ifdef HW_WITHOUT_CAN
#define GPIO_PORT_RED GPIOA
#define GPIO_PORT_GREEN GPIOA
 
#define GPIO_RED GPIO_Pin_11
#define GPIO_GREEN GPIO_Pin_12

#else //HW WITH CAN

#define GPIO_PORT_RED GPIOB
#define GPIO_PORT_GREEN GPIOB
 
#define GPIO_RED GPIO_Pin_3
#define GPIO_GREEN GPIO_Pin_4

#endif

#define BUZZER_PORT GPIOA
#define BUZZER_PIN GPIO_Pin_4

#define FLASH_FAST_TIMEOUT 20
#define FLASH_SLOW_TIMEOUT 100
  
volatile u8 led_mode;


void LedConfigureGPIO()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

#ifndef HW_WITHOUT_CAN
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
#endif

  GPIO_InitStructure.GPIO_Pin = GPIO_RED;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIO_PORT_RED, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_GREEN;
  GPIO_Init(GPIO_PORT_GREEN, &GPIO_InitStructure);
  
  led_mode = off;
  GPIO_ResetBits(GPIO_PORT_RED, GPIO_RED); 
  GPIO_ResetBits(GPIO_PORT_GREEN, GPIO_GREEN); 
}


void LedSetColor (led_color_t led_color, flash_mode_t flash_mode)
{
  led_mode = led_color | (flash_mode<<2);
}


// call LedService() every 10ms
void LedService()
{
  static u8 flash_counter = 0;

  switch ((led_color_t)(led_mode & 0x03))
  {
    case off:
      GPIO_ResetBits(GPIO_PORT_RED, GPIO_RED); 
      GPIO_ResetBits(GPIO_PORT_GREEN, GPIO_GREEN); 
      break;
      
    case red:
      GPIO_SetBits(GPIO_PORT_RED, GPIO_RED); 
      GPIO_ResetBits(GPIO_PORT_GREEN, GPIO_GREEN); 
      break;

    case green:
      GPIO_ResetBits(GPIO_PORT_RED, GPIO_RED); 
      GPIO_SetBits(GPIO_PORT_GREEN, GPIO_GREEN); 
      break;

    case amber:
      GPIO_SetBits(GPIO_PORT_RED, GPIO_RED); 
      GPIO_SetBits(GPIO_PORT_GREEN, GPIO_GREEN); 
      break;
  }
  
  if (led_mode & 0x04)
  {
    flash_counter = (flash_counter < FLASH_FAST_TIMEOUT)?flash_counter+1:0;
    if (flash_counter >= (FLASH_FAST_TIMEOUT/2))
    {
      GPIO_ResetBits(GPIO_PORT_RED, GPIO_RED); 
      GPIO_ResetBits(GPIO_PORT_GREEN, GPIO_GREEN); 
    }
  }
  else if (led_mode & 0x08)
  {
    flash_counter = (flash_counter < FLASH_SLOW_TIMEOUT)?flash_counter+1:0;
    if (flash_counter >= (FLASH_SLOW_TIMEOUT/2))
    {
      GPIO_ResetBits(GPIO_PORT_RED, GPIO_RED); 
      GPIO_ResetBits(GPIO_PORT_GREEN, GPIO_GREEN); 
    }
  }
  else 
  {
    flash_counter = 0;
  }
}

#define BUZZER_SHORT_TIME 15
#define BUZZER_LONG_TIME 50
#define BUZZER_SILENT_TIME 15

volatile u8 buzzer_mode;

void BuzzerConfigureGPIO()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);
  
  GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN); 
  buzzer_mode = 0;
}


void BuzzerSet(u8 state)
{
  if (state)
  {
    GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);
  }
  else
  {
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
  }
}


void BuzzerSetMode(u8 mode)
{
  buzzer_mode = mode;
}

// call BuzzerService() every 10ms
void BuzzerService()
{
  static u8 buzzer_counter = 0;
  static u8 beep_counter = 0;
  static u8 long_beep = 0;
  static enum  {
    BUZZER_NO_ACTION = 0,
    BUZZER_BEEPING = 1,
    BUZZER_SILENT = 2
  } buzzer_mode_internal = BUZZER_NO_ACTION;

  if (buzzer_mode & 0x07)
  {
    beep_counter = (buzzer_mode & 0x07) - 1;
    long_beep = (buzzer_mode & LONG_BEEP)?1:0;
    buzzer_mode = 0;
    buzzer_counter = (long_beep)?BUZZER_LONG_TIME:BUZZER_SHORT_TIME;
    buzzer_mode_internal = BUZZER_BEEPING;
    BuzzerSet(1);
    return;
  }
  
  if (BUZZER_NO_ACTION != buzzer_mode_internal)
  {
    if (buzzer_counter > 0)
    {
      --buzzer_counter;
    }
    else
    {
      BuzzerSet(0);
      if ((BUZZER_BEEPING == buzzer_mode_internal) && (beep_counter))
      {
        buzzer_counter = BUZZER_SILENT_TIME;
        buzzer_mode_internal = BUZZER_SILENT;
      }
      else if ((BUZZER_SILENT == buzzer_mode_internal) && (beep_counter))
      {
        --beep_counter;
        buzzer_counter = (long_beep)?BUZZER_LONG_TIME:BUZZER_SHORT_TIME;
        buzzer_mode_internal = BUZZER_BEEPING;
        BuzzerSet(1);
      }
      else
      {
        buzzer_mode_internal = BUZZER_NO_ACTION;
      }
    }
  }
}
