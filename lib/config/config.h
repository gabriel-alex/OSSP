#ifndef _EMONESP_CONFIG_H
#define _EMONESP_CONFIG_H

#include <Arduino.h>

// -------------------------------------------------------------------
// Load and save the EmonESP config.
//
// This initial implementation saves the config to the EEPROM area of flash
// -------------------------------------------------------------------

extern int LEDpin;
extern int LEDpin_inverted;
extern int CONTROLpin;

// Global config varables
extern String node_type;
extern int node_id;
extern String node_name;
extern String node_describe;
extern String node_description;

// Wifi Network Strings
extern String esid;
extern String epass;

// Web server authentication (leave blank for none)
extern String www_username;
extern String www_password;

// EMONCMS SERVER strings
extern String emoncms_server;
extern String emoncms_path;
extern String emoncms_node;
extern String emoncms_apikey;
extern String emoncms_fingerprint;

// // MQTT Settings
// extern String mqtt_server;
// extern int mqtt_port;
// extern String mqtt_topic;
// extern String mqtt_user;
// extern String mqtt_pass;
// extern String mqtt_feed_prefix;

// // Timer Settings 
// extern int timer_start1;
// extern int timer_stop1;
// extern int timer_start2;
// extern int timer_stop2;
// extern int time_offset;

extern int voltage_output;

extern String ctrl_mode;
extern bool ctrl_update;
extern bool ctrl_state;

// -------------------------------------------------------------------
// Load saved settings
// -------------------------------------------------------------------
extern void config_load_settings();

// -------------------------------------------------------------------
// Save the EmonCMS server details
// -------------------------------------------------------------------
extern void config_save_emoncms(String server, String path, String node, String apikey, String fingerprint);

// -------------------------------------------------------------------
// Save the admin/web interface details
// -------------------------------------------------------------------
extern void config_save_admin(String user, String pass);

// -------------------------------------------------------------------
// Save the admin/web interface details
// -------------------------------------------------------------------
// extern void config_save_timer(int start1, int stop1, int start2, int stop2, int voltage_output, int qtime_offset);
// extern void config_save_voltage_output(int qvoltage_output, int save_to_eeprom);

// -------------------------------------------------------------------
// Save the Wifi details
// -------------------------------------------------------------------
extern void config_save_wifi(String qsid, String qpass);


// -------------------------------------------------------------------
// Reset the config back to defaults
// -------------------------------------------------------------------
extern void config_reset();

#endif // _EMONESP_CONFIG_H
