#include <RF24.h>
#include <avr/sleep.h>
#include <Vcc.h>
#include <IA.h>
//#include <TinuDHT.h>
#include <dht.h>

// code for ATTinyCore v1.1.4 or v1.4.1 (both tested), RF24 v1.3.0:
//  Board = ATtiny 24/44/84
//  B.O.D. = disabled (this is default), TODO need to investigate is that important or not
//  LTO = enabled to reduce need of memory
//  Pin Mapping = Counterclockwise
//  Clock = 8 Mhz; 1 Mhz needs less power, need additional testing; according to additional investigation it seems here should be 8 Mhz
//  Clock = 4 Mhz for DHT11, 8 Mhz for DHT22
//  Chip = ATtiny84

// Programmer = Arduino AS ISP

// in case of compilation error comment '#define SOFTSPI' in RF24_config.h"
// uncomment configuration header for only one accessory before uploading sketch

// migrate to RF24 v1.3.11

// TODO maybe resend functionality should be removed, investigate

#define DEBUG

//#define SERVICE_DOOR_D 
//#define SERVICE_WINDOW_COVERING_D  
//#define SERVICE_AIR_QUALITY_D      // SERVICE_AIR_QUALITY

//#define SERVICE_MOTION_D           // SERVICE_MOTION
//#define SERVICE_LEAK_D             // SERVICE_LEAK
//#define SERVICE_BATTERY_LEVEL_D    // SERVICE_BATTERY_LEVEL
//#define SERVICE_TEMPERATURE_D      // SERVICE_TEMPERATURE
//#define SERVICE_CONTACT_D          // SERVICE_1_CONTACT, SERVICE_2_CONTACT
//#define SERVICE_LIGHT_D            // SERVICE_LIGHT
//#define SERVICE_RELAY_D            // SERVICE_LIGHT_BULB, SERVICE_OUTLET
//#define SERVICE_REPEATER_D         // SERVICE_REPEATER

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

/*
const byte ACCESSORY_ID = 1; // COMPLETED+
#define SERVICE_RELAY_D   
#define SERVICE_BATTERY_LEVEL_D  
const int ACCESSORY_SERVICE = SERVICE_LIGHT_BULB | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const boolean reverseRelayValue = false;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
*/

/*
const byte ACCESSORY_ID = 2; // COMPLETED+
#define SERVICE_RELAY_D
#define SERVICE_REPEATER_D
const int ACCESSORY_SERVICE = SERVICE_OUTLET;
const float VCC_CORRECTION = 1.0;
const boolean reverseRelayValue = true;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
*/

/*
const byte ACCESSORY_ID = 3; // COMPLETED+
#define SERVICE_TEMPERATURE_D           
#define SERVICE_BATTERY_LEVEL_D    
const int ACCESSORY_SERVICE = SERVICE_TEMPERATURE | SERVICE_BATTERY_LEVEL;
//const float TEMPERATURE_CORRECTION = -3.0; // C degree, calibrated
//const float VCC_CORRECTION = 1.053391; // calibrated
const float TEMPERATURE_CORRECTION = -2.0;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 2; // via repeater 2
const boolean useDht = false;
#define PIN_POWER 9
#define channel 76 // 1-st floor
*/

/*
const byte ACCESSORY_ID = 4; // COMPLETED+
#define SERVICE_MOTION_D           
#define SERVICE_BATTERY_LEVEL_D    
#define SERVICE_LIGHT_D            
const int ACCESSORY_SERVICE = SERVICE_MOTION | SERVICE_LIGHT | SERVICE_BATTERY_LEVEL;
const float TEMPERATURE_CORRECTION = 0.0;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
const int VALUE_MIN_LDR_DAY = 15;             // 150, 50, 20 too much; 15 ok; 5, 2 not enough
const unsigned long DELAY_READ_LDR = 5;        // 100, 50, 20 10, too much; 5 ok; 2, 1 not enough
#define PIN_POWER 9
const boolean usePowerPin = true;
*/

/*
const byte ACCESSORY_ID = 5; // COMPLETED+
#define SERVICE_WINDOW_COVERING_D   
const int ACCESSORY_SERVICE = SERVICE_WINDOW_COVERING;
#define WINDOW_COVERING_MOTOR_RPM 50
#define WINDOW_COVERING_MOTOR_STOCK_DIAMETER_MM 12
#define WINDOW_COVERING_LENGTH_MM 870
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
*/

/*
const byte ACCESSORY_ID = 6; // COMPLETED+
#define SERVICE_LEAK_D   
#define SERVICE_TEMPERATURE_D  
#define SERVICE_REPEATER_D   
const int ACCESSORY_SERVICE = SERVICE_LEAK | SERVICE_TEMPERATURE;
const float TEMPERATURE_CORRECTION = 0.0;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 10; // via repeater 10
#define PIN_POWER 9
*/

/*
const byte ACCESSORY_ID = 7; // COMPLETED+
#define SERVICE_RELAY_D   
#define SERVICE_BATTERY_LEVEL_D  
#define SERVICE_REPEATER_D 
const int ACCESSORY_SERVICE = SERVICE_LIGHT_BULB | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const boolean reverseRelayValue = false;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
*/

/*
const byte ACCESSORY_ID = 8; // COMPLETED+
#define SERVICE_RELAY_D  
const int ACCESSORY_SERVICE = SERVICE_OUTLET;
const boolean reverseRelayValue = true;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
*/

/*
const byte ACCESSORY_ID = 9; // COMPLETED+
#define SERVICE_RELAY_D  
const int ACCESSORY_SERVICE = SERVICE_OUTLET;
const boolean reverseRelayValue = true;
const byte CONTROLLER_RADIO_ID = 254; // 2-nd floor
#define channel 83 // 2-nd floor
*/

/*
const byte ACCESSORY_ID = 10; // COMPLETED+
#define SERVICE_RELAY_D  
#define SERVICE_REPEATER_D  
const int ACCESSORY_SERVICE = SERVICE_OUTLET;
const boolean reverseRelayValue = true;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
*/

/*
const byte ACCESSORY_ID = 11; // COMPLETED+
#define SERVICE_RELAY_D
const int ACCESSORY_SERVICE = SERVICE_OUTLET;
const boolean reverseRelayValue = true;
const byte CONTROLLER_RADIO_ID = 2; // via repeater 2
*/

/*
const byte ACCESSORY_ID = 12; // COMPLETED+
#define SERVICE_WINDOW_COVERING_D
const int ACCESSORY_SERVICE = SERVICE_WINDOW_COVERING;
#define WINDOW_COVERING_MOTOR_RPM 50
#define WINDOW_COVERING_MOTOR_STOCK_DIAMETER_MM 12
#define WINDOW_COVERING_LENGTH_MM 1100
const byte CONTROLLER_RADIO_ID = 254; // 2-nd floor
//const byte CONTROLLER_RADIO_ID = 10; // via repeater 10 - not stable!!
#define channel 83 // 2-nd floor
*/

/*
const byte ACCESSORY_ID = 13; // COMPLETED+
#define SERVICE_CONTACT_D   
#define SERVICE_BATTERY_LEVEL_D
const int ACCESSORY_SERVICE = SERVICE_1_CONTACT | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
*/

/*
const byte ACCESSORY_ID = 14; // COMPLETED+
#define SERVICE_MOTION_D           
#define SERVICE_BATTERY_LEVEL_D   
#define SERVICE_LIGHT_D           
const int ACCESSORY_SERVICE = SERVICE_MOTION | SERVICE_LIGHT | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
const int VALUE_MIN_LDR_DAY = 15;             // 150, 50, 20 too much; 15 ok; 5, 2 not enough
const unsigned long DELAY_READ_LDR = 5;        // 100, 50, 20 10, too much; 5 ok; 2, 1 not enough
#define PIN_POWER 9
const boolean usePowerPin = true;
*/

/*
const byte ACCESSORY_ID = 15; // COMPLETED+
#define SERVICE_RELAY_D
const int ACCESSORY_SERVICE = SERVICE_OUTLET;
const boolean reverseRelayValue = true;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
*/

/*
const byte ACCESSORY_ID = 16; // COMPLETED+
#define SERVICE_RELAY_D
const int ACCESSORY_SERVICE = SERVICE_OUTLET;
const boolean reverseRelayValue = true;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
//const byte CONTROLLER_RADIO_ID = 254; // 2-nd floor
//#define channel 83 // 2-nd floor
*/

/*
const byte ACCESSORY_ID = 17; // COMPLETED
#define SERVICE_CONTACT_D   
#define SERVICE_BATTERY_LEVEL_D
const int ACCESSORY_SERVICE = SERVICE_1_CONTACT | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
*/

/*
const byte ACCESSORY_ID = 18; // COMPLETED+
#define SERVICE_RELAY_D   
#define SERVICE_BATTERY_LEVEL_D  
const int ACCESSORY_SERVICE = SERVICE_LIGHT_BULB | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const boolean reverseRelayValue = false;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
*/

/*
const byte ACCESSORY_ID = 19; // COMPLETED+
#define SERVICE_RELAY_D
#define SERVICE_BATTERY_LEVEL_D
const int ACCESSORY_SERVICE = SERVICE_LIGHT_BULB | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const boolean reverseRelayValue = false;
const byte CONTROLLER_RADIO_ID = 7; // via repeater 7
#define channel 76 // 1-st floor
*/

/*
const byte ACCESSORY_ID = 20; // COMPLETED+
#define SERVICE_MOTION_D           
#define SERVICE_BATTERY_LEVEL_D    
#define SERVICE_LIGHT_D            
const int ACCESSORY_SERVICE = SERVICE_MOTION | SERVICE_LIGHT | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
const int VALUE_MIN_LDR_DAY = 15;             // 150, 50, 20 too much; 15 ok; 5, 2 not enough
const unsigned long DELAY_READ_LDR = 5;        // 100, 50, 20 10, too much; 5 ok; 2, 1 not enough
#define PIN_POWER 9
const boolean usePowerPin = true;
*/

/*
const byte ACCESSORY_ID = 21; // COMPLETED+
#define SERVICE_MOTION_D           
#define SERVICE_BATTERY_LEVEL_D    
#define SERVICE_LIGHT_D            
const int ACCESSORY_SERVICE = SERVICE_MOTION | SERVICE_LIGHT | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 254; // 2-nd floor
#define channel 83 // 2-nd floor
const int VALUE_MIN_LDR_DAY = 15;             // 150, 50, 20 too much; 15 ok; 5, 2 not enough
const unsigned long DELAY_READ_LDR = 5;        // 100, 50, 20 10, too much; 5 ok; 2, 1 not enough
#define PIN_POWER 9
const boolean usePowerPin = true;
*/

/*
const byte ACCESSORY_ID = 22; // COMPLETED+
#define SERVICE_CONTACT_D    
#define SERVICE_BATTERY_LEVEL_D     
const int ACCESSORY_SERVICE = SERVICE_1_CONTACT | SERVICE_2_CONTACT | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 6; // via repeater 6
#define channel 76 // 1-st floor
*/

/*
const byte ACCESSORY_ID = 23; // COMPLETED+
#define SERVICE_HUMIDITY_D      // Clock = 4 Mhz for DHT11 
#define SERVICE_TEMPERATURE_D     
#define SERVICE_AIR_QUALITY_D     
//#define SERVICE_REPEATER_D    
const int ACCESSORY_SERVICE = SERVICE_HUMIDITY | SERVICE_TEMPERATURE | SERVICE_AIR_QUALITY;
const float TEMPERATURE_CORRECTION = -4.0; // C degree
const float VCC_CORRECTION = 1.0;
const float HUMIDITY_CORRECTION = 10.0; 
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
const boolean useDht = false;
//#define PIN_POWER 9
*/

/*
const byte ACCESSORY_ID = 24; // COMPLETED+
#define SERVICE_RELAY_D   
#define SERVICE_BATTERY_LEVEL_D  
const int ACCESSORY_SERVICE = SERVICE_LIGHT_BULB | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const boolean reverseRelayValue = false;
const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
#define channel 76 // 1-st floor
*/


