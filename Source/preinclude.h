#define TC_LINKKEY_JOIN
#define NV_INIT
#define NV_RESTORE

#define TP2_LEGACY_ZC
// patch sdk
//#define ZDSECMGR_TC_ATTEMPT_DEFAULT_KEY TRUE

#define NWK_AUTO_POLL
#define MULTICAST_ENABLED FALSE

#define ZCL_READ
#define ZCL_WRITE
#define ZCL_BASIC
#define ZCL_IDENTIFY
#define ZCL_REPORTING_DEVICE
#define ZCL_ON_OFF

#define DISABLE_GREENPOWER_BASIC_PROXY
#define BDB_FINDING_BINDING_CAPABILITY_ENABLED 1
#define BDB_REPORTING TRUE


#define HAL_BUZZER FALSE
#define HAL_KEY TRUE
#define ISR_KEYINTERRUPT



#define HAL_LED TRUE
#define HAL_LCD FALSE

#define BLINK_LEDS TRUE


#if !defined(HAL_BOARD_TARGET) && !defined(HAL_BOARD_CHDTECH_DEV)
#error "Board type must be defined"
#endif

#if defined( ZIC_BATTERY_MODE )
    #define POWER_SAVING
#endif

#if defined( HAL_BOARD_TARGET )
    // Income ring - P0_0
    #define KEY_INCOME_PORT HAL_KEY_PORT0
    #define HAL_KEY_P0_INPUT_PINS BV(0)
    #define HAL_KEY_P0_INPUT_PINS_EDGE HAL_KEY_RISING_EDGE
    // Button - P2_0
    #define KEY1_PORT HAL_KEY_PORT2
    #define HAL_KEY_P2_INPUT_PINS BV(0)
    #define HAL_KEY_P2_INPUT_PINS_EDGE HAL_KEY_FALLING_EDGE
    #define INT_HEAP_LEN 2200//2256
#elif defined( HAL_BOARD_CHDTECH_DEV )
    // Income ring - P0_1
    #define KEY_INCOME_PORT HAL_KEY_PORT0
    #define HAL_KEY_P0_INPUT_PINS BV(1)
    #define HAL_KEY_P0_INPUT_PINS_EDGE HAL_KEY_FALLING_EDGE
    // Button - P2_0
    #define KEY1_PORT HAL_KEY_PORT2
    #define HAL_KEY_P2_INPUT_PINS BV(0)
    #define HAL_KEY_P2_INPUT_PINS_EDGE HAL_KEY_FALLING_EDGE
    #define DO_DEBUG_UART
    #define INT_HEAP_LEN 2060
#endif

#define BTN_HOLD_TIME 2000
#define FACTORY_RESET_HOLD_TIME_LONG 10000

#if defined( DO_DEBUG_UART )
    #define HAL_UART_ISR 0
    #define HAL_UART TRUE
    #define HAL_UART_DMA 1
#endif

#include "hal_board_cfg.h"
