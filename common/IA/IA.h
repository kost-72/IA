// ensure this library description is only included once
#ifndef IA_h
#define IA_h

//#include <Arduino.h>

#define I2C_PMS_SLAVE_ADDRESS 0x4
#define I2C_MHZ_SLAVE_ADDRESS 0x5

const byte MAX_COUNT_REPEATERS = 240;

const int SERVICE_MOTION = 1;
const int SERVICE_SWITCH = 2;
const int SERVICE_TEMPERATURE = 4;
const int SERVICE_BATTERY_LEVEL = 8;
const int SERVICE_LIGHT = 16;
const int SERVICE_OUTLET = 32;
const int SERVICE_LEAK = 64;
const int SERVICE_LIGHT_BULB = 128;
const int SERVICE_WINDOW_COVERING = 256;
const int SERVICE_1_CONTACT = 512;
const int SERVICE_2_CONTACT = 1024;
const int SERVICE_REPEATER = 2048;
const int SERVICE_AIR_QUALITY = 4096;
const int SERVICE_HUMIDITY = 8192;
const int SERVICE_DOOR = 16384;
//32768

const byte ID_PAIRING = 0;
const byte ID_REACHABILITY_PING = 1;
const byte ID_MOTION_ACTION = 2;
const byte ID_SWITCH_ACTION = 3;
const byte ID_OUTLET_ACTION = 4;
const byte ID_TEMPERATURE = 5;
const byte ID_BATTERY_LEVEL = 6;
const byte ID_LIGHT_LOW = 7;
const byte ID_LIGHT_MEDIUM = 8;
const byte ID_LIGHT_HIGH = 9;
const byte ID_LEAK_STATE = 10;
const byte ID_LIGHT_BULB_ACTION = 11;
const byte ID_WINDOW_COVERING_CURRENT_POSITION = 12;
const byte ID_WINDOW_COVERING_TARGET_POSITION = 13;
const byte ID_WINDOW_COVERING_POSITION_STATE = 14;
const byte ID_CONTACT_1_STATE = 15;
const byte ID_CONTACT_2_STATE = 16;
const byte ID_AIR_QUALITY = 17;
const byte ID_HUMIDITY = 18;
const byte ID_DOOR_CURRENT_POSITION = 19;
const byte ID_DOOR_TARGET_POSITION = 20;
const byte ID_DOOR_POSITION_STATE = 21;
const byte ID_PM_10_STANDARD = 22;
const byte ID_PM_25_STANDARD = 23;
const byte ID_PM_100_STANDARD = 24;
const byte ID_PM_10_ENV = 25;
const byte ID_PM_25_ENV = 26;
const byte ID_PM_100_ENV = 27;
const byte ID_PPM_CO2_LOW = 28;
const byte ID_PPM_CO2_MEDIUM = 29;
const byte ID_PPM_CO2_HIGH = 30;
const byte ID_TEST = 31;

const float KOEF_LIGHT_LOW = 1.0;
const float KOEF_LIGHT_MEDIUM = 10.0;
const float KOEF_LIGHT_HIGH = 100.0;

const float KOEF_PPM_CO2_LOW = 10.0;
const float KOEF_PPM_CO2_MEDIUM = 20.0;
const float KOEF_PPM_CO2_HIGH = 40.0; // maybe 100 here

const byte WINDOW_COVERING_POSITION_STATE_DECREASING = 0;
const byte WINDOW_COVERING_POSITION_STATE_INCREASING = 1;
const byte WINDOW_COVERING_POSITION_STATE_STOPPED = 2;

const byte DOOR_POSITION_STATE_DECREASING = 0;
const byte DOOR_POSITION_STATE_INCREASING = 1;
const byte DOOR_POSITION_STATE_STOPPED = 2;

const byte RESULT_OK = 1;
const byte RESULT_FAILED = 2;

const byte PM_10_STANDARD = 1;
const byte PM_25_STANDARD = 2;
const byte PM_100_STANDARD = 3;
const byte PM_10_ENV = 4;
const byte PM_25_ENV = 5;
const byte PM_100_ENV = 6;
const byte PPM_CO2 = 7;

struct Payload { byte accessoryId; int service; byte id; byte value; byte control; };

void setControl(Payload *payload) {
  byte control = payload->accessoryId + payload->service + payload->id + payload->value;
  payload->control = control;
}

boolean checkControl(Payload payload) {
  byte control = payload.accessoryId + payload.service + payload.id + payload.value;
  return control == payload.control;
}

void addRepeaterService(Payload *payload) {
  int service = payload->service | SERVICE_REPEATER;
  payload->service = service;
}

void removeRepeaterService(Payload *payload) {
  int service = payload->service ^ SERVICE_REPEATER;
  payload->service = service;
}

#endif