const byte ACCESSORY_ID = 25; // COMPLETED+
#define SERVICE_RELAY_D   
#define SERVICE_BATTERY_LEVEL_D  
const int ACCESSORY_SERVICE = SERVICE_LIGHT_BULB | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const boolean reverseRelayValue = false;
const byte CONTROLLER_RADIO_ID = 254; // 2-nd floor
#define channel 83 // 2-nd floor


/*
const byte ACCESSORY_ID = 26; // COMPLETED+
#define SERVICE_SWITCH_D           
#define SERVICE_BATTERY_LEVEL_D             
const int ACCESSORY_SERVICE = SERVICE_SWITCH | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
//const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
const byte CONTROLLER_RADIO_ID = 254; // 2-nd floor
//#define channel 76 // 1-st floor
#define channel 83 // 2-nd floor
*/

/*
const byte ACCESSORY_ID = 27; // COMPLETED+
#define SERVICE_SWITCH_D           
#define SERVICE_BATTERY_LEVEL_D             
const int ACCESSORY_SERVICE = SERVICE_SWITCH | SERVICE_BATTERY_LEVEL;
const float VCC_CORRECTION = 1.0;
const byte CONTROLLER_RADIO_ID = 2; // via repeater 2
#define channel 76 // 1-st floor
//#define channel 83 // 2-nd floor
*/

/*
const byte ACCESSORY_ID = 112; // TEST
#define SERVICE_DOOR_D        
const int ACCESSORY_SERVICE = SERVICE_DOOR;
#define DOOR_SERVO_DEGREES 60 
//#define DOOR_SERVO_DEGREES_CORRECTION 5 - defined in attiny85 servo program
#define DOOR_SERVO_SECS 0.5 // 0.5 secs - doDoor first send increasing/decreasing state and then next time doDoor send stopped state
#define DISTANCE_CM_ULTRASONIC_1 150 
#define DISTANCE_CM_ULTRASONIC_2 350 
const byte CONTROLLER_RADIO_ID = 254; // 2-nd floor
*/

const byte RADIO_ID = ACCESSORY_ID;

#if defined(SERVICE_REPEATER_D) || defined(SERVICE_DOOR_D)
  #include <EEPROM.h>
#endif  

#if defined(SERVICE_DOOR_D)
  //#include <Servo.h>         // radio not working together at all
  //#include <SoftwareServo.h> // radio not working together, not receiving radio payloads when servo moves
#endif

#if defined(SERVICE_AIR_QUALITY_D) 
  //#define I2C_PULLUP 1
  #define I2C_SLOWMODE 1
  #define SDA_PORT PORTB
  #define SDA_PIN 1 
  #define SCL_PORT PORTB
  #define SCL_PIN 0 
  
  #include "SoftI2CMaster.h" 
#endif

//const byte CONTROLLER_RADIO_ID = 255; // 1-st floor
//const byte CONTROLLER_RADIO_ID = 254; // 2-nd floor

// radio
#define PIN_CE 7
#define PIN_CSN 2
#define PIN_IRQ 3
volatile boolean interrupted0 = false;

volatile byte WDT_PERIOD = 16; // 8 secs, default, period = value x 0.5 secs

// pairing
#define WDT_TIMEOUT_PAIRING 16 // 8 secs, pause = value x 0.5 secs
volatile int timerWDTPairing = 0;

// reachability ping
#ifdef DEBUG
  #define WDT_TIMEOUT_REACHABILITY_PING 128 // 64 secs, pause = value x 0.5 secs, this timeout must be several times greater than other data related timeouts
  //#define WDT_TIMEOUT_SHORT_REACHABILITY_PING 10 // 5 secs, pause = value x 0.5 secs, this timeout must be several times greater than other data related timeouts
#else
  #define WDT_TIMEOUT_REACHABILITY_PING 7680 // 64 mins, pause = value x 0.5 secs, this timeout must be several times greater than other data related timeouts
  //#define WDT_TIMEOUT_SHORT_REACHABILITY_PING 10 // 5 secs, pause = value x 0.5 secs, this timeout must be several times greater than other data related timeouts
#endif

volatile int timerWDTReachabilityPing = 0;
//volatile int timerWDTShortReachabilityPing = 0;

volatile boolean interrupted1 = false;

// motion, leak or switch
#if defined(SERVICE_MOTION_D) || defined(SERVICE_LEAK_D)
  #define PIN_PIR_LEAK 0
#endif

// motion
#if defined(SERVICE_MOTION_D)  
  #define WDT_TIMEOUT_MOTION 32 // 16 secs, pause = value x 0.5 secs
  #define WDT_TIMEOUT_MOTION_LIGHT 1 // 0.5 secs, pause = value x 0.5 secs
  volatile int timerWDTMotion = 0;
  volatile int timerWDTMotionLight = 0;
  volatile byte lastMotionActionValue = -1;
#endif  

// leak
#if defined(SERVICE_LEAK_D)
  #define WDT_TIMEOUT_LEAK 1 // 0.5 secs, pause = value x 0.5 secs
  volatile int timerWDTLeak = 0;
  volatile byte leakStateSent = 0;
  const byte leakStateSendCount = 3;
#endif  

// switch
/*#if defined(SERVICE_SWITCH_D)
  #define WDT_TIMEOUT_SWITCH 1 // 0.5 secs, pause = value x 0.5 secs
  volatile int timerWDTSwitch = 0;
  volatile byte switchActionSent = 0;
  const byte switchActionSendCount = 3;
  volatile boolean switchOn = false;
#endif*/

// light
#if defined(SERVICE_LIGHT_D)
  #define PIN_LDR A2
  const byte LDR_READING_COUNT = 10;
  #ifdef DEBUG
  #define WDT_TIMEOUT_LIGHT 32 // 16 secs, pause = value x 0.5 secs
  #else
  #define WDT_TIMEOUT_LIGHT 1080 // 9 mins, pause = value x 0.5 secs
  #endif
  volatile int timerWDTLight = 0;

  #define LDR_REF_RESISTANCE  10000 // 10 kOhm
  #define LUX_CALC_SCALAR     12518931
  #define LUX_CALC_EXPONENT   -1.405
#endif  

// outlet or bulb
#if defined(SERVICE_RELAY_D) || defined(SERVICE_SWITCH_D)
  #define PIN_RELAY_SWITCH 1
#endif  

// temperature
#if defined(SERVICE_TEMPERATURE_D)
  #define PIN_TEMPERATURE A0
  const byte TEMPERATURE_READING_COUNT = 10;
#endif 
#if defined(SERVICE_TEMPERATURE_D) || defined(SERVICE_HUMIDITY_D)  
  #ifdef DEBUG
    #define WDT_TIMEOUT_TEMPERATURE 64 // 32 secs, pause = value x 0.5 secs
  #else
    #define WDT_TIMEOUT_TEMPERATURE 1920 // 16 mins, pause = value x 0.5 secs
  #endif
  volatile int timerWDTTemperature = 0;
#endif  

// dht
#if defined(SERVICE_HUMIDITY_D)  
  #define USE_DHT11 //  Clock = 4 Mhz for DHT11, Clock = 8 Mhz for DHT22
  #define PIN_DHT 9 // don't change!
  //#define PIN_DHT 10 // don't change!
#endif  

// air quality
#if defined(SERVICE_AIR_QUALITY_D)
  #define PIN_MQ135 A2
  #define MQ135_R_LOAD 1.0 // 1 kOhm
  #define MQ135_R_ZERO 232.64
  #define MQ135_PAR_A 116.6020682
  #define MQ135_PAR_B 2.769034857
  #ifdef DEBUG
  #define WDT_TIMEOUT_AIR_QUALITY 32 // 16 secs, pause = value x 0.5 secs
  #else
  #define WDT_TIMEOUT_AIR_QUALITY 1080 // 9 mins, pause = value x 0.5 secs
  #endif
  volatile int timerWDTAirQuality = 0;
  #define WDT_TIMEOUT_AIR_QUALITY_CHANGE 1 // 0.5 secs, pause = value x 0.5 secs
  volatile int timerWDTAirQualityChange = 0;
  
  const byte airQualitySendCount = 1; // common for all AirQuality characteristics
  
  volatile byte airQualitySent = 0;
  /*volatile byte pm10envSent = 0;
  volatile byte pm25envSent = 0;
  volatile byte pm100envSent = 0;*/
  volatile byte pm10standardSent = 0;
  volatile byte pm25standardSent = 0;
  volatile byte pm100standardSent = 0;

  volatile byte ppmCO2Sent = 0;
  
  volatile byte airQualityValue = 0;
  /*volatile int pm10envValue = 0;
  volatile int pm25envValue = 0;
  volatile int pm100envValue = 0;*/
  volatile int pm10standardValue = 0;
  volatile int pm25standardValue = 0;
  volatile int pm100standardValue = 0;

  volatile int ppmCO2Value = 0;
  volatile byte ppmCO2Id = ID_PPM_CO2_LOW;
#endif  

// window covering
#if defined(SERVICE_WINDOW_COVERING_D)
  #define PIN_MOTOR_IN1 10
  #define PIN_MOTOR_IN2 9
  #define PIN_WINDOW_COVERING_OPEN 0
  #define PIN_WINDOW_COVERING_CLOSED 1
  #define PIN_WINDOW_COVERING_SWITCHES 8
  #define WINDOW_COVERING_NO_INTERRUPT 0
  #define WINDOW_COVERING_STOP_ON_OPEN_INTERRUPT 1
  #define WINDOW_COVERING_STOP_ON_CLOSE_INTERRUPT 2
  volatile byte windowCoveringStopOnInterrupt = WINDOW_COVERING_NO_INTERRUPT;
  volatile float windowCoveringCurrentPosition = 50;
  volatile byte windowCoveringTargetPosition = 50;
  volatile byte windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_STOPPED;
  #define WDT_TIMEOUT_WINDOW_COVERING 1 // 0.5 secs, pause = value x 0.5 secs 
  //#define WDT_TIMEOUT_WINDOW_COVERING_STATUS 4 // 2 secs, pause = value x 0.5 secs
  float windowCoveringStep;
  volatile int timerWDTWindowCovering = 0;
  //volatile int timerWDTWindowCoveringStatus = 0;
  volatile int stateWindowCoveringSwitchOpen = HIGH;
  volatile int stateWindowCoveringSwitchClose = LOW;
  const byte windowCoveringStatusSendCount = 1;
  volatile byte windowCoveringStatusSent = windowCoveringStatusSendCount + 1;
#endif  

// door
#if defined(SERVICE_DOOR_D)
  #define PIN_SERVO 8 // 8
  #define PIN_ULTRASONIC_1_PING 10 // 0
  #define PIN_ULTRASONIC_1_ECHO 1 // 1
  #define PIN_ULTRASONIC_2_PING 9 // 9
  #define PIN_ULTRASONIC_2_ECHO 0 // 10
  //#define ULTRASONIC_USE_INTERRUPTS // working not stable when using interrupts
  #if defined(ULTRASONIC_USE_INTERRUPTS)
    volatile byte ultrasonicNumber = 0;
    volatile unsigned long ultrasonicStartEcho;
    volatile unsigned long ultrasonicStopEcho;
    const long ultrasonicTimeout = 50;
  #endif  
  #define soundSpeed 343.0  // Speed of sound in m/s (343m/s at 20°C with dry air) 
  volatile boolean ignoreUltrasonic = false;
  volatile float doorCurrentPosition = 0;
  volatile byte doorTargetPosition = 0;
  volatile byte doorPositionState = DOOR_POSITION_STATE_STOPPED;
  #define WDT_TIMEOUT_DOOR 1 // 0.5 secs, pause = value x 0.5 secs 
  #define WDT_TIMEOUT_DOOR_ULTRASONIC_CLOSE 6 // 10 secs, pause = value x 0.5 secs 
  float doorStep;
  volatile int timerWDTDoor = 0;
  volatile int timerWDTDoorUltrasonicClose = 0;
  volatile boolean ultrasonicDetectsObject = false;  
  const byte doorStatusSendCount = 1;
  volatile byte doorStatusSent = doorStatusSendCount + 1;
