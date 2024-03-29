
#include "AF.h"
#include "OSAL.h"
#include "OSAL_Clock.h"
#include "OSAL_PwrMgr.h"
#include "ZComDef.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "math.h"

#include "nwk_util.h"
#include "zcl.h"
#include "zcl_app.h"
#include "zcl_diagnostic.h"
#include "zcl_general.h"
#include "zcl_ms.h"

#include "bdb.h"
#include "bdb_interface.h"
#include "bdb_touchlink.h"
#include "bdb_touchlink_target.h"

#include "gp_interface.h"

#include "Debug.h"

#include "OnBoard.h"

#include "commissioning.h"
#include "factory_reset.h"
/* HAL */

#include "hal_drivers.h"

#include "hal_key.h"
#include "hal_led.h"
#include "hal_adc.h"

#include "utils.h"

#include "battery.h"

#include "version.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclApp_TaskID;
#if defined( ZIC_BATTERY_MODE )
static byte isRingOn = false;
static byte isLedOn = false;
#endif

// for Old LED
static byte showMode = false;
static byte showSound = false;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */
//void user_delay_ms(uint32_t period);
//void user_delay_ms(uint32_t period) { MicroWait(period * 1000); }
/*********************************************************************
 * LOCAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void zclApp_BasicResetCB(void);
static void zclApp_RestoreAttributesFromNV(void);
static void zclApp_SaveAttributesToNV(void);
static void zclApp_HandleKeys(byte portAndAction, byte keyCode);
static void zclApp_ControlPinsInit(void);
static ZStatus_t zclApp_ReadWriteAuthCB(afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8 oper);

static void zclApp_OneReport(void);
static void zclApp_ConfigInit(bool restart);

static void zclApp_BtnClicks(byte count);
static void zclApp_RingRun(void);
static void zclApp_TalkStart(void);
static void zclApp_RingEnd(void);
static void zclApp_WorkWithLEDs(void);

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclApp_CmdCallbacks = {
    zclApp_BasicResetCB, // Basic Cluster Reset command
    NULL,                // Identify Trigger Effect command
    NULL,                // On/Off cluster commands
    NULL,                // On/Off cluster enhanced command Off with Effect
    NULL,                // On/Off cluster enhanced command On with Recall Global Scene
    NULL,                // On/Off cluster enhanced command On with Timed Off
    NULL,                // RSSI Location command
    NULL                 // RSSI Location Response command
};

void zclApp_Init(byte task_id) {
    zclApp_RestoreAttributesFromNV();

    zclApp_TaskID = task_id;

    bdb_RegisterSimpleDescriptor(&zclApp_FirstEP);

    zclGeneral_RegisterCmdCallbacks(zclApp_FirstEP.EndPoint, &zclApp_CmdCallbacks);

    zcl_registerAttrList(zclApp_FirstEP.EndPoint, zclApp_AttrsCount, zclApp_AttrsFirstEP);

    zcl_registerReadWriteCB(zclApp_FirstEP.EndPoint, NULL, zclApp_ReadWriteAuthCB);
    zcl_registerForMsg(zclApp_TaskID);
    RegisterForKeys(zclApp_TaskID);

    LREP("Build %s \r\n", zclApp_DateCodeNT);

    zclApp_ConfigInit(TIMER_START);
    
    zclApp_ControlPinsInit();

    IO_IMODE_PORT_PIN(0,0,IO_TRI);
        
    #if defined( ZIC_BATTERY_MODE )
        ZMacSetTransmitPower(TX_PWR_PLUS_4); // set 4dBm
    #endif
}

static void zclApp_ControlPinsInit (void) {
    CATCH_SSR &= ~CATCH_BV; // Set PIN as general I/O port functions
    CATCH_DDR |= CATCH_BV;  // Set the port transmission mode for PIN to output

    ANSWER_SSR &= ~ANSWER_BV;
    ANSWER_DDR |= ANSWER_BV;

    HANDSET_SSR &= ~HANDSET_BV;
    HANDSET_DDR |= HANDSET_BV;

    P0SEL &= ~0x01;         // Set P0_0 as general I/O port function
    P0DIR &= ~0x01;         // Set the port transmission mode of P0_0 to input
  // ? P0INP &= ~0x01;         //Set the port input mode of P0_0 to: pull up/down
}


static void zclApp_HandleKeys(byte portAndAction, byte keyCode) {

    if (portAndAction & KEY_INCOME_PORT) { //P0 Ring //S1 P0_1  TODO add check Income pin
      //exit old stop timer
      osal_stop_timerEx(zclApp_TaskID, APP_RING_STOP_EVT);
      osal_start_timerEx(zclApp_TaskID, APP_RING_STOP_EVT, zclApp_Config.TimeBell * 1000);
      if (portAndAction & HAL_KEY_PRESS) {
          //start ring
          if (zclApp_State.State == Idle) {
              #if defined( ZIC_BATTERY_MODE )
                osal_pwrmgr_task_state(zclApp_TaskID, PWRMGR_HOLD);
                isRingOn = true;
              #endif    
              LREPMaster("Ring start\r\n");
              osal_start_timerEx(zclApp_TaskID, APP_RING_RUN_EVT, 50);
              afAddrType_t inderect_DstAddr = {.addrMode = (afAddrMode_t)AddrNotPresent, .endPoint = 0, .addr.shortAddr = 0};
              zclGeneral_SendOnOff_CmdOn(zclApp_FirstEP.EndPoint, &inderect_DstAddr, FALSE, bdb_getZCLFrameCounter());
          }
      }
    }

    if (portAndAction & KEY1_PORT) { //P2 Btn //S2 P2_0  TODO add check BUTTON pin
      zclFactoryResetter_HandleKeys(portAndAction, keyCode);
      if (portAndAction & HAL_KEY_PRESS) {
        LREP("Key pressed. Clicks - %d\r\n", zclApp_State.clicks);
        if (zclApp_State.clicks < 2) {
            zclApp_State.clicks++;
        }

        osal_start_timerEx(zclApp_TaskID, APP_BTN_HOLD_EVT, BTN_HOLD_TIME);
        osal_stop_timerEx(zclApp_TaskID, APP_BTN_CLICK_EVT);
      }
      if (portAndAction & HAL_KEY_RELEASE) {
        LREPMaster("Key released\r\n");

        osal_stop_timerEx(zclApp_TaskID, APP_BTN_HOLD_EVT);
        if (zclApp_State.clicks == 1) {
          osal_start_timerEx(zclApp_TaskID, APP_BTN_CLICK_EVT, 250);
        }
        if (zclApp_State.clicks == 2) {
          osal_start_timerEx(zclApp_TaskID, APP_BTN_DOUBLE_EVT, 50);
        }
      } 
    }
}


uint16 zclApp_event_loop(uint8 task_id, uint16 events) {
    LREP("events 0x%x \r\n", events);
    if (events & SYS_EVENT_MSG) {
        afIncomingMSGPacket_t *MSGpkt;
        while ((MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(zclApp_TaskID))) {
            LREP("MSGpkt->hdr.event 0x%X clusterId=0x%X\r\n", MSGpkt->hdr.event, MSGpkt->clusterId);
            switch (MSGpkt->hdr.event) {
            case KEY_CHANGE:
                zclApp_HandleKeys(((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys);
                break;

            case ZCL_INCOMING_MSG:
                if (((zclIncomingMsg_t *)MSGpkt)->attrCmd) {
                    osal_mem_free(((zclIncomingMsg_t *)MSGpkt)->attrCmd);
                }
                break;

            default:
                break;
            }

            // Release the memory
            osal_msg_deallocate((uint8 *)MSGpkt);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    if (events & APP_REPORT_EVT) {
        LREPMaster("APP_REPORT_EVT\r\n");
        zclApp_OneReport();
        return (events ^ APP_REPORT_EVT);
    }

    if (events & APP_SAVE_ATTRS_EVT) {
        LREPMaster("APP_SAVE_ATTRS_EVT\r\n");
        zclApp_SaveAttributesToNV();
        zclApp_ConfigInit(TIMER_RESTART);
        return (events ^ APP_SAVE_ATTRS_EVT);
    }
    
    if (events & APP_BTN_CLICK_EVT) {
        LREPMaster("APP_BTN_CLICK_EVT\r\n");
        zclApp_BtnClicks(1);
        return (events ^ APP_BTN_CLICK_EVT);
    }
    
    if (events & APP_RING_RUN_EVT) {
        LREPMaster("APP_RING_RUN_EVT\r\n");
        zclApp_RingRun();
        return (events ^ APP_RING_RUN_EVT);
    }

    if (events & APP_RING_STOP_EVT) {
        LREPMaster("APP_RING_STOP_EVT\r\n");
        // if answer started - break not allowed
        if (zclApp_State.State == Ring) {
            zclApp_RingEnd();
        }
        return (events ^ APP_RING_STOP_EVT);
    }
    
    if (events & APP_TALK_START_EVT) {
        LREPMaster("APP_TALK_START_EVT\r\n");
        zclApp_TalkStart();
        return (events ^ APP_TALK_START_EVT);
    }
    
    if (events & APP_WORK_LED_EVT) {
        LREPMaster("APP_WORK_LED_EVT\r\n");
        zclApp_WorkWithLEDs();
        return (events ^ APP_WORK_LED_EVT);
    }

    if (events & APP_BTN_HOLD_EVT) {
        LREPMaster("APP_BTN_HOLD_EVT\r\n");
        //#if !defined( ZIC_BATTERY_MODE )
        zclApp_BtnClicks(255);
        //#endif
        return (events ^ APP_BTN_HOLD_EVT);
    }

    if (events & APP_BTN_DOUBLE_EVT) {
        LREPMaster("APP_BTN_DOUBLE_EVT\r\n");
        zclApp_BtnClicks(2);
        return (events ^ APP_BTN_DOUBLE_EVT);
    }

    return 0;
}


static void zclApp_RingRun(void) {
    LREP("zclApp_State.State %d\r\n", zclApp_State.State);
    uint32 timeRingStart = osal_GetSystemClock();
    // timeout for next ring step
    // 250 - default ring timeout in Never mode
    uint32 timeout_value = 250;

    switch (zclApp_State.State) {
    case Idle:
        zclApp_State.State = Ring;
        if (zclApp_Config.ModeOpen != Never) {
            timeout_value = zclApp_Config.TimeRing * 1000;
        }
        zclApp_OneReport();
        break; 
    case Ring:
        if (zclApp_Config.ModeOpen == Never) {
            break;
        }
        zclApp_State.State = Talk;
        timeout_value = zclApp_Config.TimeTalk * 1000;
        zclApp_TalkStart();
        break;
    case Talk:
        osal_stop_timerEx(zclApp_TaskID, APP_RING_STOP_EVT);
        if (zclApp_Config.ModeOpen == Once || zclApp_Config.ModeOpen == Always) {
            // open door
            timeout_value = zclApp_Config.TimeOpen * 1000;
            zclApp_State.State = Open;
            ANSWER_PIN = 0;
            zclApp_OneReport();
            break;
        }
        // No break here. End call without open
    case Open:
        // end call
        osal_stop_timerEx(zclApp_TaskID, APP_RING_STOP_EVT);
        zclApp_RingEnd();
        timeout_value = 0;
        break;
    }

    if (timeout_value != 0) {
        osal_start_timerEx(zclApp_TaskID, APP_RING_RUN_EVT, timeout_value);
    }
    osal_start_timerEx(zclApp_TaskID, APP_WORK_LED_EVT, 50);
}

static void zclApp_TalkStart(void) {
    LREPMaster("Talk start\r\n");
    zclApp_OneReport();
    ANSWER_PIN = 1;
    if (zclApp_Config.ModeSound == true) {
        HANDSET_PIN = 1;
    }
    else {
        CATCH_PIN = 0;
    }
}

static void zclApp_RingEnd(void) {
    LREPMaster("Ring end\r\n");
    CATCH_PIN = !zclApp_Config.ModeSound;
    HANDSET_PIN = !zclApp_Config.ModeSound;
    ANSWER_PIN = 0;
    osal_stop_timerEx(zclApp_TaskID, APP_RING_RUN_EVT);
    osal_start_timerEx(zclApp_TaskID, APP_WORK_LED_EVT, 50);
    zclApp_State.State = Idle;
    
    afAddrType_t inderect_DstAddr = {.addrMode = (afAddrMode_t)AddrNotPresent, .endPoint = 0, .addr.shortAddr = 0};  
    zclGeneral_SendOnOff_CmdOff(zclApp_FirstEP.EndPoint, &inderect_DstAddr, FALSE, bdb_getZCLFrameCounter());
    
    if (zclApp_Config.ModeOpen == Once) {
        zclApp_Config.ModeOpen = Never;
    }
    zclApp_OneReport(); 
    
    #if defined( ZIC_BATTERY_MODE )
      zclBattery_Report();
      isRingOn = false;
      if (isLedOn == false) {
          osal_pwrmgr_task_state(zclApp_TaskID, PWRMGR_CONSERVE);
      }
    #endif
}

static void zclApp_WorkWithLEDs(void) {
#if defined( ZIC_BATTERY_MODE )
    if (isLedOn != false) { // Off LEDs and goto sleep
        isLedOn = false;
        HalLedSet(HAL_LED_ALL, HAL_LED_MODE_OFF);
        if (isRingOn == false) {
            osal_pwrmgr_task_state(zclApp_TaskID, PWRMGR_CONSERVE);
        }
        return;
    }
    // On LEDs and hold power 2 sec.
    isLedOn = true;
    osal_pwrmgr_task_state(zclApp_TaskID, PWRMGR_HOLD);
    osal_start_timerEx(zclApp_TaskID, APP_WORK_LED_EVT, 2000);
#endif

    if (showSound & showMode) {
        showSound = false;
        showMode = false;
        HalLedBlink(OLD_LED, 1, 99, 2000);
    } else if (showSound) {
        showSound = false;
        HalLedBlink(OLD_LED, zclApp_Config.ModeSound + 1, 50, 750);
    } else if (showMode) {
        showMode = false;
        HalLedBlink(OLD_LED, zclApp_Config.ModeOpen + 1, 50, 250);
    }

  if (zclApp_Config.ModeOpen == Always) {
    HalLedSet(GREEN_LED, HAL_LED_MODE_ON);
  }
  if (zclApp_Config.ModeOpen != Always) {
    HalLedSet(GREEN_LED, HAL_LED_MODE_OFF);
  }
  if (zclApp_Config.ModeOpen == Once) {
    HalLedBlink(GREEN_LED, 0, 95, 500);
  }
  if (zclApp_Config.ModeOpen == Drop) {
    HalLedSet(RED_LED, HAL_LED_MODE_ON);
  }
  if (zclApp_Config.ModeOpen != Drop) {
    HalLedSet(RED_LED, HAL_LED_MODE_OFF);
  }
  if (zclApp_State.State != Idle) {
    HalLedSet(RED_LED, HAL_LED_MODE_BLINK);
  }
  if (zclApp_Config.ModeSound == true) {
    HalLedSet(BLUE_LED, HAL_LED_MODE_OFF);
  }
  else {
    HalLedSet(BLUE_LED, HAL_LED_MODE_ON);
  }
}

static void zclApp_BtnClicks(byte count) {
    zclApp_State.clicks = 0;
    osal_start_timerEx(zclApp_TaskID, APP_WORK_LED_EVT, 50);

    switch (count) {
    case 1:
        LREPMaster("Button click\r\n");
        if (zclApp_State.State == Idle) {
          // if we don`t have income ring - change mode
          if (zclApp_Config.ModeOpen < Drop) {
            zclApp_Config.ModeOpen++;
          }
          else {
            zclApp_Config.ModeOpen = Never;
          }
          zclApp_OneReport();
        }
        else {
          // if we have income ring, and mode is Never - open door
          if (zclApp_State.State == Ring && zclApp_Config.ModeOpen == Never) {
            zclApp_Config.ModeOpen = Once;
          }
        }
        showMode = true;
        break;
    case 2:
        LREPMaster("Button double\r\n");
        if (zclApp_State.State == Idle) {
          zclApp_Config.ModeSound = !zclApp_Config.ModeSound;
          HANDSET_PIN = !zclApp_Config.ModeSound;
          CATCH_PIN = !zclApp_Config.ModeSound;
          zclApp_OneReport();
        }
        showSound = true;
        break;
    case 255:
        LREPMaster("Button hold\r\n");
        if (zclApp_State.State == Idle) {
          // if we don`t have income ring - reset mode
          zclApp_Config.ModeSound = true;
          HANDSET_PIN = !zclApp_Config.ModeSound;
          CATCH_PIN = !zclApp_Config.ModeSound;
          zclApp_Config.ModeOpen = Never;
          zclApp_OneReport();
        }
        else {
          // if we have income ring - drop it
          if (zclApp_State.State == Ring) {
              // in stage "ring" - start talk
              osal_stop_timerEx(zclApp_TaskID, APP_RING_RUN_EVT);
              osal_start_timerEx(zclApp_TaskID, APP_RING_RUN_EVT, zclApp_Config.TimeTalk * 1000);
              zclApp_TalkStart();
          }
          // skip step "open", go to end call
          zclApp_State.State = Open;
          osal_start_timerEx(zclApp_TaskID, APP_RING_STOP_EVT, 250);
        }
        showMode = true;
        showSound = true;
        break;
    }
}

static void zclApp_OneReport(void) {
    bdb_RepChangedAttrValue(zclApp_FirstEP.EndPoint, ZCL_INTERCOM, ATTRID_STATE);
    bdb_RepChangedAttrValue(zclApp_FirstEP.EndPoint, ZCL_INTERCOM, ATTRID_MODEOPEN);
    bdb_RepChangedAttrValue(zclApp_FirstEP.EndPoint, ZCL_INTERCOM, ATTRID_MODESOUND);
}

static void zclApp_BasicResetCB(void) {
    LREPMaster("BasicResetCB\r\n");
    zclApp_ResetAttributesToDefaultValues();
    zclApp_SaveAttributesToNV();
}

static ZStatus_t zclApp_ReadWriteAuthCB(afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8 oper) {
    LREPMaster("AUTH CB called\r\n");
    osal_start_timerEx(zclApp_TaskID, APP_SAVE_ATTRS_EVT, 200);
    return ZSuccess;
}

static void zclApp_SaveAttributesToNV(void) {
    uint8 writeStatus = osal_nv_write(NW_APP_CONFIG, 0, sizeof(application_config_t), &zclApp_Config);
    LREP("Saving attributes to NV write=%d\r\n", writeStatus);
}

static void zclApp_ConfigInit(bool restart) {
    if (restart) {
      LREP("Stop report timer event\r\n");
      osal_stop_timerEx(zclApp_TaskID, APP_REPORT_EVT);
      osal_clear_event(zclApp_TaskID, APP_REPORT_EVT);
    }
    uint32 ReportInterval = (uint32)zclApp_Config.TimeReport * (uint32)60;
    LREP("Start report with interval %d seconds\r\n", ReportInterval);
    osal_start_reload_timer(zclApp_TaskID, APP_REPORT_EVT, ((uint32)ReportInterval*(uint32)1000));
    osal_start_timerEx(zclApp_TaskID, APP_WORK_LED_EVT, 50);

    if (zclApp_State.State == Idle) {
      HANDSET_PIN = !zclApp_Config.ModeSound;
      CATCH_PIN = !zclApp_Config.ModeSound;
      ANSWER_PIN = 0;
    }
}

static void zclApp_RestoreAttributesFromNV(void) {
    uint8 status = osal_nv_item_init(NW_APP_CONFIG, sizeof(application_config_t), NULL);
    LREP("Restoring attributes from NV  status=%d \r\n", status);
    if (status == NV_ITEM_UNINIT) {
        uint8 writeStatus = osal_nv_write(NW_APP_CONFIG, 0, sizeof(application_config_t), &zclApp_Config);
        LREP("NV was empty, writing %d\r\n", writeStatus);
    }
    if (status == ZSUCCESS) {
        LREPMaster("Reading from NV\r\n");
        osal_nv_read(NW_APP_CONFIG, 0, sizeof(application_config_t), &zclApp_Config);
    }
}

/****************************************************************************
****************************************************************************/
