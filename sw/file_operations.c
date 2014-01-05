#include "stm32f10x_lib.h"
#include "file_operations.h"
#include "stdio.h"
#include "string.h"
#include "rtc.h"
#include "time.h"
#include "kw1281.h"

#include "filesystem/integer.h"
#include "filesystem/diskio.h"
#include "filesystem/ff.h"

u8 config[3][4];
const char * path  = "";
const char * ext = ".CSV";
extern FIL log_file2;

//returns: 0 - error
//returns 1-999 - first free file number
int GetNextFileNumber ()
{
  FRESULT res;
  FILINFO fno;
  DIR dir;
  int file_number = 1;
  int i;
  char *fn;

  res = f_opendir(&dir, path);
  if (res != FR_OK) return 0; //filesystem error
  
  for (;;) 
  {
    res = f_readdir(&dir, &fno);
    if (res != FR_OK) { return 0; } //filesystem error
    if (fno.fname[0] == 0) { break; } //no more files in this directory
  
    if ((fno.fname[0] == '.') || (fno.fattrib & AM_DIR)) { continue; }
    
    fn = strstr(fno.fname, ext);
    if (NULL == fn) { continue; } //matching extension not found in filename
    fn[0] = 0;
    if ((strlen(fno.fname) != 3) || (!isdigit(fno.fname[0])) || (!isdigit(fno.fname[1])) || (!isdigit(fno.fname[2]))) { continue; }
    
    i = atoi(fno.fname);
    if (i >= file_number) { file_number = i+1; }
    if (999 == i) { file_number = 0; break; }
  }

  return file_number;
}


int CreateLogFile(int file_number, FIL *log_file)
{
  FRESULT res;
  char s[12];
  
  s[0] = '0' + file_number/100;
  s[1] = '0' + file_number/10;
  s[2] = '0' + file_number%10;
  s[3] = 0;
  strcat(s, ext);
  
  res = f_open(log_file, s, FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK) { return 0; }
  
  return 1;
}


int CloseLogFile(FIL * log_file)
{
  FRESULT res;
  res = f_close(log_file);
  if (res != FR_OK) { return 0; }
  
  return 1; 
}


const char * config_filename = "config.txt";
const char * delimiters = " ,.:;\n\r";

int ReadConfig()
{
  FIL config_file;
  FRESULT res;
  int i, j, count, val;
  u8 textbuf[50];
  u8 * s;

  memset(config, sizeof(config), 0);
  
  res = f_open(&config_file, config_filename, FA_OPEN_EXISTING | FA_READ);
  if (res != FR_OK) { return 0; }

  i = 0;
  s = f_gets(textbuf, sizeof(textbuf), &config_file);
  if (NULL == s)
  {
    f_close(&config_file);
    return 0;
  }

  do
  {
    s = strtok(textbuf, delimiters);
    j = 0;
    while ((s!=NULL) && (j<4))
    {
      val = atoi(s);
      if ((val > 0) && (val <= 255))
      {
        j++;
        config[i][0] = j;
        config[i][j] = val;
      }
      else
      {
        break; //ignore this line
      }
      
      s = strtok(NULL, delimiters);
    }
    if (j > 0) { i++; }
    
    s = f_gets(textbuf, sizeof(textbuf), &config_file);
  } while ((s != NULL) && (i < 3));

  res = f_close(&config_file);
  if (res != FR_OK) { return 0; }
  
  return 1;
}


const u8 * speed_filename = "SPEED.TXT";

u16 ReadSpeedFile()
{
  FIL speed_file;
  FRESULT res;
  u16 speed;
  u8 textbuf[10];
  u8 * s;

  res = f_open(&speed_file, speed_filename, FA_OPEN_EXISTING | FA_READ);
  if (res != FR_OK) { return 0; }

  s = f_gets(textbuf, sizeof(textbuf), &speed_file);
  if (NULL == s)
  {
    f_close(&speed_file);
    return 0;
  }

  s = strtok(textbuf, delimiters);
  if ((NULL == s) || (strlen(s) < 4) || (strlen(s) > 5))
  {
    f_close(&speed_file);
    return 0;
  }

  speed = atoi(s);
  if ((speed < 7000) && (speed > 10400))
  {
    speed = 0;
  }
  res = f_close(&speed_file);
  if (res != FR_OK) { return 0; }
  
  return speed;
}

const u8 * delay_filename = "TIMINGS.TXT";