#endif

// contact
#if defined(SERVICE_CONTACT_D)  
  #define PIN_CONTACT_1 0
  #define PIN_CONTACT_2 1
  #define WDT_TIMEOUT_CONTACT 1 // 0.5 secs, pause = value x 0.5 secs
  volatile int timerWDT1Contact = 0;
  volatile int timerWDT2Contact = 0;
  const byte contactStateSendCount = 3;
  volatile byte lastContact1State = -1;
  volatile byte lastContact2State = -1;
  volatile byte contact1StateSent = 0;
  volatile byte contact2StateSent = 0;
#endif  

// battery level
#if defined(SERVICE_BATTERY_LEVEL_D)  
  #ifdef DEBUG
  #define WDT_TIMEOUT_BATTERY_LEVEL 112 // 56 secs, pause = value x 0.5 secs
  #else
  #define WDT_TIMEOUT_BATTERY_LEVEL 3600 // 30 mins, pause = value x 0.5 secs
  #endif

  volatile int timerWDTBatteryLevel = 0;

  // 2 AA rechargeable batteries, 2100-2700 mAh
  // 2 AAA rechargeable batteries, 1100 mAh
  #define VCC_MIN 2.2
  #define VCC_MAX 2.75
  
  // 1 CR2032 battery 200 mAh
  /*#define VCC_MIN 2.4
    #define VCC_MAX 3.0*/
#endif  

volatile boolean accessoryPaired = false;

RF24 radio(PIN_CE, PIN_CSN);
#if defined(SERVICE_BATTERY_LEVEL_D)  
  Vcc vcc(VCC_CORRECTION);
#endif
#if defined(SERVICE_HUMIDITY_D)    
  dht dht;
#endif  
#if defined(SERVICE_DOOR_D)
  //SoftwareServo servo;
#endif  

void setup() {
  #if defined(SERVICE_REPEATER_D)
    //EEPROM.write(0, 255); // reset once for initialization for new board
    byte eepromRepeatersInitialized = EEPROM.read(0);
    if (eepromRepeatersInitialized == 255) {
      EEPROM.write(0, 1);
      // not initialized yet
      for (byte i = 0; i < MAX_COUNT_REPEATERS; i++) {
        int address = i + 1;
        EEPROMWriteByte(address, 255);
      }
    } else {
      // already initialized
      // uncomment if you need to clear stored accessories repeaters
      /*for (byte i=0; i<MAX_COUNT_REPEATERS; i++) {
        int address = i+1;
        byte repeaterId = EEPROMReadByte(address);
        if (accessoryService != 0) {
          EEPROMUpdateByte(address, 0);
        }
        }*/
    }
  #endif  
  #if defined(SERVICE_WINDOW_COVERING_D)
    // TODO check that SERVICE_WINDOW_COVERING is not combined together with any other services (except SERVICE_BATTERY_LEVEL) because of shared pins
    float windowCoveringLengthSec = WINDOW_COVERING_LENGTH_MM / (3.1416 * WINDOW_COVERING_MOTOR_STOCK_DIAMETER_MM * WINDOW_COVERING_MOTOR_RPM / 60);
    windowCoveringStep = 100 * (WDT_TIMEOUT_WINDOW_COVERING * 0.5 / windowCoveringLengthSec);
  #endif  
  #if defined(SERVICE_DOOR_D)
    doorStep = 100 * (WDT_TIMEOUT_DOOR * 0.5 * DOOR_SERVO_DEGREES / (90 * DOOR_SERVO_SECS));
  #endif
  radio.begin();
  radio.setAutoAck(1);
  //radio.enableAckPayload();                     // Allow optional ack payloads
  //radio.enableDynamicPayloads();                // Ack payloads are dynamic payloads
  //radio.setCRCLength(RF24_CRC_16);
  radio.setPALevel(RF24_PA_MAX);
  //radio.setDataRate(RF24_1MBPS);
  radio.setChannel(channel);
  radio.setRetries(30, 30);
  radio.setPayloadSize(sizeof(Payload));
  //radio.openWritingPipe(CONTROLLER_RADIO_ID);
  radio.openReadingPipe(0, RADIO_ID);
  radio.startListening();
  if (isServiceSupported(SERVICE_WINDOW_COVERING)) {
    #if defined(SERVICE_WINDOW_COVERING_D)
      setupWatchdogWindowCovering();
    #endif
  } else if (isServiceSupported(SERVICE_DOOR)) {
    #if defined(SERVICE_DOOR_D)
      setupWatchdogDoor();
    #endif
  } else if (isServiceSupported(SERVICE_AIR_QUALITY)) {
    #if defined(SERVICE_AIR_QUALITY_D)
      setupWatchdogAirQuality();
    #endif  
  } else {
    setupWatchdogDefault();
  }
  pinMode(PIN_IRQ, INPUT);
  GIMSK |= _BV(PCIE0);    // Enable Pin Change Interrupts
  PCMSK0 |= _BV(PCINT7);  // Use PA7 as interrupt pin
  #if defined(PIN_POWER)
    pinMode(PIN_POWER, OUTPUT);
  #endif  
  #if defined(SERVICE_MOTION_D) || defined(SERVICE_LEAK_D) 
    if (isServiceSupported(SERVICE_MOTION) || isServiceSupported(SERVICE_LEAK)) {
      pinMode(PIN_PIR_LEAK, INPUT);
      GIMSK |= _BV(PCIE1);    // Enable Pin Change Interrupts
      PCMSK1 |= _BV(PCINT8);  // Use PB0 as interrupt pin
    }
  #endif  
  #if defined(SERVICE_CONTACT_D)
    if (isServiceSupported(SERVICE_1_CONTACT)) {
      pinMode(PIN_CONTACT_1, INPUT);
      GIMSK |= _BV(PCIE1);    // Enable Pin Change Interrupts
      PCMSK1 |= _BV(PCINT8);  // Use PB0 as interrupt pin
    }
    if (isServiceSupported(SERVICE_2_CONTACT)) {
      pinMode(PIN_CONTACT_2, INPUT);
      GIMSK |= _BV(PCIE1);    // Enable Pin Change Interrupts
      PCMSK1 |= _BV(PCINT9);  // Use PB1 as interrupt pin
    }
  #endif  
  #if defined(SERVICE_TEMPERATURE_D)
    if (isServiceSupported(SERVICE_TEMPERATURE) && !useDht) {
      pinMode(PIN_TEMPERATURE, INPUT);
    }
  #endif  
  #if defined(SERVICE_RELAY_D)
    if (isServiceSupported(SERVICE_OUTLET)  || isServiceSupported(SERVICE_LIGHT_BULB)) {
      pinMode(PIN_RELAY_SWITCH, OUTPUT);
      if (reverseRelayValue) {
        digitalWrite(PIN_RELAY_SWITCH, HIGH);
      }
    }
  #endif
  #if defined(SERVICE_SWITCH_D)
    if (isServiceSupported(SERVICE_SWITCH)) {
      pinMode(PIN_RELAY_SWITCH, INPUT);
      GIMSK |= _BV(PCIE1);    // Enable Pin Change Interrupts
      PCMSK1 |= _BV(PCINT9);  // Use PB1 as interrupt pin
    }
  #endif
  #if defined(SERVICE_LIGHT_D)  
    if (isServiceSupported(SERVICE_LIGHT)) {
      pinMode(PIN_LDR, INPUT);
    }
  #endif  
  #if defined(SERVICE_AIR_QUALITY_D)
    if (isServiceSupported(SERVICE_AIR_QUALITY)) {
      pinMode(PIN_MQ135, INPUT);
    }
  #endif  
  #if defined(SERVICE_WINDOW_COVERING_D)
    if (isServiceSupported(SERVICE_WINDOW_COVERING)) {
      pinMode(PIN_WINDOW_COVERING_OPEN, INPUT);
      pinMode(PIN_WINDOW_COVERING_CLOSED, INPUT);
      GIMSK |= _BV(PCIE1);    // Enable Pin Change Interrupts
      PCMSK1 |= _BV(PCINT8);  // Use PB0 as interrupt pin
      PCMSK1 |= _BV(PCINT9);  // Use PB1 as interrupt pin
      pinMode(PIN_MOTOR_IN1, OUTPUT);
      pinMode(PIN_MOTOR_IN2, OUTPUT);
      pinMode(PIN_WINDOW_COVERING_SWITCHES, INPUT);
      byte valueWindowCoveringOpen = digitalRead(PIN_WINDOW_COVERING_OPEN);
      byte valueWindowCoveringClosed = digitalRead(PIN_WINDOW_COVERING_CLOSED);
      if (valueWindowCoveringOpen == HIGH) {
        windowCoveringTargetPosition = 100;
        windowCoveringCurrentPosition = windowCoveringTargetPosition;
      } else if (valueWindowCoveringClosed == HIGH) {
        windowCoveringTargetPosition = 0;
        windowCoveringCurrentPosition = windowCoveringTargetPosition;
      }
    }
  #endif  
  #if defined(SERVICE_DOOR_D)
    if (isServiceSupported(SERVICE_DOOR)) {
      pinMode(PIN_ULTRASONIC_1_PING, OUTPUT);
      pinMode(PIN_ULTRASONIC_2_PING, OUTPUT);
      pinMode(PIN_ULTRASONIC_1_ECHO, INPUT);
      pinMode(PIN_ULTRASONIC_2_ECHO, INPUT);
      digitalWrite(PIN_ULTRASONIC_1_PING, LOW);
      digitalWrite(PIN_ULTRASONIC_2_PING, LOW);
      #if defined(ULTRASONIC_USE_INTERRUPTS)
        GIMSK |= _BV(PCIE1);    // Enable Pin Change Interrupts
        PCMSK1 |= _BV(PCINT8);  // Use PB0 as interrupt pin
        PCMSK1 |= _BV(PCINT9);  // Use PB1 as interrupt pin
      #endif  
      /*servo.attach(PIN_SERVO);
      servo.setMinimumPulse(500);
      servo.setMaximumPulse(2500);*/
      doorCurrentPosition = EEPROMReadByte(250);
      if (doorCurrentPosition < 0) {
        doorCurrentPosition = 0;
      }
      if (doorCurrentPosition > 100) {
        doorCurrentPosition = 100;
      }
      doorTargetPosition = doorCurrentPosition;
      pinMode(PIN_SERVO, OUTPUT); 
      byte servoAngle = (90 * doorCurrentPosition / 100);
      doChangeDoor(servoAngle);
    }
  #endif
  #if defined(SERVICE_AIR_QUALITY_D)
    i2c_init();
  #endif  
  sei();                  //Enable interrupts
  sendPairingRequest();
}

