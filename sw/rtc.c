/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"
#include "config.h"
#include <stdio.h>
#include <time.h>

void ConfigureClock(void)
{
  if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
  {
    /* Backup data register value is not correct or not yet programmed (when
       the first time the program is executed) */

#ifdef UART1_DEBUG
    printf("RTC not yet configured....\n");
#endif

    /* RTC Configuration */
    RTC_Configuration();

#ifdef UART1_DEBUG
    printf("RTC configured....\n");
#endif

    struct tm timedate;

    timedate.tm_sec = 0;     /* seconds after the minute - [0,59] */
    timedate.tm_min = 13;     /* minutes after the hour - [0,59] */
    timedate.tm_hour = 20;    /* hours since midnight - [0,23] */
    timedate.tm_mday = 3;    /* day of the month - [1,31] */
    timedate.tm_mon = 0;     /* months since January - [0,11] */
    timedate.tm_year = 111;    /* years since 1900 */
    //int tm_wday;    /* days since Sunday - [0,6] */
    //int tm_yday;    /* days since January 1 - [0,365] */
    //int tm_isdst;   /* daylight savings time flag */

    time_t rawtime;

    rawtime = mktime (&timedate);
    Time_Adjust(rawtime); 

    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
  }
  else
  {
    /* Check if the Power On Reset flag is set */
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {
#ifdef UART1_DEBUG
      printf("Power On Reset occurred....\n");
#endif
    }
    /* Check if the Pin Reset flag is set */
    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {
#ifdef UART1_DEBUG
      printf("External Reset occurred....\n");
#endif
    }

#ifdef UART1_DEBUG
    printf("No need to configure RTC....\n");
    /* Wait for RTC registers synchronization */
#endif
    RTC_WaitForSynchro();

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
  }

  /* Clear reset flags */
  RCC_ClearFlag();
}

#ifdef UART1_DEBUG
void ShowClock()
{
  time_t rawtime;
  while (1)
  {
    rawtime = RTC_GetCounter();
    printf ( "The current local time is: %s\n", ctime (&rawtime) );
    longdelay(500);
  }
}
#endif

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_Configuration(void)
{
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();

  /* Enable LSE */
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {}

  /* Select LSE as RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC registers synchronization */
  RTC_WaitForSynchro();

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Enable the RTC Second */
  RTC_ITConfig(RTC_IT_SEC, ENABLE);

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Set RTC prescaler: set RTC period to 1sec */
  RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}


/*******************************************************************************
* Function Name  : Time_Adjust
* Description    : Adjusts time.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Time_Adjust(u32 val)
{
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  /* Change the current time */
  RTC_SetCounter(val);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}
