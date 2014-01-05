#include "stm32f10x_lib.h"
#include "stdio.h"
#include "config.h"
#include "kw1281.h"
#include "usart.h"
#include "led.h"
#include "file_operations.h"
#include "rtc.h"
#include <time.h>
#include "vwtp.h"

#include "filesystem/integer.h"
#include "filesystem/diskio.h"
#include "filesystem/ff.h"

vu16 timer1 = 0;
vu16 timer2 = 0;
vu16 timerKW = 0;
vu16 timeSec = 0;
vu8 time10MSec = 0;

FATFS Fatfs[_DRIVES];    // File system object for each logical drive 


/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM  
  /* Set the Vector Table base location at 0x20000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
}

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{    
  ErrorStatus HSEStartUpStatus;

  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
 
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08);
  }
  
  /* Enable GPIOA  clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
}

/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //MMC supply control
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB, GPIO_Pin_1); 
}


#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


//delay(4000); //1ms @ 72MHz
void delay(int time)
{
  while(time--)
  __asm__ volatile ("nop");  
}

//longdelay(96); //0.2s @ 72MHz
void longdelay(unsigned long t) 
{
  while (t--) 
  {
    delay(10000);
  }
}


#ifdef UART1_DEBUG
int putchar(const int ch)
{
  USART_SendData(USART1, (u8) ch);
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
  return ch;
}
#endif


void SystickConfigure()
{
  /* SysTick end of count event each 1ms with input clock equal to 9MHz (HCLK/8, default) */
  SysTick_SetReload(9000);
  SysTick_ITConfig(ENABLE);
  SysTick_CounterCmd(SysTick_Counter_Enable);
  timer1 = 0;
  timer2 = 0;
  timerKW = 0;
}

#define MMC_PORT GPIOB
#define MMC_CS GPIO_Pin_12
#define MMC_MOSI GPIO_Pin_15

void MMC_PowerOn()
{
  GPIO_InitTypeDef GPIO_InitStructure; 

  // GPIO configuration
  GPIO_InitStructure.GPIO_Pin = MMC_CS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(MMC_PORT, &GPIO_InitStructure);

  (void)GPIO_SetBits(MMC_PORT, MMC_CS);

  /* Configure SPI2 pins*/
  GPIO_InitStructure.GPIO_Pin = MMC_MOSI;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(MMC_PORT, &GPIO_InitStructure);

  GPIO_ResetBits(GPIOB, GPIO_Pin_1);
}

void MMC_PowerOff()
{
  GPIO_InitTypeDef GPIO_InitStructure; 

  // GPIO configuration
  GPIO_InitStructure.GPIO_Pin = MMC_CS |  MMC_MOSI;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(MMC_PORT, &GPIO_InitStructure);

  (void)GPIO_ResetBits(MMC_PORT, MMC_CS |  MMC_MOSI);

  GPIO_SetBits(GPIOB, GPIO_Pin_1);
}


void Timer3Init()
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  
  //clocked with 36MHz timers clock is multiplied by 2 so CLK is 72MHz

  /* ---------------------------------------------------------------------------
    TIM3 Configuration: 
    TIM3CLK = 72 MHz, Prescaler = 719 (divide by 720), TIM3 counter clock = 100 kHz (period 0.01ms)
  ----------------------------------------------------------------------------*/
 
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 71; 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  TIM_Cmd(TIM3, ENABLE);
}


u16 KW1281_SPEED=0; //autobaud
u8 autobaud = 1;
u8 user_defined_timings = 0;
u8 canDiag = 0;

