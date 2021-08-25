#include "config.h"

#include <Arduino.h>
#include <EEPROM.h>                   // Save config settings

int LEDpin = 2;
int LEDpin_inverted = 1;
int CONTROLpin = 2;

String node_type = "";
int node_id = 0;
String node_name = "";
String node_describe = "";
String node_description = "";

// Wifi Network Strings
String esid = "";
String epass = "";

// Web server authentication (leave blank for none)
String www_username = "";
String www_password = "";

// EMONCMS SERVER strings
String emoncms_server = "";
String emoncms_path = "";
String emoncms_node = "";
String emoncms_apikey = "";
String emoncms_fingerprint = "";

// // MQTT Settings
// String mqtt_server = "";
// int mqtt_port = 1883;
// String mqtt_topic = "";
// String mqtt_user = "";
// String mqtt_pass = "";
// String mqtt_feed_prefix = "";

// // Timer Settings 
// int timer_start1 = 0;
// int timer_stop1 = 0;
// int timer_start2 = 0;
// int timer_stop2 = 0;

int voltage_output = 0;

String ctrl_mode = "Off";
bool ctrl_update = 0;
bool ctrl_state = 0;
int time_offset = 0;

#define EEPROM_ESID_SIZE          32
#define EEPROM_EPASS_SIZE         64
#define EEPROM_EMON_API_KEY_SIZE  32
#define EEPROM_EMON_SERVER_SIZE   45
#define EEPROM_EMON_PATH_SIZE     16
#define EEPROM_EMON_NODE_SIZE     32
#define EEPROM_MQTT_SERVER_SIZE   45
#define EEPROM_MQTT_PORT_SIZE     2
#define EEPROM_MQTT_TOPIC_SIZE    32
#define EEPROM_MQTT_USER_SIZE     32
#define EEPROM_MQTT_PASS_SIZE     64
#define EEPROM_EMON_FINGERPRINT_SIZE  60
#define EEPROM_MQTT_FEED_PREFIX_SIZE  10
#define EEPROM_WWW_USER_SIZE      16
#define EEPROM_WWW_PASS_SIZE      16
#define EEPROM_TIMER_START1_SIZE  2
#define EEPROM_TIMER_STOP1_SIZE   2
#define EEPROM_TIMER_START2_SIZE  2
#define EEPROM_TIMER_STOP2_SIZE   2
#define EEPROM_VOLTAGE_OUTPUT_SIZE 2
#define EEPROM_TIME_OFFSET_SIZE   2
// TOTAL SIZE:                    589
#define EEPROM_SIZE 512



#define EEPROM_ESID_START         0
#define EEPROM_ESID_END           (EEPROM_ESID_START + EEPROM_ESID_SIZE)
#define EEPROM_EPASS_START        EEPROM_ESID_END
#define EEPROM_EPASS_END          (EEPROM_EPASS_START + EEPROM_EPASS_SIZE)
#define EEPROM_EMON_API_KEY_START EEPROM_EPASS_END
#define EEPROM_EMON_API_KEY_END   (EEPROM_EMON_API_KEY_START + EEPROM_EMON_API_KEY_SIZE)
#define EEPROM_EMON_SERVER_START  EEPROM_EMON_API_KEY_END
#define EEPROM_EMON_SERVER_END    (EEPROM_EMON_SERVER_START + EEPROM_EMON_SERVER_SIZE)
#define EEPROM_EMON_NODE_START    EEPROM_EMON_SERVER_END
#define EEPROM_EMON_NODE_END      (EEPROM_EMON_NODE_START + EEPROM_EMON_NODE_SIZE)
#define EEPROM_MQTT_SERVER_START  EEPROM_EMON_NODE_END
#define EEPROM_MQTT_SERVER_END    (EEPROM_MQTT_SERVER_START + EEPROM_MQTT_SERVER_SIZE)
#define EEPROM_MQTT_PORT_START    EEPROM_MQTT_SERVER_END
#define EEPROM_MQTT_PORT_END      (EEPROM_MQTT_PORT_START + EEPROM_MQTT_PORT_SIZE)
#define EEPROM_MQTT_TOPIC_START   EEPROM_MQTT_PORT_END
#define EEPROM_MQTT_TOPIC_END     (EEPROM_MQTT_TOPIC_START + EEPROM_MQTT_TOPIC_SIZE)
#define EEPROM_MQTT_USER_START    EEPROM_MQTT_TOPIC_END
#define EEPROM_MQTT_USER_END      (EEPROM_MQTT_USER_START + EEPROM_MQTT_USER_SIZE)
#define EEPROM_MQTT_PASS_START    EEPROM_MQTT_USER_END
#define EEPROM_MQTT_PASS_END      (EEPROM_MQTT_PASS_START + EEPROM_MQTT_PASS_SIZE)
#define EEPROM_EMON_FINGERPRINT_START  EEPROM_MQTT_PASS_END
#define EEPROM_EMON_FINGERPRINT_END    (EEPROM_EMON_FINGERPRINT_START + EEPROM_EMON_FINGERPRINT_SIZE)
#define EEPROM_MQTT_FEED_PREFIX_START  EEPROM_EMON_FINGERPRINT_END
#define EEPROM_MQTT_FEED_PREFIX_END    (EEPROM_MQTT_FEED_PREFIX_START + EEPROM_MQTT_FEED_PREFIX_SIZE)
#define EEPROM_WWW_USER_START     EEPROM_MQTT_FEED_PREFIX_END
#define EEPROM_WWW_USER_END       (EEPROM_WWW_USER_START + EEPROM_WWW_USER_SIZE)
#define EEPROM_WWW_PASS_START     EEPROM_WWW_USER_END
#define EEPROM_WWW_PASS_END       (EEPROM_WWW_PASS_START + EEPROM_WWW_PASS_SIZE)
#define EEPROM_EMON_PATH_START    EEPROM_WWW_PASS_END
#define EEPROM_EMON_PATH_END      (EEPROM_EMON_PATH_START + EEPROM_EMON_PATH_SIZE)

