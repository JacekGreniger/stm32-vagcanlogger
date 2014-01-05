#ifndef CONFIG_H
#define CONFIG_H

/* VAGLOGGER CONFIGURATION */

//#define HW_WITHOUT_CAN //for HW published in EP
#define CAN_FEATURE_ENABLED

//#define UART1_DEBUG
//#define DEBUG_LOG_CAN_FRAME
//#define DEBUG_LOG

#define SPI2_DMA_TRANSFER //don't disable
/* ----------------------------- */

#ifdef CAN_FEATURE_ENABLED
  #ifdef HW_WITHOUT_CAN
    #error CAN_FEATURE cannot be enabled on HW without CAN
  #endif
#endif

#ifdef DEBUG_LOG_CAN_FRAME
  #ifndef DEBUG_LOG
    #error Set also flag DEBUG_LOG
  #endif
#endif

#ifdef CAN_FEATURE_ENABLED
  #define HW_VERSION "VAGCANLOGGER, hw_ver: 2"
#elif defined HW_WITHOUT_CAN
  #define HW_VERSION "VAGLOGGER, hw_ver: 1"
#else
  #define HW_VERSION "VAGLOGGER, hw_ver: 2"
#endif

#define SW_VERSION "1.0"

#endif