int main(void)
{
  FIL log_file;
  int i, j, result;
  int file_number = 0;
  u8 configNumber = 1;
  u8 maxConfig;
  u8 *p;
  time_t rawtime;
  u8 debugState = 0; //debug off

#ifdef DEBUG
  debug();
#endif

  RCC_Configuration();
  GPIO_Configuration();
  NVIC_Configuration();
  SystickConfigure();
  Timer3Init();
#ifdef UART1_DEBUG
  (void)USART1_Init(115200);
#endif
  LedConfigureGPIO();
  ButtonConfigureGPIO();
  BuzzerConfigureGPIO();

#ifdef CAN_FEATURE_ENABLED
  GPIO_InitTypeDef GPIO_InitStructure;

  // Configure CAN pin: RX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // Configure CAN pin: TX
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

  MMC_PowerOff();
  timer2=1000;
  while(timer2);
  MMC_PowerOn();

#ifdef UART1_DEBUG
  printf("\n\n%s %s\n", HW_VERSION, SW_VERSION);
#endif

  ConfigureClock();
  timer2=1000;
  while(timer2);

  buttonState = 0;

  CardInserted();
  
  if (FR_OK != f_mount(0, &Fatfs[0]))
  {
    LedSetColor(amber, fast);
    BuzzerSetMode(0x83);
#ifdef UART1_DEBUG
    printf("error mounting fat device\n");
#endif
    while(buttonState != 2);
    NVIC_GenerateSystemReset();
  }

  if (ReadTimeDateFile())
  {
#ifdef UART1_DEBUG
    rawtime = RTC_GetCounter();
    printf("Time set: %s\n", ctime (&rawtime) );
#endif
  }

#ifdef DEBUG_LOG
  if (ReadDebugFile())
  {
    debugState = 1;
  }
#endif

#ifdef CAN_FEATURE_ENABLED
  if (ReadCanFile())
  {
    canDiag = 1;
    CAN_Open();
  }
#endif

  i = ReadSpeedFile();
  if (i > 0)
  {
    KW1281_SPEED = i;
    autobaud = 0;
  }

  if (ReadDelayFile())
  {
    user_defined_timings = 1;
  }
  
  if (!ReadConfig())
  {
    LedSetColor(amber, fast); 
    BuzzerSetMode(0x83);
#ifdef UART1_DEBUG
    printf("error reading config\n");
#endif
    MMC_PowerOff();
    while(buttonState != 2);
    NVIC_GenerateSystemReset(); 
  }
  
  if (config[0][0] > 0)
  {
    maxConfig = 1;
    if (config[1][0] > 0)
    {
      maxConfig = 2;
      if (config[2][0] > 0)
      {
        maxConfig = 3;
      }
    }
  }
  else
  {
    LedSetColor(amber, fast); 
    BuzzerSetMode(0x83);
#ifdef UART1_DEBUG
    printf("error reading config\n");
#endif
    MMC_PowerOff();
    while(buttonState != 2);
    NVIC_GenerateSystemReset(); 
  }

  file_number = GetNextFileNumber();
#ifdef UART1_DEBUG
  printf("file number %d\n", file_number);
#endif
  if (!file_number)
  {
    LedSetColor(amber, fast); 
    BuzzerSetMode(0x83);
#ifdef UART1_DEBUG
    printf("can't get number of next file\n");
#endif
    MMC_PowerOff();
    while(buttonState != 2);
    NVIC_GenerateSystemReset(); 
  }
  
  while(1)
  {
    i = USART2_GetData();
    buttonState = 0;
    LedSetColor(configNumber, continous);
    BuzzerSetMode(configNumber);
    do
    {
      if (2 == buttonState)
      {
        configNumber = (configNumber < maxConfig)?configNumber+1:1;
        LedSetColor(configNumber, continous);
        BuzzerSetMode(configNumber);
        buttonState = 0;
      }
    } while (1 != buttonState);
    
    buttonState = 0;
    LedSetColor(green, fast);
    BuzzerSetMode(0x81);

    i = CreateLogFile(file_number, &log_file);

    i = f_printf(&log_file, "%s, sw_ver: %s\n", HW_VERSION, SW_VERSION);

    rawtime = RTC_GetCounter();
    i = f_printf(&log_file, "Current time: %s\n", ctime (&rawtime));
    
    timeSec = 0;
    time10MSec = 0;
    
    if (canDiag)
    {
      i = f_printf(&log_file, "CAN 500kbit\n\n");
      LedSetColor(configNumber, slow);
      BuzzerSetMode(configNumber);
      result = vwtp(&config[configNumber-1], &log_file, debugState);
      
      switch (result)
      {
        case 0:
          timer2 = 500;
          i = f_printf(&log_file, "\n\nLogging terminated by user\n");
          break;
        
        case 1:
          LedSetColor(red, fast);
          BuzzerSetMode(0x82); //communication error
          timer2 = 2000;
          i = f_printf(&log_file, "\n\nConnection lost\n");
          break;
          
        case 2:
          i = CloseLogFile(&log_file);
          LedSetColor(amber, fast);
          BuzzerSetMode(0x83); //filesystem error
          MMC_PowerOff();
          while(buttonState != 2);
          NVIC_GenerateSystemReset();  
          break;
        
        case 11:
          LedSetColor(red, fast);
          BuzzerSetMode(0x82); //communication error
          timer2 = 2000;
          i = f_printf(&log_file, "\n\nCannot connect with ECU\n", result);
          break;

        default:
          LedSetColor(red, fast);
          BuzzerSetMode(0x82); //communication error
          timer2 = 2000;
          i = f_printf(&log_file, "\n\nCommunication error, error code = %d\n", result);
          break;
      }
    } 
    else //KW1281 diag
    {
      i = kw1281_max_init_attempts;
      do
      {
        result = ISO9141Init(&KW1281_SPEED);

        --i;
        if ((result>0) && (i>0))
        {
          timer2=1000;
          while(timer2);
        }
      } while (result && i);

      if (0 == result) //connected with ECU
      {
        i = f_printf(&log_file, "Connected @ %d baud %s%s\n\n", KW1281_SPEED, autobaud?"(autodetected)":"(set by user)", user_defined_timings?", user defined timings":"");
        LedSetColor(configNumber, slow);
        BuzzerSetMode(configNumber);
    
        buttonState = 0;
        i = kw1281_diag(&config[configNumber-1], &log_file, debugState);
      
        switch (i)
        {
          case 0:
            timer2 = 500;
            i = f_printf(&log_file, "\n\nLogging terminated by user\n");
            break;
        
          case 1:
            LedSetColor(red, fast);
            BuzzerSetMode(0x82); //communication error
            timer2 = 2000;
            i = f_printf(&log_file, "\n\nConnection lost\n");
            break;
          
          case 2:
          default:
            i = CloseLogFile(&log_file);
            LedSetColor(amber, fast);
            BuzzerSetMode(0x83); //filesystem error
            MMC_PowerOff();
            while(buttonState != 2);
            NVIC_GenerateSystemReset();  
        }
      }
      if (0 != result)
      {
        i = f_printf(&log_file, "Cannot connect with ECU: ");
        if (0xff == result)
        {
          i = f_printf(&log_file, "no response\n");
        }
        else if (0x7f == result)
        {
          i = f_printf(&log_file, "sync error - wrong speed (set to %d baud)?!\n", KW1281_SPEED);
        }
        else if (0x10 == result)
        {
          i = f_printf(&log_file, "not a KW1281 protocol\n");
        }
        else
        {
          i = f_printf(&log_file, "error code %d\n", result);
        }
        LedSetColor(red, fast); 
        BuzzerSetMode(0x82);
        timer2 = 2000;
      }
    } // end of KW1281 diag

    i = CloseLogFile(&log_file);
    file_number++;
    while (timer2>0);
  }
}