#define EEPROM_TIMER_START1_START EEPROM_EMON_PATH_END
#define EEPROM_TIMER_START1_END   (EEPROM_TIMER_START1_START + EEPROM_TIMER_START1_SIZE)
#define EEPROM_TIMER_STOP1_START  EEPROM_TIMER_START1_END
#define EEPROM_TIMER_STOP1_END    (EEPROM_TIMER_STOP1_START + EEPROM_TIMER_STOP1_SIZE)
#define EEPROM_TIMER_START2_START EEPROM_TIMER_STOP1_END
#define EEPROM_TIMER_START2_END   (EEPROM_TIMER_START2_START + EEPROM_TIMER_START2_SIZE)
#define EEPROM_TIMER_STOP2_START  EEPROM_TIMER_START2_END
#define EEPROM_TIMER_STOP2_END    (EEPROM_TIMER_STOP2_START + EEPROM_TIMER_STOP2_SIZE)

#define EEPROM_VOLTAGE_OUTPUT_START  EEPROM_TIMER_STOP2_END
#define EEPROM_VOLTAGE_OUTPUT_END    (EEPROM_VOLTAGE_OUTPUT_START + EEPROM_VOLTAGE_OUTPUT_SIZE)

#define EEPROM_TIME_OFFSET_START  EEPROM_VOLTAGE_OUTPUT_END
#define EEPROM_TIME_OFFSET_END    (EEPROM_TIME_OFFSET_START + EEPROM_TIME_OFFSET_SIZE)
// -------------------------------------------------------------------
// Reset EEPROM, wipes all settings
// -------------------------------------------------------------------
void ResetEEPROM(){
  //DEBUG.println("Erasing EEPROM");
  for (int i = 0; i < EEPROM_SIZE; ++i) {
    EEPROM.write(i, 0);
    //DEBUG.print("#");
  }
  EEPROM.commit();
}

void EEPROM_read_string(int start, int count, String & val) {
  for (int i = 0; i < count; ++i){
    byte c = EEPROM.read(start+i);
    if (c != 0 && c != 255)
      val += (char) c;
  }
}

void EEPROM_write_string(int start, int count, String val) {
  for (int i = 0; i < count; ++i){
    if (i<val.length()) {
      EEPROM.write(start+i, val[i]);
    } else {
      EEPROM.write(start+i, 0);
    }
  }
}

void EEPROM_read_int(int start, int & val) {
  byte high = EEPROM.read(start);
  byte low = EEPROM.read(start+1);
  val=word(high,low);
}

void EEPROM_write_int(int start, int val) {
  EEPROM.write(start,highByte(val));
  EEPROM.write(start+1,lowByte(val));
}

