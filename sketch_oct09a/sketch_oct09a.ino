#include <myIOT2.h>
#include <homeController.h>

myIOT2 iot;
homeCtl controller;

// ~~~ Software Flags ~~~
#define MAN_MODE true
#define PARAM_PRESET 3

// ~~~ Debug ~~~
#define EN_WIFI true
#define DEBUG_MODE true
#define RETAINED_MSG true
#define BurnOUT true

#if BurnOUT
#include "soc/soc.h"          // disable   detector
#include "soc/rtc_cntl_reg.h" // disable   detector
#endif

#if MAN_MODE == true
#include "manual_mode.h"
#endif

#include "myIOT_settings.h"
#include "parameterRead.h"

#if DEBUG_MODE == true
#include "debug_mode.h"
#endif

bool bootProcess_OK = true;

void post_telemetry_2MQTT(Cotroller_Ent_telemetry &MSG) /* get telemetry from any entity */
{
#if EN_WIFI
  char msg[100];

  if (MSG.type == WIN_ENT)
  {
    Win_props win_props;

    controller.get_entity_prop(MSG.type /* window */, MSG.id /* win entity id */, win_props);
    if (!win_props.virtCMD)
    {
      sprintf(msg, "[%s]: [WIN#%d] [%s] turned [%s]", controller.WinTrigs[MSG.trig], MSG.id, win_props.name, controller.WinStates[MSG.state]);
      iot.pub_msg(msg);

#if RETAINED_MSG
      MQTT_clear_retained(win_props.name);
#endif
      MQTT_update_state(MSG.state, win_props.name); /* Retain State */
    }
    else
    {
      MQTT_send_virtCMD(controller.winMQTTcmds[MSG.state], win_props.name);
      MQTT_notify_virtCMD(win_props.name, controller.winMQTTcmds[MSG.state], controller.WinTrigs[MSG.trig], msg);
    }
  }
  else if (MSG.type == SW_ENT)
  {
    SW_props sw_props;
    controller.get_entity_prop(MSG.type, MSG.id, sw_props);

    if (!sw_props.virtCMD)
    {
      char msg2[30]{};

      if (MSG.state == 1 && MSG.timeout > 0) /* On, with timeout */
      {
        char t[20];
        iot.convert_epoch2clock((int)((MSG.timeout + 250) / 1000), 0, t);
        sprintf(msg2, "timeout [%s]", t);
      }
      else if (MSG.state == 1 && MSG.timeout == 0) /* On, without timeout*/
      {
        strcpy(msg2, "timeout [No]");
      }
      else if (MSG.state == 0) /* Off*/
      {
        strcpy(msg2, "");
      }
      else
      {
        strcpy(msg2, "err");
      }

      sprintf(msg, "[%s]: [SW#%d] [%s] turned [%s] %s", controller.SW_Types[MSG.trig], sw_props.id, sw_props.name, controller.SW_MQTT_cmds[MSG.state], msg2);
      iot.pub_msg(msg);

#if RETAINED_MSG
      MQTT_clear_retained(sw_props.name);
#endif
      MQTT_update_state(MSG.state, sw_props.name); /* Retain State */
    }
    else
    {
      MQTT_send_virtCMD(controller.SW_MQTT_cmds[MSG.state], sw_props.name);
      MQTT_notify_virtCMD(sw_props.name, controller.SW_MQTT_cmds[MSG.state], controller.SW_Types[MSG.trig], msg);
    }
  }
#endif
}
void new_telemetry_handler()
{
  Cotroller_Ent_telemetry localMSG;
  controller.get_telemetry(localMSG);

#if DEBUG_MODE
  print_telemetry(localMSG);
#endif

  post_telemetry_2MQTT(localMSG);
  controller.clear_telemetryMSG();
}

void setup()
{
#if BurnOUT
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable   detector
#endif
#if DEBUG_MODE
  init_Serial_DBG_MODE();
#endif
  read_all_parameters();
#if EN_WIFI
  startIOTservices();
#endif
}
void loop()
{
  if (controller.loop())
  {
    new_telemetry_handler();
  }
#if EN_WIFI
  iot.looper();
#endif
}
