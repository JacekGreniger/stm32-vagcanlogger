#ifndef RTC_H
#define RTC_H

#include "stm32f10x_lib.h"

void ConfigureClock(void);
void RTC_Configuration(void);
void Time_Adjust(u32 val);
void ShowClock();

#endif //RTC_H