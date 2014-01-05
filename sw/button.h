#ifndef BUTTON_H
#define BUTTON_H

#include "stm32f10x_lib.h"

extern vu8 buttonState;

void ButtonService();
void ButtonConfigureGPIO();

#endif //BUTTON_H