void loop() {
  systemSleep();
  #if defined(SERVICE_DOOR_D)
    if (isServiceSupported(SERVICE_DOOR)) {
      //refreshSoftwareServo();
    }
  #endif
  if (!accessoryPaired) {
    if (interrupted0) {
      while (radio.available()) {
        Payload payloadPairingResponse;
        radio.read(&payloadPairingResponse, sizeof(Payload));
        if (checkControl(payloadPairingResponse)) {
          if ((payloadPairingResponse.accessoryId == ACCESSORY_ID) && (payloadPairingResponse.service == ACCESSORY_SERVICE)) {
            if (payloadPairingResponse.id == ID_PAIRING) {
              accessoryPaired = (payloadPairingResponse.value == RESULT_OK);
            }
          }
        }
      }
      interrupted0 = false;
    }
    if (accessoryPaired) {
      if (!isListeningMode()) {
        radio.stopListening();
        radio.closeReadingPipe(0);
      }
      #if defined(SERVICE_MOTION_D)
        if (isServiceSupported(SERVICE_MOTION)) {
          sendMotionAction(0);
          lastMotionActionValue = 0;
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
      #endif
      #if defined(SERVICE_LEAK_D)  
        if (isServiceSupported(SERVICE_LEAK)) {
          sendLeakState(readLeakState());
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
      #endif
      #if defined(SERVICE_SWITCH_D)  
        if (isServiceSupported(SERVICE_SWITCH)) {
          sendSwitchAction(3);    
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
      #endif
      #if defined(SERVICE_CONTACT_D)  
        if (isServiceSupported(SERVICE_1_CONTACT)) {
          lastContact1State = readContact1State();
          sendContactState(SERVICE_1_CONTACT, ID_CONTACT_1_STATE, lastContact1State);
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
        if (isServiceSupported(SERVICE_2_CONTACT)) {
          lastContact2State = readContact2State();
          sendContactState(SERVICE_2_CONTACT, ID_CONTACT_2_STATE, lastContact2State);
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
      #endif  
      #if defined(SERVICE_WINDOW_COVERING_D)
        if (isServiceSupported(SERVICE_WINDOW_COVERING)) {
          sendWindowCoveringCurrentPosition(static_cast<byte>(windowCoveringCurrentPosition));
          sendWindowCoveringTargetPosition(windowCoveringTargetPosition);
          sendWindowCoveringPositionState(windowCoveringPositionState);
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
          //timerWDTWindowCoveringStatus = 0;
        }
      #endif  
      #if defined(SERVICE_DOOR_D)
        if (isServiceSupported(SERVICE_DOOR)) {
          sendDoorCurrentPosition(static_cast<byte>(doorCurrentPosition));
          sendDoorTargetPosition(doorTargetPosition);
          sendDoorPositionState(doorPositionState);
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
      #endif  
      #if defined(SERVICE_AIR_QUALITY_D)
        if (isServiceSupported(SERVICE_AIR_QUALITY)) {
          checkAirQuality();
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
      #endif  
    } else {
      if (timerWDTPairing >= WDT_TIMEOUT_PAIRING) {
        sendPairingRequest();
        timerWDTPairing = 0;
        timerWDTReachabilityPing = 0;
        //timerWDTShortReachabilityPing = 0;
      }
    }
  } else {
    if (interrupted0) {
      if (isListeningMode()) {
        while (radio.available()) {
          Payload payload;
          radio.read(&payload, sizeof(Payload));
          if (checkControl(payload)) {
            if (payload.accessoryId == ACCESSORY_ID) {
              #if defined(SERVICE_RELAY_D)
                if (isServiceSupported(SERVICE_OUTLET) && (payload.service == SERVICE_OUTLET) && (payload.id == ID_OUTLET_ACTION)) {
                  //byte payloadAck = RESULT_OK;
                  //radio.writeAckPayload(1, &payloadAck, 1);
                  doRelayAction(payload.value);
                }
                if (isServiceSupported(SERVICE_LIGHT_BULB) && (payload.service == SERVICE_LIGHT_BULB) && (payload.id == ID_LIGHT_BULB_ACTION)) {
                  //byte payloadAck = RESULT_OK;
                  //radio.writeAckPayload(1, &payloadAck, 1);
                  doRelayAction(payload.value);
                }
              #endif  
              /*#if defined(SERVICE_SWITCH_D)
                if (isServiceSupported(SERVICE_SWITCH) && (payload.service == SERVICE_SWITCH) && (payload.id == ID_SWITCH_ACTION)) {
                  //byte payloadAck = RESULT_OK;
                  //radio.writeAckPayload(1, &payloadAck, 1);
                  byte switchValue = payload.value;
                  if (switchValue == 0) {
                    switchOn = false;
                  } else {
                    switchOn = true;
                  }
                }
              #endif*/
              #if defined(SERVICE_WINDOW_COVERING_D)
                if (isServiceSupported(SERVICE_WINDOW_COVERING) && (payload.service == SERVICE_WINDOW_COVERING) && (payload.id == ID_WINDOW_COVERING_TARGET_POSITION)) {
                  //byte payloadAck = RESULT_OK;
                  //radio.writeAckPayload(1, &payloadAck, 1);
                  windowCoveringTargetPosition = payload.value;
                  doWindowCovering();
                  timerWDTWindowCovering = 0;
                  //timerWDTWindowCoveringStatus = 0;
                }
              #endif  
              #if defined(SERVICE_DOOR_D)
                if (isServiceSupported(SERVICE_DOOR) && (payload.service == SERVICE_DOOR) && (payload.id == ID_DOOR_TARGET_POSITION)) {
                  //byte payloadAck = RESULT_OK;
                  //radio.writeAckPayload(1, &payloadAck, 1);
                  doorTargetPosition = payload.value;
                  ignoreUltrasonic = (doorTargetPosition == 100);
                  doDoor();
                  timerWDTDoor = 0;
                }
              #endif  
            } else {
              #if defined(SERVICE_REPEATER_D)
                // forward payload
                if (isAccessoryServiceSupported(payload.service, SERVICE_REPEATER)) {
                  // from controller
                  byte repeaterId = getRepeaterId(payload.accessoryId);
                  if (repeaterId != payload.accessoryId) {
                    addRepeaterService(&payload);
                    // to accessory via repeater
                    forwardPayload(payload.accessoryId, payload.service, payload.id, payload.value, repeaterId);
                  } else {
                    // to accessory
                    forwardPayload(payload.accessoryId, payload.service ^ SERVICE_REPEATER, payload.id, payload.value, payload.accessoryId);
                  }
                } else {
                  // to controller
                  if (payload.id == ID_PAIRING) {
                    // store repeater id
                    int address1 = payload.accessoryId + 1;
                    if ((payload.value > 0) && (payload.value < MAX_COUNT_REPEATERS)) { // TODO change to payload.value>=0 and in accessory send 255 instead of 0 for payload value in pairing request
                      EEPROMUpdateByte(address1, payload.value);
                    } else {
                      EEPROMUpdateByte(address1, 255);
                    }
                    forwardPayload(payload.accessoryId, payload.service, payload.id, ACCESSORY_ID, CONTROLLER_RADIO_ID);
                  } else {
                    forwardPayload(payload.accessoryId, payload.service, payload.id, payload.value, CONTROLLER_RADIO_ID);
                  }
                }
              #endif
            }
          }
        }
      }
      interrupted0 = false;
    }
    if (interrupted1) {
      #if defined(SERVICE_MOTION_D)
        if (isServiceSupported(SERVICE_MOTION)) {
          byte valuePIR = digitalRead(PIN_PIR_LEAK);
          if (valuePIR == HIGH) {
            setupWatchdogMotion();
            byte motionActionValue = applyLightToMotionAction(1);
            if (lastMotionActionValue != motionActionValue) {
              sendMotionAction(motionActionValue);
              lastMotionActionValue = motionActionValue;
              timerWDTReachabilityPing = 0;
              //timerWDTShortReachabilityPing = 0;
            }
          }
          timerWDTMotion = 0;
        }
      #endif
      #if defined(SERVICE_LEAK_D)  
        if (isServiceSupported(SERVICE_LEAK)) {
          setupWatchdogLeak();
          leakStateSent = 0;
        }
      #endif
      #if defined(SERVICE_SWITCH_D)
        if (isServiceSupported(SERVICE_SWITCH)) {
          //digitalWrite(PIN_RELAY_SWITCH, LOW);    // make sure the pullup resistors are off // no need!!!
          byte valueSwitch = digitalRead(PIN_RELAY_SWITCH);
          //digitalWrite(PIN_RELAY_SWITCH, HIGH);   // turn on the pullup resistors // no need!!!
          if (valueSwitch == HIGH) {
            /*switchOn = !switchOn;
            doSwitchAction();*/
            sendSwitchAction(2);
            /*setupWatchdogSwitch();
            switchActionSent = 0;*/
          }
        }
      #endif
      #if defined(SERVICE_CONTACT_D)  
        if (isServiceSupported(SERVICE_1_CONTACT)) {
          setupWatchdogContact();
          byte contact1State = readContact1State();
          if (lastContact1State != contact1State) {
            contact1StateSent = 0;
          }
        }
        if (isServiceSupported(SERVICE_2_CONTACT)) {
          setupWatchdogContact();
          byte contact2State = readContact2State();
          if (lastContact2State != contact2State) {
            contact2StateSent = 0;
          }
        }
      #endif  
      #if defined(SERVICE_WINDOW_COVERING_D)
        if (isServiceSupported(SERVICE_WINDOW_COVERING)) {
          if (windowCoveringStopOnInterrupt != WINDOW_COVERING_NO_INTERRUPT) {
            if (windowCoveringStopOnInterrupt == WINDOW_COVERING_STOP_ON_OPEN_INTERRUPT) {
              windowCoveringTargetPosition = 100;
            } else if (windowCoveringStopOnInterrupt == WINDOW_COVERING_STOP_ON_CLOSE_INTERRUPT) {
              windowCoveringTargetPosition = 0;
            }
            windowCoveringCurrentPosition = windowCoveringTargetPosition;
            windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_STOPPED;
            doStopWindowCovering();
            sendWindowCoveringCurrentPosition(static_cast<byte>(windowCoveringCurrentPosition));
            sendWindowCoveringTargetPosition(windowCoveringTargetPosition);
            sendWindowCoveringPositionState(windowCoveringPositionState);
            windowCoveringStopOnInterrupt = WINDOW_COVERING_NO_INTERRUPT;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
            //timerWDTWindowCoveringStatus = 0;
            windowCoveringStatusSent = 0;
          }
        }
      #endif  
      interrupted1 = false;
    }
    if (timerWDTReachabilityPing >= WDT_TIMEOUT_REACHABILITY_PING) {
      sendReachabilityPing();
      timerWDTReachabilityPing = 0;
    }
    /*if (timerWDTShortReachabilityPing >= WDT_TIMEOUT_SHORT_REACHABILITY_PING) {
      if (!isServiceSupported(SERVICE_BATTERY_LEVEL)) {
        sendReachabilityPing();
      }
      timerWDTShortReachabilityPing = 0;
      }*/
    #if defined(SERVICE_MOTION_D)
      if (timerWDTMotionLight > WDT_TIMEOUT_MOTION_LIGHT) {
        if (isServiceSupported(SERVICE_MOTION)) {
          if (lastMotionActionValue > 0) {
            byte motionActionValue = applyLightToMotionAction(1);
            if (lastMotionActionValue != motionActionValue) {
              sendMotionAction(motionActionValue);
              lastMotionActionValue = motionActionValue;
              timerWDTReachabilityPing = 0;
              //timerWDTShortReachabilityPing = 0;
            }
          }
        }
        timerWDTMotionLight = 0;
      }
      if (timerWDTMotion > WDT_TIMEOUT_MOTION) {
        if (isServiceSupported(SERVICE_MOTION)) {
          if (lastMotionActionValue > 0) {
            byte valuePIR = digitalRead(PIN_PIR_LEAK);
            if (valuePIR == HIGH) {
              byte motionActionValue = applyLightToMotionAction(1);
              if (lastMotionActionValue != motionActionValue) {
                sendMotionAction(motionActionValue);
                lastMotionActionValue = motionActionValue;
                timerWDTReachabilityPing = 0;
                //timerWDTShortReachabilityPing = 0;
              }
            } else {
              setupWatchdogDefault();
              sendMotionAction(0);
              lastMotionActionValue = 0;
              timerWDTReachabilityPing = 0;
              //timerWDTShortReachabilityPing = 0;
            }
          }
        }
        timerWDTMotion = 0;
      }
    #endif 
    #if defined(SERVICE_LEAK_D) 
      if (timerWDTLeak >= WDT_TIMEOUT_LEAK) {
        if (isServiceSupported(SERVICE_LEAK)) {
          if (leakStateSent < leakStateSendCount) {
            sendLeakState(readLeakState());
            leakStateSent = leakStateSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (leakStateSent == leakStateSendCount) {
            setupWatchdogDefault();
            leakStateSent = leakStateSent + 1;
          }
        }
        timerWDTLeak = 0;
      }
    #endif
    /*#if defined(SERVICE_SWITCH_D) 
      if (timerWDTSwitch >= WDT_TIMEOUT_SWITCH) {
        if (isServiceSupported(SERVICE_SWITCH)) {
          if (switchActionSent < switchActionSendCount) {
            doSwitchAction();
            switchActionSent = switchActionSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (switchActionSent == switchActionSendCount) {
            setupWatchdogDefault();
            switchActionSent = switchActionSent + 1;
          }
        }
        timerWDTSwitch = 0;
      }
    #endif*/
    #if defined(SERVICE_LIGHT_D)
      if (timerWDTLight >= WDT_TIMEOUT_LIGHT) {
        if (isServiceSupported(SERVICE_LIGHT)) {
          sendLight();
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
        timerWDTLight = 0;
      }
    #endif
    #if defined(SERVICE_TEMPERATURE_D) || defined(SERVICE_HUMIDITY_D)  
      if (timerWDTTemperature >= WDT_TIMEOUT_TEMPERATURE) {
        #if defined(SERVICE_TEMPERATURE_D)
          if ((isServiceSupported(SERVICE_TEMPERATURE) && useDht) || isServiceSupported(SERVICE_HUMIDITY)) {
            #if defined(SERVICE_HUMIDITY_D)
              #ifdef USE_DHT11
                dht.read11(PIN_DHT);
              #else
                dht.read22(PIN_DHT);
              #endif
              delay(100); // important to make delay after reading dht sensor
            #endif  
          }
          if (isServiceSupported(SERVICE_TEMPERATURE)) {
            sendTemperature();
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          }
        #endif
        #if defined(SERVICE_HUMIDITY_D)    
          if (isServiceSupported(SERVICE_HUMIDITY)) {
            sendHumidity();
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          }
        #endif  
        timerWDTTemperature = 0;
      }
    #endif  
    #if defined(SERVICE_AIR_QUALITY_D)
      if (timerWDTAirQualityChange >= WDT_TIMEOUT_AIR_QUALITY_CHANGE) {
        if (isServiceSupported(SERVICE_AIR_QUALITY)) {
          if (airQualitySent < airQualitySendCount) {
            sendAirQuality(ID_AIR_QUALITY,airQualityValue); 
            airQualitySent = airQualitySent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (airQualitySent == airQualitySendCount) {
            airQualitySent = airQualitySent + 1;
          } else {
            checkAirQuality();
          }
          if (pm10standardSent < airQualitySendCount) {
            sendAirQuality(ID_PM_10_STANDARD,pm10standardValue);
            pm10standardSent = pm10standardSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (pm10standardSent == airQualitySendCount) {
            pm10standardSent = pm10standardSent + 1;
          } else {         
            pm10standardValue = checkPm(ID_PM_10_STANDARD,PM_10_STANDARD,pm10standardValue);
          }
          if (pm25standardSent < airQualitySendCount) {
            sendAirQuality(ID_PM_25_STANDARD,pm25standardValue);
            pm25standardSent = pm25standardSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (pm25standardSent == airQualitySendCount) {
            pm25standardSent = pm25standardSent + 1;
          } else {         
            pm25standardValue = checkPm(ID_PM_25_STANDARD,PM_25_STANDARD,pm25standardValue);
          }
          if (pm100standardSent < airQualitySendCount) {
            sendAirQuality(ID_PM_100_STANDARD,pm100standardValue);
            pm100standardSent = pm100standardSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (pm100standardSent == airQualitySendCount) {
            pm100standardSent = pm100standardSent + 1;
          } else {         
            pm100standardValue = checkPm(ID_PM_100_STANDARD,PM_100_STANDARD,pm100standardValue);
          }
          if (ppmCO2Sent < airQualitySendCount) {
            sendAirQuality(ppmCO2Id,ppmCO2Value);
            ppmCO2Sent = ppmCO2Sent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (ppmCO2Sent == airQualitySendCount) {
            ppmCO2Sent = ppmCO2Sent + 1;
          } else {         
            checkPpmCO2();
          }
          /*if (pm10envSent < airQualitySendCount) {
            sendAirQuality(ID_PM_10_STANDARD,pm10envValue);
            pm10envSent = pm10envSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (pm10envSent == airQualitySendCount) {
            pm10envSent = pm10envSent + 1;
          } else {         
            pm10envValue = checkPm(ID_PM_10_STANDARD,PM_10_STANDARD,pm10envValue);
          }
          if (pm25envSent < airQualitySendCount) {
            sendAirQuality(ID_PM_25_STANDARD,pm25envValue);
            pm25envSent = pm25envSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (pm25envSent == airQualitySendCount) {
            pm25envSent = pm25envSent + 1;
          } else {         
            pm25envValue = checkPm(ID_PM_25_STANDARD,PM_25_STANDARD,pm25envValue);
          }
          if (pm100envSent < airQualitySendCount) {
            sendAirQuality(ID_PM_100_STANDARD,pm100envValue);
            pm100envSent = pm100envSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (pm25envSent == airQualitySendCount) {
            pm100envSent = pm100envSent + 1;
          } else {         
            pm100envValue = checkPm(ID_PM_100_STANDARD,PM_100_STANDARD,pm100envValue);
          }*/
        }
        timerWDTAirQualityChange = 0;
      }
      if (timerWDTAirQuality >= WDT_TIMEOUT_AIR_QUALITY) {
        if (isServiceSupported(SERVICE_AIR_QUALITY)) {
          sendAirQuality(ID_AIR_QUALITY,airQualityValue);
          sendAirQuality(ID_PM_10_STANDARD,pm10standardValue);
          sendAirQuality(ID_PM_25_STANDARD,pm25standardValue);
          sendAirQuality(ID_PM_100_STANDARD,pm100standardValue);
          /*sendAirQuality(ID_PM_10_ENV,pm10envValue);
          sendAirQuality(ID_PM_25_ENV,pm25envValue);
          sendAirQuality(ID_PM_100_ENV,pm100envValue);*/
          sendAirQuality(ppmCO2Id,ppmCO2Value);
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
        timerWDTAirQuality = 0;
      }
    #endif  
    #if defined(SERVICE_WINDOW_COVERING_D)
      if (timerWDTWindowCovering >= WDT_TIMEOUT_WINDOW_COVERING) {
        if (isServiceSupported(SERVICE_WINDOW_COVERING)) {
          doWindowCovering();
        }
        timerWDTWindowCovering = 0;
      }
    #endif  
    #if defined(SERVICE_DOOR_D)
      if (timerWDTDoor >= WDT_TIMEOUT_DOOR) {
        if (isServiceSupported(SERVICE_DOOR)) {
          doDoor();
        }
        timerWDTDoor = 0;
      }
    #endif  
    /*#if defined(SERVICE_WINDOW_COVERING_D)
      if (timerWDTWindowCoveringStatus >= WDT_TIMEOUT_WINDOW_COVERING_STATUS) {
        if (isServiceSupported(SERVICE_WINDOW_COVERING)) {
          sendWindowCoveringCurrentPosition(static_cast<byte>(windowCoveringCurrentPosition));
          //sendWindowCoveringTargetPosition(windowCoveringTargetPosition);
          sendWindowCoveringPositionState(windowCoveringPositionState);
        }
        timerWDTWindowCoveringStatus = 0;
      }
    #endif*/
    #if defined(SERVICE_CONTACT_D)    
      if (timerWDT1Contact >= WDT_TIMEOUT_CONTACT) {
        if (isServiceSupported(SERVICE_1_CONTACT)) {
          if (contact1StateSent < contactStateSendCount) {
            lastContact1State = readContact1State();
            sendContactState(SERVICE_1_CONTACT, ID_CONTACT_1_STATE, lastContact1State);
            contact1StateSent = contact1StateSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (contact1StateSent == contactStateSendCount) {
            setupWatchdogDefault();
            contact1StateSent = contact1StateSent + 1;
          }
        }
        timerWDT1Contact = 0;
      }
      if (timerWDT2Contact >= WDT_TIMEOUT_CONTACT) {
        if (isServiceSupported(SERVICE_2_CONTACT)) {
          if (contact2StateSent < contactStateSendCount) {
            lastContact2State = readContact2State();
            sendContactState(SERVICE_2_CONTACT, ID_CONTACT_2_STATE, lastContact2State);
            contact2StateSent = contact2StateSent + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
          } else if (contact2StateSent == contactStateSendCount) {
            setupWatchdogDefault();
            contact2StateSent = contact2StateSent + 1;
          }
        }
        timerWDT2Contact = 0;
      }
    #endif  
    #if defined(SERVICE_BATTERY_LEVEL_D)  
      if (timerWDTBatteryLevel >= WDT_TIMEOUT_BATTERY_LEVEL) {
        if (isServiceSupported(SERVICE_BATTERY_LEVEL)) {
          sendBatteryLevel();
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        }
        timerWDTBatteryLevel = 0;
      }
    #endif  
    #if defined(SERVICE_WINDOW_COVERING_D)
      if (isServiceSupported(SERVICE_WINDOW_COVERING)) {
        doWindowCoveringSwitches();
      }
    #endif  
    #if defined(SERVICE_DOOR_D)
      if (isServiceSupported(SERVICE_DOOR)) {
        doDoorUltrasonic();
      }
    #endif  
  }
}

#if defined(SERVICE_REPEATER_D)
  byte getRepeaterId(byte accessoryId) {
    int address = accessoryId + 1;
    int repeaterId = EEPROMReadByte(address);
    if ((repeaterId > 0) && (repeaterId < MAX_COUNT_REPEATERS)) { // TODO change to repeaterId>=0 and in accessory send 255 instead of 0 for payload value in pairing request
      return repeaterId;
    } else {
      return accessoryId;
    }
  }
#endif  

#if defined(SERVICE_WINDOW_COVERING_D)
  void doWindowCovering() {
    byte windowCoveringPositionStatePrevious = windowCoveringPositionState;
    if (windowCoveringCurrentPosition != windowCoveringTargetPosition) {
      if (windowCoveringCurrentPosition > windowCoveringTargetPosition) {
        windowCoveringCurrentPosition = windowCoveringCurrentPosition - windowCoveringStep;
        windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_DECREASING;
        windowCoveringStatusSent = windowCoveringStatusSendCount + 1;
        if (windowCoveringCurrentPosition <= windowCoveringTargetPosition) {
          windowCoveringCurrentPosition = windowCoveringTargetPosition;
          windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_STOPPED;
          doStopWindowCovering();
          sendWindowCoveringCurrentPosition(static_cast<byte>(windowCoveringCurrentPosition));
          windowCoveringStatusSent = 0;
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
          //timerWDTWindowCoveringStatus = 0;
        } else {
          doCloseWindowCovering();
        }
      } else if (windowCoveringCurrentPosition < windowCoveringTargetPosition) {
        windowCoveringCurrentPosition = windowCoveringCurrentPosition + windowCoveringStep;
        windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_INCREASING;
        windowCoveringStatusSent = windowCoveringStatusSendCount + 1;
        if (windowCoveringCurrentPosition >= windowCoveringTargetPosition) {
          windowCoveringCurrentPosition = windowCoveringTargetPosition;
          windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_STOPPED;
          doStopWindowCovering();
          sendWindowCoveringCurrentPosition(static_cast<byte>(windowCoveringCurrentPosition));
          windowCoveringStatusSent = 0;
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
          //timerWDTWindowCoveringStatus = 0;
        } else {
          doOpenWindowCovering();
        }
      }
      if (windowCoveringPositionStatePrevious != windowCoveringPositionState) {
        if (windowCoveringPositionState != WINDOW_COVERING_POSITION_STATE_STOPPED) {
          sendWindowCoveringCurrentPosition(static_cast<byte>(windowCoveringCurrentPosition));
        }
        sendWindowCoveringPositionState(windowCoveringPositionState);
        timerWDTReachabilityPing = 0;
        //timerWDTShortReachabilityPing = 0;
        //timerWDTWindowCoveringStatus = 0;
      }
    }
    if (windowCoveringStatusSent < 1) {
      windowCoveringStatusSent = windowCoveringStatusSent + 1;
    } else {
      if (windowCoveringStatusSent <= windowCoveringStatusSendCount) {
        sendWindowCoveringCurrentPosition(static_cast<byte>(windowCoveringCurrentPosition));
        //sendWindowCoveringTargetPosition(windowCoveringTargetPosition);
        sendWindowCoveringPositionState(windowCoveringPositionState);
        windowCoveringStatusSent = windowCoveringStatusSent + 1;
        timerWDTReachabilityPing = 0;
        //timerWDTShortReachabilityPing = 0;
        //timerWDTWindowCoveringStatus = 0;
      } else if (windowCoveringStatusSent == windowCoveringStatusSendCount) {
        windowCoveringStatusSent = windowCoveringStatusSent + 1;
      }
    }
  }
  
  void doWindowCoveringSwitches() {
    byte windowCoveringPositionStatePrevious = windowCoveringPositionState;
    int stateWindowCoveringSwitchOpenPrevious = stateWindowCoveringSwitchOpen;
    int stateWindowCoveringSwitchClosePrevious = stateWindowCoveringSwitchClose;
    digitalWrite(PIN_WINDOW_COVERING_SWITCHES, LOW);    // make sure the pullup resistors are off
    stateWindowCoveringSwitchOpen = digitalRead(PIN_WINDOW_COVERING_SWITCHES);
    digitalWrite(PIN_WINDOW_COVERING_SWITCHES, HIGH);   // turn on the pullup resistors
    stateWindowCoveringSwitchClose = digitalRead(PIN_WINDOW_COVERING_SWITCHES);
    if ((stateWindowCoveringSwitchOpenPrevious != stateWindowCoveringSwitchOpen) || (stateWindowCoveringSwitchClosePrevious != stateWindowCoveringSwitchClose)) {
      if ((stateWindowCoveringSwitchOpen == HIGH) && (stateWindowCoveringSwitchClose == HIGH)) {  // both states HIGH - switch Open must be pushed
        if (windowCoveringPositionState == WINDOW_COVERING_POSITION_STATE_STOPPED) {
          windowCoveringTargetPosition = 100;
          if (windowCoveringCurrentPosition != windowCoveringTargetPosition) {
            windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_INCREASING;
            doOpenWindowCovering();
            sendWindowCoveringTargetPosition(windowCoveringTargetPosition);
            windowCoveringStatusSent = windowCoveringStatusSendCount + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
            //timerWDTWindowCoveringStatus = 0;
          }
        } else {
          windowCoveringTargetPosition = windowCoveringCurrentPosition;
          windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_STOPPED;
          doStopWindowCovering();
          sendWindowCoveringTargetPosition(windowCoveringTargetPosition);
          windowCoveringStatusSent = 0;
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
          //timerWDTWindowCoveringStatus = 0;
        }
      } else if ((stateWindowCoveringSwitchOpen == LOW) && (stateWindowCoveringSwitchClose == LOW)) { // both states LOW - switch Close must be pushed
        if (windowCoveringPositionState == WINDOW_COVERING_POSITION_STATE_STOPPED) {
          windowCoveringTargetPosition = 0;
          if (windowCoveringCurrentPosition != windowCoveringTargetPosition) {
            windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_DECREASING;
            doCloseWindowCovering();
            sendWindowCoveringTargetPosition(windowCoveringTargetPosition);
            windowCoveringStatusSent = windowCoveringStatusSendCount + 1;
            timerWDTReachabilityPing = 0;
            //timerWDTShortReachabilityPing = 0;
            //timerWDTWindowCoveringStatus = 0;
          }
        } else {
          windowCoveringTargetPosition = windowCoveringCurrentPosition;
          windowCoveringPositionState = WINDOW_COVERING_POSITION_STATE_STOPPED;
          doStopWindowCovering();
          sendWindowCoveringTargetPosition(windowCoveringTargetPosition);
          windowCoveringStatusSent = 0;
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
          //timerWDTWindowCoveringStatus = 0;
        }
      }
      if (windowCoveringPositionStatePrevious != windowCoveringPositionState) {
        sendWindowCoveringPositionState(windowCoveringPositionState);
        timerWDTReachabilityPing = 0;
        //timerWDTShortReachabilityPing = 0;
        //timerWDTWindowCoveringStatus = 0;
      }
    }
  }
#endif  

#if defined(SERVICE_DOOR_D)
  void doDoor() {
    byte doorPositionStatePrevious = doorPositionState;
    if (doorCurrentPosition != doorTargetPosition) {
      byte servoAngle;
      if (doorCurrentPosition > doorTargetPosition) {
        doorPositionState = DOOR_POSITION_STATE_DECREASING;
        doorStatusSent = doorStatusSendCount + 1;
        doorCurrentPosition = doorCurrentPosition - doorStep;
        if (doorCurrentPosition <= doorTargetPosition) {
          doorCurrentPosition = doorTargetPosition;
          servoAngle = (90 * doorCurrentPosition / 100);
          doChangeDoor(servoAngle);
          EEPROMUpdateByte(250,doorCurrentPosition);
          doorPositionState = DOOR_POSITION_STATE_STOPPED;
          sendDoorCurrentPosition(static_cast<byte>(doorCurrentPosition));
          doorStatusSent = 0;
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        } else {
          servoAngle = (90 * doorCurrentPosition / 100);
          doChangeDoor(servoAngle);
        }
      } else if (doorCurrentPosition < doorTargetPosition) {
        doorPositionState = DOOR_POSITION_STATE_INCREASING;
        doorStatusSent = doorStatusSendCount + 1;
        doorCurrentPosition = doorCurrentPosition + doorStep;
        if (doorCurrentPosition >= doorTargetPosition) {
          doorCurrentPosition = doorTargetPosition;
          servoAngle = (90 * doorCurrentPosition / 100);
          doChangeDoor(servoAngle);
          EEPROMUpdateByte(250,doorCurrentPosition);
          doorPositionState = DOOR_POSITION_STATE_STOPPED;
          sendDoorCurrentPosition(static_cast<byte>(doorCurrentPosition));
          doorStatusSent = 0;
          timerWDTReachabilityPing = 0;
          //timerWDTShortReachabilityPing = 0;
        } else {
          servoAngle = (90 * doorCurrentPosition / 100);
          doChangeDoor(servoAngle);
        }
      }
      if (doorPositionStatePrevious != doorPositionState) {
        if (doorPositionState != DOOR_POSITION_STATE_STOPPED) {
          sendDoorCurrentPosition(static_cast<byte>(doorCurrentPosition));
        }
        sendDoorPositionState(doorPositionState);
        timerWDTReachabilityPing = 0;
        //timerWDTShortReachabilityPing = 0;
      }
    }
    if (doorStatusSent < 1) {
      doorStatusSent = doorStatusSent + 1;
    } else {
      if (doorStatusSent <= doorStatusSendCount) {
        sendDoorCurrentPosition(static_cast<byte>(doorCurrentPosition));
        //sendDoorTargetPosition(doorTargetPosition);
        sendDoorPositionState(doorPositionState);
        doorStatusSent = doorStatusSent + 1;
        timerWDTReachabilityPing = 0;
        //timerWDTShortReachabilityPing = 0;
      } else if (doorStatusSent == doorStatusSendCount) {
        doorStatusSent = doorStatusSent + 1;
      }
    }
  }

  void doDoorUltrasonic() {
    if (!ignoreUltrasonic && (doorPositionState == DOOR_POSITION_STATE_STOPPED)) {
      boolean ultrasonicDetectsObjectPrevious = ultrasonicDetectsObject;
      boolean ultrasonicDetectsObjectCurrent = isUltrasonicDetectsObject(1) || isUltrasonicDetectsObject(2);
      if (!ultrasonicDetectsObjectCurrent) {
        if (timerWDTDoorUltrasonicClose >= WDT_TIMEOUT_DOOR_ULTRASONIC_CLOSE) {
          ultrasonicDetectsObject = ultrasonicDetectsObjectCurrent;
          timerWDTDoorUltrasonicClose = 0;   
        }
      } else {
        ultrasonicDetectsObject = ultrasonicDetectsObjectCurrent; 
        timerWDTDoorUltrasonicClose = 0;    
      }
      if (ultrasonicDetectsObjectPrevious != ultrasonicDetectsObject) {
        if (ultrasonicDetectsObject) {
          doorTargetPosition = 100;
        } else {
          doorTargetPosition = 0;
        }
        sendDoorTargetPosition(doorTargetPosition);
        doDoor();
        timerWDTDoor = 0;
      } 
    }
  }
#endif  

#if defined(SERVICE_AIR_QUALITY_D)
  void checkAirQuality() {
    byte airQualityValuePrevious = airQualityValue;
    airQualityValue = readAirQualityValue();
    if (airQualityValue != airQualityValuePrevious) {
      sendAirQuality(ID_AIR_QUALITY,airQualityValue);
      airQualitySent = 1;
    }
  }

  int checkPm(byte payloadId, byte id, int value) {
    int valuePrevious = value;
    int result = readPmValue(id);
    if (result != valuePrevious) {
      sendAirQuality(payloadId,result);
      if (payloadId == ID_PM_10_STANDARD) {
        pm10standardSent = 1;
      } else if (payloadId == ID_PM_25_STANDARD) {
        pm25standardSent = 1;
      } else if (payloadId == ID_PM_100_STANDARD) {
        pm100standardSent = 1;
      } 
    }
    return result;
  }
  
  void checkPpmCO2() {
    int ppmCO2ValuePrevious = ppmCO2Value;
    int ppmCO2ValueRaw = readPpmCO2Value();
    if (ppmCO2ValueRaw > (255 * KOEF_PPM_CO2_HIGH)) {
      ppmCO2Value = 255 * KOEF_PPM_CO2_HIGH;
    }
    if (ppmCO2ValueRaw > (255 * KOEF_PPM_CO2_MEDIUM)) {
      ppmCO2Id = ID_PPM_CO2_HIGH;
      ppmCO2Value = ppmCO2ValueRaw / KOEF_PPM_CO2_HIGH;   // on controller multiply by 40.0 - implementation for using 1 byte only
    } else if (ppmCO2ValueRaw > (255 * KOEF_PPM_CO2_LOW)) {
      ppmCO2Id = ID_PPM_CO2_MEDIUM;
      ppmCO2Value = ppmCO2ValueRaw / KOEF_PPM_CO2_MEDIUM;   // on controller multiply by 20.0 - implementation for using 1 byte only
    } else {
      ppmCO2Id = ID_PPM_CO2_LOW;
      ppmCO2Value = ppmCO2ValueRaw / KOEF_PPM_CO2_LOW;   // on controller multiply by 10.0 - implementation for using 1 byte only
    }  
    if (ppmCO2Value != ppmCO2ValuePrevious) {
      sendAirQuality(ppmCO2Id,ppmCO2Value);
      ppmCO2Sent = 1;
    }
  }
#endif  

#if defined(SERVICE_MOTION_D)
  void setupWatchdogMotion() {
    WDT_PERIOD = 1;
    setupWatchdog(5); // 0.5 sec watchdog
  }
#endif  

#if defined(SERVICE_LEAK_D)
  void setupWatchdogLeak() {
    WDT_PERIOD = 1;
    setupWatchdog(5); // 0.5 sec watchdog
  }
#endif  

/*#if defined(SERVICE_SWITCH_D)
  void setupWatchdogSwitch() {
    WDT_PERIOD = 1;
    setupWatchdog(5); // 0.5 sec watchdog
  }
#endif*/  

#if defined(SERVICE_AIR_QUALITY_D)
  void setupWatchdogAirQuality() {
    WDT_PERIOD = 1;
    setupWatchdog(5); // 0.5 sec watchdog
  }
#endif  

#if defined(SERVICE_WINDOW_COVERING_D)
  void setupWatchdogWindowCovering() {
    WDT_PERIOD = 1;
    setupWatchdog(5); // 0.5 sec watchdog
  }
#endif  

#if defined(SERVICE_DOOR_D)
  void setupWatchdogDoor() {
    WDT_PERIOD = 1;
    setupWatchdog(5); // 0.5 sec watchdog
  }
#endif  

#if defined(SERVICE_CONTACT_D)
  void setupWatchdogContact() {
    WDT_PERIOD = 1;
    setupWatchdog(5); // 0.5 sec watchdog
  }
#endif  

void setupWatchdogDefault() {
  WDT_PERIOD = 16;
  setupWatchdog(9); // 8 sec watchdog
}

// 0 = 16ms, 1 = 32ms, 2 = 64ms, 3 = 128ms, 4 = 250ms, 5 = 500ms, 6 = 1sec, 7 = 2sec, 8 = 4sec, 9 = 8sec
void setupWatchdog(int ii) {
  byte bb;
  int ww;
  if (ii > 9 ) ii = 9;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);
  ww = bb;
  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

void systemSleep() {
  if (accessoryPaired && !isListeningMode()) {
    radio.powerDown();
    //cbi(ADCSRA, ADEN);                 // switch Analog to Digitalconverter OFF, TODO make investigation
  }
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sleep_mode();                        // System sleeps here, waiting for interrupt
  sleep_disable();                     // System continues execution here when watchdog timed out
  if (accessoryPaired && !isListeningMode()) {
    //sbi(ADCSRA, ADEN);                 // switch Analog to Digitalconverter ON
    radio.powerUp();
  }
}

ISR(PCINT0_vect) {
  interrupted0 = true;
}

ISR(PCINT1_vect) {
  interrupted1 = true;
  #if defined(SERVICE_WINDOW_COVERING_D)
    if (isServiceSupported(SERVICE_WINDOW_COVERING)) {
      byte valueWindowCoveringOpen = digitalRead(PIN_WINDOW_COVERING_OPEN);
      byte valueWindowCoveringClosed = digitalRead(PIN_WINDOW_COVERING_CLOSED);
      if ((valueWindowCoveringOpen == HIGH) && (windowCoveringPositionState == WINDOW_COVERING_POSITION_STATE_INCREASING)) {
        windowCoveringStopOnInterrupt = WINDOW_COVERING_STOP_ON_OPEN_INTERRUPT;
      } else if ((valueWindowCoveringClosed == HIGH) && (windowCoveringPositionState == WINDOW_COVERING_POSITION_STATE_DECREASING)) {
        windowCoveringStopOnInterrupt = WINDOW_COVERING_STOP_ON_CLOSE_INTERRUPT;
      }
    }
  #endif  
  #if defined(SERVICE_DOOR_D)
    if (isServiceSupported(SERVICE_DOOR)) {
      #if defined(ULTRASONIC_USE_INTERRUPTS)
        byte valueUltrasonicEcho;
        if ((ultrasonicNumber == 1) || (ultrasonicNumber == 2)) {
          if (ultrasonicNumber == 1) {
            valueUltrasonicEcho = digitalRead(PIN_ULTRASONIC_1_ECHO);
          } else if (ultrasonicNumber == 2) {
            valueUltrasonicEcho = digitalRead(PIN_ULTRASONIC_2_ECHO);
          }
          if (valueUltrasonicEcho == LOW) {
            ultrasonicStopEcho = micros();
          } else {
            ultrasonicStartEcho = micros();
          }
        }
      #endif  
    }
  #endif  
}

ISR(WDT_vect) {
  if (!accessoryPaired) {
    timerWDTPairing = timerWDTPairing + WDT_PERIOD;
  } else {
    timerWDTReachabilityPing = timerWDTReachabilityPing + WDT_PERIOD;
    //timerWDTShortReachabilityPing = timerWDTShortReachabilityPing + WDT_PERIOD;
    #if defined(SERVICE_MOTION_D)
      timerWDTMotion = timerWDTMotion + WDT_PERIOD;
      timerWDTMotionLight = timerWDTMotionLight + WDT_PERIOD;
    #endif  
    #if defined(SERVICE_LIGHT_D)
      timerWDTLight = timerWDTLight + WDT_PERIOD;
    #endif
    #if defined(SERVICE_LEAK_D)  
      timerWDTLeak = timerWDTLeak + WDT_PERIOD;
    #endif  
    /*#if defined(SERVICE_SWITCH_D)  
      timerWDTSwitch = timerWDTSwitch + WDT_PERIOD;
    #endif*/  
    #if defined(SERVICE_TEMPERATURE_D) || defined(SERVICE_HUMIDITY_D)
      timerWDTTemperature = timerWDTTemperature + WDT_PERIOD;
    #endif  
    #if defined(SERVICE_AIR_QUALITY_D)
      timerWDTAirQuality = timerWDTAirQuality + WDT_PERIOD;
      timerWDTAirQualityChange = timerWDTAirQualityChange + WDT_PERIOD;
    #endif  
    #if defined(SERVICE_WINDOW_COVERING_D)
      timerWDTWindowCovering = timerWDTWindowCovering + WDT_PERIOD;
      //timerWDTWindowCoveringStatus = timerWDTWindowCoveringStatus + WDT_PERIOD;
    #endif  
    #if defined(SERVICE_DOOR_D)
      timerWDTDoor = timerWDTDoor + WDT_PERIOD;
      timerWDTDoorUltrasonicClose = timerWDTDoorUltrasonicClose + WDT_PERIOD;
    #endif  
    #if defined(SERVICE_CONTACT_D)
      timerWDT1Contact = timerWDT1Contact + WDT_PERIOD;
      timerWDT2Contact = timerWDT2Contact + WDT_PERIOD;
    #endif
    #if defined(SERVICE_BATTERY_LEVEL_D)  
      timerWDTBatteryLevel = timerWDTBatteryLevel + WDT_PERIOD;
    #endif  
  }
}

#if defined(SERVICE_TEMPERATURE_D)
  byte readTemperatureValue() {
    float temp;
    if (useDht) {
      #if defined(SERVICE_HUMIDITY_D)
        temp = dht.temperature;
        temp = temp + TEMPERATURE_CORRECTION;
        temp = (temp + 50.0) * 2.0;      // on controller divide by 2.0 and then substract 50.0 - implementation for using 1 byte only to a) get 0.5 degrees step (more than enough) and b) don't send negative values
      #endif  
    } else {
      temp = 0.0;
      analogReference(INTERNAL);
      #if defined(PIN_POWER)
        digitalWrite(PIN_POWER, HIGH);
      #endif
      analogRead(PIN_TEMPERATURE);                              // throw away the first reading
      delay(100);
      for (byte i = 0; i < TEMPERATURE_READING_COUNT ; i++) {   // take more readings
        temp = temp + analogRead(PIN_TEMPERATURE);              // accumulate readings
      }
      #if defined(PIN_POWER)
        digitalWrite(PIN_POWER, LOW);
      #endif    
      analogReference(DEFAULT);
      temp = temp / TEMPERATURE_READING_COUNT ;                 // calculate the average
      // constant 1100/1024 = 1.07421875
      temp = (TEMPERATURE_CORRECTION * 10.0) + (VCC_CORRECTION * temp * 1.07421875);
      temp = round(temp / 5.0);     // on controller divide by 2.0 and then substract 50.0 - implementation for using 1 byte only to a) get 0.5 degrees step (more than enough) and b) don't send negative values
    }
    return static_cast<byte>(temp);
  }
#endif  

#if defined(SERVICE_HUMIDITY_D)
  byte readHumidityValue() {
    float humidity;
    humidity = dht.humidity;
    humidity = humidity + HUMIDITY_CORRECTION;
    humidity = humidity * 2.0;   // on controller divide by 2.0 - implementation for using 1 byte only to get 0.5 % step (more than enough)
    return static_cast<byte>(humidity);
  }
#endif  

#if defined(SERVICE_AIR_QUALITY_D)
  byte readAirQualityValue() {
    int val = analogRead(PIN_MQ135);
    float resist = ((1023. / (float)val) * 5. - 1.) * MQ135_R_LOAD;
    float ppm = MQ135_PAR_A * pow((resist / MQ135_R_ZERO), -MQ135_PAR_B);
    int aq = 0; // UNKNOWN
    if (ppm < 400) {
      aq = 1; // EXCELLENT;
    } else if (ppm < 750) {
      aq = 2; // GOOD
    } else if (ppm < 1200) {
      aq = 3; // FAIR
    } else if (ppm < 2000) {
      aq = 4; // INFERIOR
    } else {
      aq = 5; // POOR
    }
    return aq;
  }

  int readPmValue(byte id) {
    return read_int(I2C_PMS_SLAVE_ADDRESS, id);
  }
  
  int readPpmCO2Value() {
    return read_int(I2C_MHZ_SLAVE_ADDRESS, PPM_CO2);
  }
#endif  

#if defined(SERVICE_LIGHT_D)
  int readLDRValue() {
    if (usePowerPin) {
      //analogReference(INTERNAL); // not working with this line
      digitalWrite(PIN_POWER, HIGH);
    }
    analogRead(PIN_LDR);                             // throw away the first reading
    delay(DELAY_READ_LDR);
    float light = 0.0;
    for (byte i = 0; i < LDR_READING_COUNT ; i++) { // take more readings
      light = light + analogRead(PIN_LDR);          // accumulate readings
    }
    if (usePowerPin) {
      digitalWrite(PIN_POWER, LOW);
      //analogReference(DEFAULT); // not working with this line
    }
    light = light / LDR_READING_COUNT ;
    return static_cast<int>(light);
  }
#endif  

#if defined(SERVICE_LEAK_D)
  byte readLeakState() {
    byte valueLeak = digitalRead(PIN_PIR_LEAK);
    if (valueLeak == HIGH) {
      return 0;
    } else {
      return 1;
    }
  }
#endif

#if defined(SERVICE_CONTACT_D)
  byte readContact1State() {
    byte valueContact = digitalRead(PIN_CONTACT_1);
    if (valueContact == HIGH) {
      return 0;
    } else {
      return 1;
    }
  }

  byte readContact2State() {
    byte valueContact = digitalRead(PIN_CONTACT_2);
    if (valueContact == HIGH) {
      return 0;
    } else {
      return 1;
    }
  }
#endif  

#if defined(SERVICE_DOOR_D)
  boolean isUltrasonicDetectsObject(byte number) {
    #if defined(ULTRASONIC_USE_INTERRUPTS)
      ultrasonicNumber = number;
      ultrasonicStartEcho = 0;
      ultrasonicStopEcho = 0;
    #endif  
    long duration;
    if (number == 1) {
      digitalWrite(PIN_ULTRASONIC_1_PING, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_ULTRASONIC_1_PING, LOW);
      #if !defined(ULTRASONIC_USE_INTERRUPTS)
        duration = pulseIn(PIN_ULTRASONIC_1_ECHO, HIGH);
      #endif  
    } else if (number == 2) {
      digitalWrite(PIN_ULTRASONIC_2_PING, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_ULTRASONIC_2_PING, LOW);
      #if !defined(ULTRASONIC_USE_INTERRUPTS)
        duration = pulseIn(PIN_ULTRASONIC_2_ECHO, HIGH);
      #endif  
    }
    #if defined(ULTRASONIC_USE_INTERRUPTS)
      long startTimeout = millis();
      while ((ultrasonicStartEcho == 0) || (ultrasonicStopEcho == 0)) {
        if (startTimeout + ultrasonicTimeout < millis()) {
          ultrasonicStartEcho = 0;
          ultrasonicStopEcho = 0;
          return false;
        }
      }
      duration = ultrasonicStopEcho - ultrasonicStartEcho;  
    #endif  
    float distanceCm = duration * (float)soundSpeed / 1000.0 / 10.0 / 2.0;
    sendTest(SERVICE_DOOR, number);
    sendTest(SERVICE_DOOR, static_cast<byte>(distanceCm/10));
    if (number == 1) {
      if (distanceCm < DISTANCE_CM_ULTRASONIC_1) {
         return true;
      }
    } else if (number == 2) {
      if (distanceCm < DISTANCE_CM_ULTRASONIC_2) {
        return true;
      }
    }
    return false;
  }
#endif  

#if defined(SERVICE_MOTION_D)
  byte applyLightToMotionAction(int value) {
    byte result = value;
    if (isServiceSupported(SERVICE_LIGHT) && (value > 0)) {
      if (lastMotionActionValue == 2) {
        result = 2;
      } else {
        int ldrValue = readLDRValue();
        if (ldrValue < VALUE_MIN_LDR_DAY) {
          result = 2;
        } else {
          result = 1;
        }
      }
    }
    return result;
  }
#endif  

boolean sendPairingRequest() {
  boolean result = false;
  radio.stopListening();
  radio.closeReadingPipe(0);
  radio.openWritingPipe(CONTROLLER_RADIO_ID);
  sendDelay(100);
  Payload payloadPairingRequest = { ACCESSORY_ID, ACCESSORY_SERVICE, ID_PAIRING, 0, 0 };
  setControl(&payloadPairingRequest);
  if (radio.write(&payloadPairingRequest, sizeof(Payload))) {
    result = true;
  }
  radio.openReadingPipe(0, RADIO_ID);
  radio.startListening();
  return result;
}

boolean sendReachabilityPing() {
  boolean result = false;
  stopListening();
  radio.openWritingPipe(CONTROLLER_RADIO_ID);
  sendDelay(100);
  Payload payloadReachabilityPing = { ACCESSORY_ID, ACCESSORY_SERVICE, ID_REACHABILITY_PING, 0, 0 };
  setControl(&payloadReachabilityPing);
  if (radio.write(&payloadReachabilityPing, sizeof(Payload))) {
    result = true;
  }
  startListening();
  return result;
}

#if defined(SERVICE_MOTION_D)
  boolean sendMotionAction(byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadMotionAction = { ACCESSORY_ID, SERVICE_MOTION, ID_MOTION_ACTION, value, 0 };
    setControl(&payloadMotionAction);
    if (radio.write(&payloadMotionAction, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_LEAK_D)
  boolean sendLeakState(byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadLeakState = { ACCESSORY_ID, SERVICE_LEAK, ID_LEAK_STATE, value, 0 };
    setControl(&payloadLeakState);
    if (radio.write(&payloadLeakState, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_SWITCH_D)
  /*void doSwitchAction() {
    if (switchOn) {
      sendSwitchAction(1);
    } else {
      sendSwitchAction(0);
    }
  }*/

  boolean sendSwitchAction(byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadSwitchAction = { ACCESSORY_ID, SERVICE_SWITCH, ID_SWITCH_ACTION, value, 0 };
    setControl(&payloadSwitchAction);
    if (radio.write(&payloadSwitchAction, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_CONTACT_D)
  boolean sendContactState(int service, int id, byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadContactState = { ACCESSORY_ID, service, id, value, 0 };
    setControl(&payloadContactState);
    if (radio.write(&payloadContactState, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_LIGHT_D)
  boolean sendLight() {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    int ldrValue = readLDRValue();
    float ldrResistance = ((1023.0 / ldrValue) - 1) * LDR_REF_RESISTANCE;
    float ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);
    //float ldrLux = ldrResistance; // debugging
    //float ldrLux = ldrValue; // debugging
    byte id;
    float luxValue;
    if (ldrLux > (255 * KOEF_LIGHT_HIGH)) {
      ldrLux = 255 * KOEF_LIGHT_HIGH;
    }
    if (ldrLux > (255 * KOEF_LIGHT_MEDIUM)) {
      id = ID_LIGHT_HIGH;
      luxValue = ldrLux / KOEF_LIGHT_HIGH;   // on controller multiply by 100.0 - implementation for using 1 byte only
    } else if (ldrLux > (255 * KOEF_LIGHT_LOW)) {
      id = ID_LIGHT_MEDIUM;
      luxValue = ldrLux / KOEF_LIGHT_MEDIUM;   // on controller multiply by 10.0 - implementation for using 1 byte only
    } else {
      id = ID_LIGHT_LOW;
      luxValue = ldrLux / KOEF_LIGHT_LOW;   // on controller multiply by 1.0 - implementation for using 1 byte only
    }
    Payload payloadLight = { ACCESSORY_ID, SERVICE_LIGHT, id, static_cast<byte>(luxValue), 0 };
    setControl(&payloadLight);
    if (radio.write(&payloadLight, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_TEMPERATURE_D)
  boolean sendTemperature() {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    byte temp = readTemperatureValue();
    Payload payloadTemperature = { ACCESSORY_ID, SERVICE_TEMPERATURE, ID_TEMPERATURE, temp , 0 };
    setControl(&payloadTemperature);
    if (radio.write(&payloadTemperature, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_HUMIDITY_D)
  boolean sendHumidity() {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    byte humidity = readHumidityValue();
    Payload payloadHumidity = { ACCESSORY_ID, SERVICE_HUMIDITY, ID_HUMIDITY, humidity , 0 };
    setControl(&payloadHumidity);
    if (radio.write(&payloadHumidity, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_AIR_QUALITY_D)
  boolean sendAirQuality(byte id, int value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    Payload payload = { ACCESSORY_ID, SERVICE_AIR_QUALITY, id, static_cast<byte>(value), 0 };
    setControl(&payload);
    if (radio.write(&payload, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_REPEATER_D)
  boolean forwardPayload(byte accessoryId, int service, byte id, byte value, byte radioId) {
    boolean result = false;
    radio.stopListening();
    radio.closeReadingPipe(0);
    radio.openWritingPipe(radioId);
    sendDelay(100);
    Payload payload = { accessoryId, service, id, value, 0 };
    setControl(&payload);
    if (radio.write(&payload, sizeof(Payload))) {
      result = true;
    }
    radio.openReadingPipe(0, RADIO_ID);
    radio.startListening();
    return result;
  }
#endif  

#if defined(SERVICE_RELAY_D)
  void doRelayAction(byte action) {
    if (action == 0) {
      if (reverseRelayValue) {
        digitalWrite(PIN_RELAY_SWITCH, HIGH);
      } else {
        digitalWrite(PIN_RELAY_SWITCH, LOW);
      }
    } else {
      if (reverseRelayValue) {
        digitalWrite(PIN_RELAY_SWITCH, LOW);
      } else {
        digitalWrite(PIN_RELAY_SWITCH, HIGH);
      }
    }
  }
#endif  

#if defined(SERVICE_WINDOW_COVERING_D)
  void doOpenWindowCovering() {
    digitalWrite(PIN_MOTOR_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IN2, LOW);
  }
  
  void doCloseWindowCovering() {
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, HIGH);
  }
  
  void doStopWindowCovering() {
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
  }
  
  boolean sendWindowCoveringCurrentPosition(byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadWindowCoveringCurrentPosition = { ACCESSORY_ID, SERVICE_WINDOW_COVERING, ID_WINDOW_COVERING_CURRENT_POSITION, value, 0 };
    setControl(&payloadWindowCoveringCurrentPosition);
    if (radio.write(&payloadWindowCoveringCurrentPosition, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
  
  boolean sendWindowCoveringTargetPosition(byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadWindowCoveringTargetPosition = { ACCESSORY_ID, SERVICE_WINDOW_COVERING, ID_WINDOW_COVERING_TARGET_POSITION, value, 0 };
    setControl(&payloadWindowCoveringTargetPosition);
    if (radio.write(&payloadWindowCoveringTargetPosition, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
  
  boolean sendWindowCoveringPositionState(byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadWindowCoveringPositionState = { ACCESSORY_ID, SERVICE_WINDOW_COVERING, ID_WINDOW_COVERING_POSITION_STATE, value, 0 };
    setControl(&payloadWindowCoveringPositionState);
    if (radio.write(&payloadWindowCoveringPositionState, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_DOOR_D)
  void doChangeDoor(byte servoAngle) {
    //servoAngle = servoAngle + DOOR_SERVO_DEGREES_CORRECTION;
    //servo.write(servoAngle);
    //delay(100);
    if (servoAngle > 0) {
      digitalWrite(PIN_SERVO,HIGH);
    } else {
      digitalWrite(PIN_SERVO,LOW);
    }
  }
  
  boolean sendDoorCurrentPosition(byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadDoorCurrentPosition = { ACCESSORY_ID, SERVICE_DOOR, ID_DOOR_CURRENT_POSITION, value, 0 };
    setControl(&payloadDoorCurrentPosition);
    if (radio.write(&payloadDoorCurrentPosition, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
  
  boolean sendDoorTargetPosition(byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadDoorTargetPosition = { ACCESSORY_ID, SERVICE_DOOR, ID_DOOR_TARGET_POSITION, value, 0 };
    setControl(&payloadDoorTargetPosition);
    if (radio.write(&payloadDoorTargetPosition, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
  
  boolean sendDoorPositionState(byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadDoorPositionState = { ACCESSORY_ID, SERVICE_DOOR, ID_DOOR_POSITION_STATE, value, 0 };
    setControl(&payloadDoorPositionState);
    if (radio.write(&payloadDoorPositionState, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

#if defined(SERVICE_BATTERY_LEVEL_D)
  boolean sendBatteryLevel() {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    float vccPerc = vcc.Read_Perc(VCC_MIN, VCC_MAX, true);
    Payload payloadBatteryLevel = { ACCESSORY_ID, SERVICE_BATTERY_LEVEL, ID_BATTERY_LEVEL, static_cast<byte>(vccPerc), 0 };
    setControl(&payloadBatteryLevel);
    if (radio.write(&payloadBatteryLevel, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }
#endif  

boolean sendTest(byte service, byte value) {
    boolean result = false;
    stopListening();
    radio.openWritingPipe(CONTROLLER_RADIO_ID);
    sendDelay(100);
    Payload payloadTest = { ACCESSORY_ID, service, ID_TEST, value, 0 };
    setControl(&payloadTest);
    if (radio.write(&payloadTest, sizeof(Payload))) {
      result = true;
    }
    startListening();
    return result;
  }

boolean isServiceSupported(int service) {
  return (ACCESSORY_SERVICE & service) == service;
}

boolean isAccessoryServiceSupported(int accessoryService, int service) {
  return (accessoryService & service) == service;
}

// radio takes ~14 mA in listening mode so it's important to optimize implementation and minimize listening time for battery powered accessories
boolean isListeningMode() {
  boolean repeater = false;
  #if defined(SERVICE_REPEATER_D)
    repeater = true;
  #endif  
  return /*isServiceSupported(SERVICE_SWITCH) ||*/ isServiceSupported(SERVICE_OUTLET) || isServiceSupported(SERVICE_LIGHT_BULB) || isServiceSupported(SERVICE_WINDOW_COVERING) || isServiceSupported(SERVICE_DOOR) || repeater;
}

void stopListening() {
  if (isListeningMode()) {
    radio.stopListening();
    radio.closeReadingPipe(0);
  }
}

void startListening() {
  if (isListeningMode()) {
    radio.openReadingPipe(0, RADIO_ID);
    radio.startListening();
  }
}

#if defined(SERVICE_REPEATER_D) || defined(SERVICE_DOOR_D)
  void EEPROMWriteByte(int address, byte value) {
    EEPROM.write(address, value);
  }
  
  void EEPROMUpdateByte(int address, byte value) {
    EEPROM.update(address, value);
  }
  
  byte EEPROMReadByte(int address) {
    return EEPROM.read(address);
  }
#endif  

void sendDelay(unsigned long ms) {
  //delay(ms); // delay is not needed
}

#if defined(SERVICE_DOOR_D)
  /*void refreshSoftwareServo() {
    SoftwareServo::refresh();
    delay(10);
  }*/
#endif  

#if defined(SERVICE_AIR_QUALITY_D)
  int read_int(byte address, byte parameter) {
    i2c_start_wait((address << 1) | I2C_WRITE);
    i2c_write(parameter);
    //si.i2c_stop();
    i2c_rep_start((address << 1) | I2C_READ);
    ///i2c_start_wait((address << 1) | I2C_READ);
    byte l = i2c_read(false);
    byte h = i2c_read(true);
    int result = ((l << 0) & 0xFF) + ((h << 8) & 0xFF00);
    i2c_stop();
    return result;  
  }

  void write_int(byte address, byte parameter) {
    i2c_start_wait((address << 1) | I2C_WRITE);
    i2c_write(parameter);
    i2c_stop();
  }
#endif  
