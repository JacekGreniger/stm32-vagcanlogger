#ifndef LED_H
#define LED_H

#include "stm32f10x_lib.h"

typedef enum
{
  off = 0,
  green = 1,
  red = 2,
  amber = 3
} led_color_t;

typedef enum
{
  continous = 0,
  fast = 1,
  slow = 2,
} flash_mode_t;

#define LONG_BEEP 0x80

void LedConfigureGPIO();
void LedSetColor (led_color_t led_color, flash_mode_t flash_mode);
void LedService();

void BuzzerConfigureGPIO();
void BuzzerSet(u8 state);
void BuzzerSetMode(u8 mode);
void BuzzerService();

#endif //LED_H