#include "integer.h"
#include "fattime.h"
#include "rtc.h"
#include "time.h"

DWORD get_fattime (void)
{
  DWORD res;
  time_t rawtime;

  rawtime = RTC_GetCounter();  
  struct tm * timeinfo;

  timeinfo = localtime ( &rawtime );
  res =  (((DWORD)timeinfo->tm_year - 2000) << 25)
      | ((DWORD)(timeinfo->tm_mon+1) << 21)
      | ((DWORD)timeinfo->tm_mday << 16)
      | (WORD)(timeinfo->tm_hour << 11)
      | (WORD)(timeinfo->tm_min << 5)
      | (WORD)(timeinfo->tm_sec >> 1);

  return res;
}

