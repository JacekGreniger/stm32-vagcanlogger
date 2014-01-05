#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "stm32f10x_lib.h"
#include "filesystem/integer.h"
#include "filesystem/diskio.h"
#include "filesystem/ff.h"

extern u8 config[3][4];

int ReadConfig();
int GetNextFileNumber();
int CreateLogFile(int file_number, FIL *log_file);
int CloseLogFile(FIL * log_file);
u16 ReadSpeedFile();
u16 ReadDelayFile();
int ReadTimeDateFile();
int ReadDebugFile();

#endif