u16 ReadDelayFile()
{
  FIL delay_file;
  FRESULT res;
  u8 textbuf[80];
  u8 * s;
  
  u8  local_kw1281_max_init_attempts;
  u8  local_kw1281_max_byte_transmit_attempts; //wysylanie komunikatu do ecu, maksymalna liczba prób wyslania bajtu w przypadku nieotrzymania zanegowanej odpowiedzi
  u8  local_kw1281_interbyte_delay; //opoznienie przed wyslaniem kolejnych bajtow komunikatu lub opoznienie w wysylaniu zanegowanego potwierdzenia do ecu
  u8  local_kw1281_interbyte_delaymax; //maksymalny czas na przyjscie kolejnego bajtu z ecu lub maksymalny czas oczekiwania na przyjscie zanegowanego potwierdzenia od ecu
  u16 local_kw1281_intermessage_delay; //opoznienie przed wyslaniem nowego komunikatu
  u16 local_kw1281_intermessage_delaymax; //maksymalny czas oczekiwania na przyjscie nowego komunikatu od ecu

  res = f_open(&delay_file, delay_filename, FA_OPEN_EXISTING | FA_READ);
  if (res != FR_OK) 
  { 
    return 0; 
  }

  s = f_gets(textbuf, sizeof(textbuf), &delay_file);
  res = f_close(&delay_file);
  if ((NULL == s) || (res != FR_OK))
  {
    return 0;
  }

  s = strtok(textbuf, delimiters);
  if (NULL == s)
  {
    return 0;
  }
  local_kw1281_max_init_attempts = atoi(s);

  s = strtok(NULL, delimiters);
  if (NULL == s)
  {
    return 0;
  }
  local_kw1281_max_byte_transmit_attempts = atoi(s);
  //------
  s = strtok(NULL, delimiters);
  if (NULL == s)
  {
    return 0;
  }
  local_kw1281_interbyte_delay = atoi(s);

  s = strtok(NULL, delimiters);
  if (NULL == s)
  {
    return 0;
  }
  local_kw1281_interbyte_delaymax = atoi(s);
  //------
  s = strtok(NULL, delimiters);
  if (NULL == s)
  {
    return 0;
  }
  local_kw1281_intermessage_delay = atoi(s);

  s = strtok(NULL, delimiters);
  if (NULL == s)
  {
    return 0;
  }
  local_kw1281_intermessage_delaymax = atoi(s);

  if ((local_kw1281_max_init_attempts < 1) || (local_kw1281_max_init_attempts > 3) ||
      (local_kw1281_max_byte_transmit_attempts < 1) || (local_kw1281_max_byte_transmit_attempts > 3) ||
      (local_kw1281_intermessage_delay > 2000) ||
      (local_kw1281_intermessage_delaymax > 2000))
  {
    return 0;
  }

  kw1281_max_init_attempts = local_kw1281_max_init_attempts;
  kw1281_max_byte_transmit_attempts = local_kw1281_max_byte_transmit_attempts;
  kw1281_interbyte_delay = local_kw1281_interbyte_delay;
  kw1281_interbyte_delaymax = local_kw1281_interbyte_delaymax;
  kw1281_intermessage_delay = local_kw1281_intermessage_delay;
  kw1281_intermessage_delaymax = local_kw1281_intermessage_delaymax;

  return 255;
}


const u8 * timedate_filename = "SETTIME.TXT";

static u8 Str2Val(u8 * s, u8 maxVal)
{
  u8 val;
  if ((s==NULL) || (strlen(s)>2))
  {
    return 255;
  }
  if (!isdigit(s[0]) || ((strlen(s) > 1) && !isdigit(s[1])))
  {
    return 255;
  }
  val = atoi(s);
  if (val > maxVal)
  {
    return 255;
  }
  return val;
}


int ReadTimeDateFile()
{
  FIL config_file;
  FRESULT res;
  struct tm timedate;
  u8 textbuf[30];
  u8 * s;
  u8 error = 0;
  
  res = f_open(&config_file, timedate_filename, FA_OPEN_EXISTING | FA_READ);
  if (res != FR_OK) { return 0; }

  s = f_gets(textbuf, sizeof(textbuf), &config_file);
  if (NULL == s)
  {
    f_close(&config_file);
    return 0;
  }

  s = strtok(textbuf, delimiters);
  timedate.tm_hour = Str2Val(s, 23);
  if (255 == timedate.tm_hour)
  {
    error = 1;
  }

  s = strtok(NULL, delimiters);
  timedate.tm_min = Str2Val(s, 59);
  if (255 == timedate.tm_min)
  {
    error = 1;
  }

  s = strtok(NULL, delimiters);
  timedate.tm_sec = Str2Val(s, 59);
  if (255 == timedate.tm_sec)
  {
    error = 1;
  }

  if (error)
  {
    f_close(&config_file);
    return 0;    
  }

  s = f_gets(textbuf, sizeof(textbuf), &config_file);
  if (NULL == s)
  {
    f_close(&config_file);
    return 0;
  }

  s = strtok(textbuf, delimiters);
  timedate.tm_mday = Str2Val(s, 31);
  if (255 == timedate.tm_mday)
  {
    error = 1;
  }

  s = strtok(NULL, delimiters);
  timedate.tm_mon = Str2Val(s, 12);
  if (255 == timedate.tm_mon)
  {
    error = 1;
  }
  --timedate.tm_mon;

  s = strtok(NULL, delimiters);
  timedate.tm_year = Str2Val(s, 99);
  if (255 == timedate.tm_year)
  {
    error = 1;
  }
  timedate.tm_year += 100;

  if (error)
  {
    f_close(&config_file);
    return 0;    
  }

  res = f_close(&config_file);
  if (res != FR_OK) { return 0; }

  res = f_unlink(timedate_filename);
  if (res != FR_OK) { return 0; }

  RTC_Configuration();
  time_t rawtime;
  rawtime = mktime (&timedate);
  Time_Adjust(rawtime);  
  BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
  return 1;
}

const u8 * debug_filename = "DEBUG.TXT";

int ReadDebugFile()
{
  FIL speed_file;
  FRESULT res;

  res = f_open(&speed_file, debug_filename, FA_OPEN_EXISTING | FA_READ);
  if (res != FR_OK) { return 0; }

  res = f_close(&speed_file);
  if (res != FR_OK) { return 0; }
  
  return 1;
}


const u8 * can_filename = "CAN.TXT";

int ReadCanFile()
{
  FIL speed_file;
  FRESULT res;

  res = f_open(&speed_file, can_filename, FA_OPEN_EXISTING | FA_READ);
  if (res != FR_OK) { return 0; }

  res = f_close(&speed_file);
  if (res != FR_OK) { return 0; }
  
  return 1;
}