// -------------------------------------------------------------------
// Load saved settings from EEPROM
// -------------------------------------------------------------------
void config_load_settings()
{
  EEPROM.begin(EEPROM_SIZE);

  // Load WiFi values
  EEPROM_read_string(EEPROM_ESID_START, EEPROM_ESID_SIZE, esid);
  EEPROM_read_string(EEPROM_EPASS_START, EEPROM_EPASS_SIZE, epass);

  // EmonCMS settings
  EEPROM_read_string(EEPROM_EMON_API_KEY_START, EEPROM_EMON_API_KEY_SIZE,
                     emoncms_apikey);
  EEPROM_read_string(EEPROM_EMON_SERVER_START, EEPROM_EMON_SERVER_SIZE,
                     emoncms_server);
  EEPROM_read_string(EEPROM_EMON_PATH_START, EEPROM_EMON_PATH_SIZE,
                     emoncms_path);
  EEPROM_read_string(EEPROM_EMON_NODE_START, EEPROM_EMON_NODE_SIZE,
                     emoncms_node);
  EEPROM_read_string(EEPROM_EMON_FINGERPRINT_START,
                     EEPROM_EMON_FINGERPRINT_SIZE, emoncms_fingerprint);

  // MQTT settings
//   EEPROM_read_string(EEPROM_MQTT_SERVER_START, EEPROM_MQTT_SERVER_SIZE, mqtt_server);
//   EEPROM_read_int(EEPROM_MQTT_PORT_START, mqtt_port);
//   if (mqtt_port==0) mqtt_port = 1883; // apply a default port
  
//   EEPROM_read_string(EEPROM_MQTT_TOPIC_START, EEPROM_MQTT_TOPIC_SIZE, mqtt_topic);
//   EEPROM_read_string(EEPROM_MQTT_FEED_PREFIX_START, EEPROM_MQTT_FEED_PREFIX_SIZE, mqtt_feed_prefix);
//   EEPROM_read_string(EEPROM_MQTT_USER_START, EEPROM_MQTT_USER_SIZE, mqtt_user);
//   EEPROM_read_string(EEPROM_MQTT_PASS_START, EEPROM_MQTT_PASS_SIZE, mqtt_pass);

  // Web server credentials
  EEPROM_read_string(EEPROM_WWW_USER_START, EEPROM_WWW_USER_SIZE, www_username);
  EEPROM_read_string(EEPROM_WWW_PASS_START, EEPROM_WWW_PASS_SIZE, www_password);

  // Read timer settings
//   EEPROM_read_int(EEPROM_TIMER_START1_START, timer_start1);
//   EEPROM_read_int(EEPROM_TIMER_STOP1_START, timer_stop1);
//   EEPROM_read_int(EEPROM_TIMER_START2_START, timer_start2);
//   EEPROM_read_int(EEPROM_TIMER_STOP2_START, timer_stop2);

  EEPROM_read_int(EEPROM_VOLTAGE_OUTPUT_START, voltage_output);
  
  EEPROM_read_int(EEPROM_TIME_OFFSET_START, time_offset);
}

void config_save_emoncms(String server, String path, String node, String apikey, String fingerprint)
{
  emoncms_server = server;
  emoncms_path = path;
  emoncms_node = node;
  emoncms_apikey = apikey;
  emoncms_fingerprint = fingerprint;

  // save apikey to EEPROM
  EEPROM_write_string(EEPROM_EMON_API_KEY_START, EEPROM_EMON_API_KEY_SIZE, emoncms_apikey);

  // save emoncms server to EEPROM max 45 characters
  EEPROM_write_string(EEPROM_EMON_SERVER_START, EEPROM_EMON_SERVER_SIZE, emoncms_server);

  // save emoncms server to EEPROM max 16 characters
  EEPROM_write_string(EEPROM_EMON_PATH_START, EEPROM_EMON_PATH_SIZE, emoncms_path);

  // save emoncms node to EEPROM max 32 characters
  EEPROM_write_string(EEPROM_EMON_NODE_START, EEPROM_EMON_NODE_SIZE, emoncms_node);

  // save emoncms HTTPS fingerprint to EEPROM max 60 characters
  EEPROM_write_string(EEPROM_EMON_FINGERPRINT_START, EEPROM_EMON_FINGERPRINT_SIZE, emoncms_fingerprint);

  EEPROM.commit();
}

void config_save_admin(String user, String pass)
{
  www_username = user;
  www_password = pass;

  EEPROM_write_string(EEPROM_WWW_USER_START, EEPROM_WWW_USER_SIZE, user);
  EEPROM_write_string(EEPROM_WWW_PASS_START, EEPROM_WWW_PASS_SIZE, pass);

  EEPROM.commit();
}

void config_save_voltage_output(int qvoltage_output, int save_to_eeprom)
{
  voltage_output = qvoltage_output;
  
  if (save_to_eeprom) {
    EEPROM_write_int(EEPROM_VOLTAGE_OUTPUT_START, voltage_output);
    EEPROM.commit();
  }
}

void config_save_wifi(String qsid, String qpass)
{
  esid = qsid;
  epass = qpass;

  EEPROM_write_string(EEPROM_ESID_START, EEPROM_ESID_SIZE, qsid);
  EEPROM_write_string(EEPROM_EPASS_START, EEPROM_EPASS_SIZE, qpass);

  EEPROM.commit();
}

void config_reset()
{
  ResetEEPROM();
}
