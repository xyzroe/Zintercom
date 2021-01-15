#include "AF.h"
#include "OSAL.h"
#include "ZComDef.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_ms.h"

#include "zcl_app.h"

#include "battery.h"
#include "version.h"

#include "bdb_touchlink.h"
#include "bdb_touchlink_target.h"
#include "stub_aps.h"

/*********************************************************************
 * CONSTANTS
 */

#define APP_DEVICE_VERSION 2
#define APP_FLAGS 0

#define APP_HWVERSION 1
#define APP_ZCLVERSION 1

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Global attributes
const uint16 zclApp_clusterRevision_all = 0x0002;

// Basic Cluster
const uint8 zclApp_HWRevision = APP_HWVERSION;
const uint8 zclApp_ZCLVersion = APP_ZCLVERSION;

const uint8 zclApp_ApplicationVersion = 3;
const uint8 zclApp_StackVersion = 4;

//{lenght, 'd', 'a', 't', 'a'}
const uint8 zclApp_ManufacturerName[] = {6, 'x', 'y', 'z', 'r', 'o', 'e'};
const uint8 zclApp_ModelId[] = {13, 'D', 'I', 'Y', '_', 'Z', 'i', 'n', 't', 'e', 'r', 'c', 'o', 'm'};

#if defined( ZIC_BATTERY_MODE ) 
const uint8 zclApp_PowerSource = POWER_SOURCE_BATTERY; 
#define DEFAULT_TimeReport     30 //minutes
#else
const uint8 zclApp_PowerSource = POWER_SOURCE_DC; 
#define DEFAULT_TimeReport     1 //minutes
#endif

#define DEFAULT_ModeOpen       Never 
#define DEFAULT_ModeSound      TRUE 

#define DEFAULT_TimeRing       3 //seconds to ring, before answer
#define DEFAULT_TimeTalk       1 //seconds to talk, before open
#define DEFAULT_TimeOpen       2 //seconds to hold open


application_config_t zclApp_Config = {
    .ModeOpen = DEFAULT_ModeOpen,
    .ModeSound = DEFAULT_ModeSound,
    .TimeRing = DEFAULT_TimeRing,
    .TimeTalk = DEFAULT_TimeTalk,
    .TimeOpen = DEFAULT_TimeOpen,
    .TimeReport = DEFAULT_TimeReport,
};

device_state_t zclApp_State = {
    .State = Idle,
    .RingRunStep = 0,
};


/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */

CONST zclAttrRec_t zclApp_AttrsFirstEP[] = {
    {BASIC, {ATTRID_BASIC_ZCL_VERSION, ZCL_UINT8, R, (void *)&zclApp_ZCLVersion}},
    {BASIC, {ATTRID_BASIC_APPL_VERSION, ZCL_UINT8, R, (void *)&zclApp_ApplicationVersion}},
    {BASIC, {ATTRID_BASIC_STACK_VERSION, ZCL_UINT8, R, (void *)&zclApp_StackVersion}},
    {BASIC, {ATTRID_BASIC_HW_VERSION, ZCL_UINT8, R, (void *)&zclApp_HWRevision}},
    {BASIC, {ATTRID_BASIC_MANUFACTURER_NAME, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_ManufacturerName}},
    {BASIC, {ATTRID_BASIC_MODEL_ID, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_ModelId}},
    {BASIC, {ATTRID_BASIC_DATE_CODE, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_DateCode}},
    {BASIC, {ATTRID_BASIC_POWER_SOURCE, ZCL_DATATYPE_ENUM8, R, (void *)&zclApp_PowerSource}},
   
    
//#if defined( ZIC_BATTERY_MODE )    
    {POWER_CFG, {ATTRID_POWER_CFG_BATTERY_VOLTAGE, ZCL_UINT8, RR, (void *)&zclBattery_Voltage}},
    /**
    * FYI: calculating battery percentage can be tricky, since this device can be powered from 2xAA or 1xCR2032 batteries
    * */
    {POWER_CFG, {ATTRID_POWER_CFG_BATTERY_PERCENTAGE_REMAINING, ZCL_UINT8, RR, (void *)&zclBattery_PercentageRemainig}},
//#endif
    
    {BASIC, {ATTRID_BASIC_SW_BUILD_ID, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_DateCode}},
    {BASIC, {ATTRID_CLUSTER_REVISION, ZCL_UINT16, R, (void *)&zclApp_clusterRevision_all}},

    


    {ZCL_INTERCOM, {ATTRID_STATE, ZCL_DATATYPE_ENUM8, RRW, (void *)&zclApp_State.State}},
    
    {ZCL_INTERCOM, {ATTRID_MODEOPEN, ZCL_DATATYPE_ENUM8, RRW, (void *)&zclApp_Config.ModeOpen}},
    {ZCL_INTERCOM, {ATTRID_MODESOUND, ZCL_DATATYPE_BOOLEAN, RRW, (void *)&zclApp_Config.ModeSound}},
    
    {ZCL_INTERCOM, {ATTRID_TIMERING, ZCL_UINT8, RW, (void *)&zclApp_Config.TimeRing}},
    {ZCL_INTERCOM, {ATTRID_TIMETALK, ZCL_UINT8, RW, (void *)&zclApp_Config.TimeTalk}},
    {ZCL_INTERCOM, {ATTRID_TIMEOPEN, ZCL_UINT8, RW, (void *)&zclApp_Config.TimeOpen}},
    {ZCL_INTERCOM, {ATTRID_TIMEREPORT, ZCL_UINT8, RW, (void *)&zclApp_Config.TimeReport}},
};


uint8 CONST zclApp_AttrsCount = (sizeof(zclApp_AttrsFirstEP) / sizeof(zclApp_AttrsFirstEP[0]));

const cId_t zclApp_InClusterList[] = {ZCL_CLUSTER_ID_GEN_BASIC};

#define APP_MAX_INCLUSTERS (sizeof(zclApp_InClusterList) / sizeof(zclApp_InClusterList[0]))

const cId_t zclApp_OutClusterList[] = {ZCL_INTERCOM, GEN_ON_OFF};


#define APP_MAX_OUT_CLUSTERS (sizeof(zclApp_OutClusterList) / sizeof(zclApp_OutClusterList[0]))


SimpleDescriptionFormat_t zclApp_FirstEP = {
    1,                             //  int Endpoint;
    ZCL_HA_PROFILE_ID,             //  uint16 AppProfId[2];
    ZCL_HA_DEVICEID_SIMPLE_SENSOR, //  uint16 AppDeviceId[2];
    APP_DEVICE_VERSION,            //  int   AppDevVer:4;
    APP_FLAGS,                     //  int   AppFlags:4;
    APP_MAX_INCLUSTERS,            //  byte  AppNumInClusters;
    (cId_t *)zclApp_InClusterList, //  byte *pAppInClusterList;
    APP_MAX_OUT_CLUSTERS,          //  byte  AppNumInClusters;
    (cId_t *)zclApp_OutClusterList //  byte *pAppInClusterList;
};


void zclApp_ResetAttributesToDefaultValues(void) {
    zclApp_Config.ModeOpen = DEFAULT_ModeOpen;
    zclApp_Config.ModeSound = DEFAULT_ModeSound;
    zclApp_Config.TimeRing = DEFAULT_TimeRing;
    zclApp_Config.TimeTalk = DEFAULT_TimeTalk;
    zclApp_Config.TimeOpen  = DEFAULT_TimeOpen;
    zclApp_Config.TimeReport  = DEFAULT_TimeReport;
}