#ifndef ZCL_APP_H
#define ZCL_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "version.h"
#include "zcl.h"
#include "zcl_general.h"

/*********************************************************************
 * CONSTANTS
 */
#define APP_REPORT_DELAY ((uint32)60 * (uint32)1000) // 1 minute
// Application Events
#define APP_REPORT_EVT 0x0001
#define APP_SAVE_ATTRS_EVT 0x0002
#define APP_BTN_CLICK_EVT 0x0004
#define APP_RING_RUN_EVT 0x0008
#define APP_BTN_HOLD_EVT 0x0010
#define APP_RING_STOP_EVT 0x0020
#define APP_TALK_START_EVT 0x0040
  
/*********************************************************************
 * MACROS
 */
#define NW_APP_CONFIG 0x0402

#define R ACCESS_CONTROL_READ
// ACCESS_CONTROL_AUTH_WRITE
#define RW (R | ACCESS_CONTROL_WRITE | ACCESS_CONTROL_AUTH_WRITE)
#define RR (R | ACCESS_REPORTABLE)

//READ REPORT WRITE
#define RRW (R | ACCESS_REPORTABLE | ACCESS_CONTROL_WRITE | ACCESS_CONTROL_AUTH_WRITE)
  
#define BASIC ZCL_CLUSTER_ID_GEN_BASIC
#define GEN_ON_OFF ZCL_CLUSTER_ID_GEN_ON_OFF
#define POWER_CFG ZCL_CLUSTER_ID_GEN_POWER_CFG
   


//Intercom
#define ZCL_INTERCOM    0x0101

#define ATTRID_STATE                0x0050
#define ATTRID_MODEOPEN             0x0051
#define ATTRID_MODESOUND            0x0052
#define ATTRID_TIMERING             0x0053                                  
#define ATTRID_TIMETALK             0x0054
#define ATTRID_TIMEOPEN             0x0055
#define ATTRID_TIMEBELL             0x0057
#define ATTRID_TIMEREPORT           0x0056

#define LED_PIN         HAL_LED_1
#define CATCH_PIN       HAL_LED_2
#define ANSWER_PIN      HAL_LED_3
#define HANDSET_PIN     HAL_LED_4

#define TIMER_RESTART           TRUE
#define TIMER_START             FALSE

#define ZIC_WORKMODE_NEVER              0
#define ZIC_WORKMODE_ONCE               1
#define ZIC_WORKMODE_ALWAYS             2
#define ZIC_WORKMODE_DROP               3


#define ZIC_WORKSTATE_IDLE               0
#define ZIC_WORKSTATE_RING               1
#define ZIC_WORKSTATE_TALK               2
#define ZIC_WORKSTATE_OPEN               3
#define ZIC_WORKSTATE_DROP               4

#define ZCL_UINT8 ZCL_DATATYPE_UINT8
#define ZCL_UINT16 ZCL_DATATYPE_UINT16
#define ZCL_INT16 ZCL_DATATYPE_INT16
#define ZCL_INT8  ZCL_DATATYPE_INT8
#define ZCL_INT32 ZCL_DATATYPE_INT32
#define ZCL_UINT32 ZCL_DATATYPE_UINT32
#define ZCL_SINGLE ZCL_DATATYPE_SINGLE_PREC
/*********************************************************************
 * TYPEDEFS
 */
  
typedef enum {
    Never,
    Once,
    Always,
    Drop
} WorkMode_t;

typedef enum {
    Idle,
    Ring,
    Talk,
    Open,
    Droped
} WorkState_t;

typedef struct {
    WorkMode_t ModeOpen;
    uint8 ModeSound;   
    uint8 TimeRing;
    uint8 TimeTalk;
    uint8 TimeOpen;
    uint8 TimeBell;
    uint8 TimeReport;
} application_config_t;

typedef struct {
    WorkState_t State;
    uint8 RingRunStep;
    uint32 pressTime;
} device_state_t;


/*
typedef enum {
  UNKNOWN, SENSEAIR, MHZ19
} SensorType_t;
*/
/*********************************************************************
 * VARIABLES
 */

extern SimpleDescriptionFormat_t zclApp_FirstEP;
extern CONST zclAttrRec_t zclApp_AttrsFirstEP[];
extern CONST uint8 zclApp_AttrsCount;


extern const uint8 zclApp_ManufacturerName[];
extern const uint8 zclApp_ModelId[];
extern const uint8 zclApp_PowerSource;

extern uint8 zclApp_BatteryVoltage;
extern uint8 zclApp_BatteryPercentageRemainig;
extern uint16 zclApp_BatteryVoltageRawAdc;

extern application_config_t zclApp_Config;
extern device_state_t zclApp_State;
// APP_TODO: Declare application specific attributes here

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Initialization for the task
 */
extern void zclApp_Init(byte task_id);

/*
 *  Event Process for the task
 */
extern UINT16 zclApp_event_loop(byte task_id, UINT16 events);

extern void zclApp_ResetAttributesToDefaultValues(void);

/*********************************************************************
 *********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_APP_H */
