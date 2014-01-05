#include "button.h"
#include "config.h"

#define GPIO_PORT_BUTTON GPIOB
#define GPIO_PORT_PIN_BUTTON GPIO_Pin_0

vu8 buttonState = 0;


void ButtonConfigureGPIO()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_PORT_PIN_BUTTON;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //GPIO_Mode_IN_FLOATING
  GPIO_Init(GPIO_PORT_BUTTON, &GPIO_InitStructure); 
}


// ButtonService should be called every 10ms
void ButtonService()
{
  extern vu8 buttonState;
  static u8 buttonTimer = 0;
  static u8 keyPressed = 0;

  buttonTimer = (buttonTimer<128)?buttonTimer+1:buttonTimer;

  if (0 == GPIO_ReadInputDataBit(GPIO_PORT_BUTTON, GPIO_PORT_PIN_BUTTON)) 
  {
    if (keyPressed == 0)
    {
      keyPressed = 1;
      buttonTimer = 0;
    }
    else if ((buttonTimer > 80) && (buttonTimer < 128))
    {
      buttonState = 2;
      buttonTimer = 128;
    }
  }
  else if (GPIO_ReadInputDataBit(GPIO_PORT_BUTTON, GPIO_PORT_PIN_BUTTON) && (keyPressed))
  {
    keyPressed = 0;
    if ((buttonTimer > 5) &&(buttonTimer < 50))
    {
      buttonState = 1;
    }
    else if ((buttonTimer > 80) && (buttonTimer < 128))
    {
      buttonState = 2;
    }
  }
}