  #include <printf.h>
#include <RF24.h>
//#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <IA.h>
#include <EEPROM.h>

// for mega 2560 use good (short, blue) usb cable to delivery enough power, otherwise upload is not working and radio module with antenna too
// in file PubSubClient.h (C:\Users\Kostya\Documents\Arduino\libraries\PubSubClient\src) make the following change: #define MQTT_MAX_PACKET_SIZE 256

// migrate to RF24 v1.3.11 - done

// TODO maybe I have to remove delay before sending, investigate

#define DEBUG

#ifdef DEBUG
 #define DEBUG_PRINT(x)    Serial.print (x)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTLN(x)
#endif 

// IAController 1-st floor

const byte CONTROLLER_RADIO_ID = 255;
#define mqttControllerName  "IAController"
byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 178, 210);
#define paLevel RF24_PA_MAX // some (new??) radio modules are not working with RF24_PA_MAX, RF24_PA_HIGH and even RF24_PA_LOW, but "green" module from amazon.de with shell is working
//#define paLevel RF24_PA_MIN
#define channel 76


// IAController 2-nd floor
/*
const byte CONTROLLER_RADIO_ID = 254; 
#define mqttControllerName  "IAController2" 
byte mac[] = { 0xAE, 0x56, 0x96, 0x65, 0x0E, 0x37 };
IPAddress ip(192, 168, 178, 209);
#define paLevel RF24_PA_MAX // some (new??) radio modules are not working with RF24_PA_MAX, RF24_PA_HIGH and even RF24_PA_LOW, but "green" module from amazon.de with shell is working
//#define paLevel RF24_PA_MIN
#define channel 83
*/

// arduino mega
#define PIN_CE 53
#define PIN_CSN 46

RF24 radio(PIN_CE, PIN_CSN);

IPAddress mqttServer(192, 168, 178, 160);

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

volatile unsigned long lastConnectMqttAttempt = 0;
volatile unsigned long lastReceiveRadioPayload = 0;
const byte DELAY_BEFORE_SEND = 100; // milliseconds

const byte MAX_COUNT_ACCESSORIES = 200;
volatile byte reachabilityTimer[MAX_COUNT_ACCESSORIES];
volatile byte accessoryState[MAX_COUNT_ACCESSORIES];

volatile unsigned long lastReachability = 0;
const unsigned long TIMER_REACHABILITY = 60000; // 1 min
const byte TIMEOUT_REACHABILITY = 4; // time ~ TIMEOUT_REACHABILITY x (TIMER_REACHABILITY + 1) (+- up to TIMER_REACHABILITY)

const byte LOW_BATTERY_VALUE = 15; // percentage

const String PAYLOAD_STRING_NAME = "{\"name\": \"";
const String PAYLOAD_STRING_SERVICE_NAME = "\", \"service_name\": \"";
const String PAYLOAD_STRING_SERVICE = "\", \"service\": \"";
const String PAYLOAD_STRING_CURRENT_TEMPERATURE_PROPERTIES = "{\"minValue\": -50, \"maxValue\": 70, \"minStep\": 0.5}";
const String PAYLOAD_STRING_CURRENT_AMBIENT_LIGHT_LEVEL_PROPERTIES = "{\"minValue\": 0, \"maxValue\": 100000}";

void callback(char* topic, byte* payload, unsigned int length) {
  DEBUG_PRINT(F("got mqtt payload ["));
  DEBUG_PRINT(topic);
  DEBUG_PRINT(F("] "));
  for (int i=0; i<length; i++) {
    DEBUG_PRINT((char)payload[i]);
  }
  DEBUG_PRINTLN();
  if (strcmp(topic, "homebridge/from/set") == 0) {
    DEBUG_PRINTLN(F("got homebridge/from/set"));
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    const char* accessoryName = root[F("name")];
    const char* characteristic = root[F("characteristic")];
    const char* value = root[F("value")];
    DEBUG_PRINTLN(accessoryName);
    DEBUG_PRINTLN(characteristic);
    DEBUG_PRINTLN(value);
    byte accessoryId = atoi(strtok(accessoryName, " "));
    if ((accessoryId>0) && (accessoryId<MAX_COUNT_ACCESSORIES)) {
      int address = accessoryId*2+1;
      int accessoryService = EEPROMReadInt(address);
      if (isServiceSupported(accessoryService,SERVICE_SWITCH)) { 
        if (strcmp(characteristic, "On") == 0) {
          DEBUG_PRINT(F("send switch action "));     
          if (strcmp(value,"false") == 0) {
            DEBUG_PRINT(F("-off- "));
            accessoryState[accessoryId] = 0;
            if (sendSwitchAction(accessoryId,0)) {
              DEBUG_PRINTLN(F("ok"));     
            } else {
              DEBUG_PRINTLN(F("failed")); 
            }
          } else {
            DEBUG_PRINT(F("-on- "));
            accessoryState[accessoryId] = 1;
            if (sendSwitchAction(accessoryId,1)) {
              DEBUG_PRINTLN(F("ok"));     
            } else {
              DEBUG_PRINTLN(F("failed")); 
            }
          }
        }
      }
      if (isServiceSupported(accessoryService,SERVICE_OUTLET)) {
        if (strcmp(characteristic, "On") == 0) {
          DEBUG_PRINT(F("send outlet action "));
          if (strcmp(value,"false") == 0) {
            DEBUG_PRINT(F("-off- "));
            if (sendOutletAction(accessoryId,0)) {
              DEBUG_PRINTLN(F("ok"));     
            } else {
              DEBUG_PRINTLN(F("failed")); 
            }
          } else {
            DEBUG_PRINT(F("-on- "));
            if (sendOutletAction(accessoryId,1)) {
              DEBUG_PRINTLN(F("ok"));     
            } else {
              DEBUG_PRINTLN(F("failed")); 
            }
          }
        }
      }
      if (isServiceSupported(accessoryService,SERVICE_LIGHT_BULB)) {
        if (strcmp(characteristic, "On") == 0) {
          DEBUG_PRINT(F("send light bulb action "));
          if (strcmp(value,"false") == 0) {
            DEBUG_PRINT(F("-off- "));
            if (sendLightBulbAction(accessoryId,0)) {
              DEBUG_PRINTLN(F("ok"));     
            } else {
              DEBUG_PRINTLN(F("failed")); 
            }
          } else {
            DEBUG_PRINT(F("-on- "));
            if (sendLightBulbAction(accessoryId,1)) {
              DEBUG_PRINTLN(F("ok"));     
            } else {
              DEBUG_PRINTLN(F("failed")); 
            }
          }
        }
      }
      if (isServiceSupported(accessoryService,SERVICE_WINDOW_COVERING)) {
        if (strcmp(characteristic, "TargetPosition") == 0) {
          DEBUG_PRINT(F("send window covering target position "));
          DEBUG_PRINT(value);
          DEBUG_PRINT(F(" "));
          if (sendWindowCoveringTargetPosition(accessoryId,atoi(value))) {
            DEBUG_PRINTLN(F("ok"));     
          } else {
            DEBUG_PRINTLN(F("failed")); 
          }
        }
      }
      if (isServiceSupported(accessoryService,SERVICE_DOOR)) {
        if (strcmp(characteristic, "TargetPosition") == 0) {
          DEBUG_PRINT(F("send door target position "));
          DEBUG_PRINT(value);
          DEBUG_PRINT(F(" "));
          if (sendDoorTargetPosition(accessoryId,atoi(value))) {
            DEBUG_PRINTLN(F("ok"));     
          } else {
            DEBUG_PRINTLN(F("failed")); 
          }
        }
      }
    }
  } /*else if (strcmp(topic, "homebridge/to/set/reachability") == 0) {
    DEBUG_PRINTLN(F("got homebridge/to/set/reachability"));
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    const char* accessoryName = root[F("name")];
    const char* reachable = root[F("reachable")];
    DEBUG_PRINTLN(accessoryName);
    DEBUG_PRINTLN(reachable);
    byte accessoryId = atoi(strtok(accessoryName, " "));
    if ((accessoryId>=0) && (accessoryId<MAX_COUNT_ACCESSORIES)) {
      reachabilityTimer[accessoryId] = 0;
    }
  }*/
}

boolean connectMqtt() {
  //boolean subscribed = true;
  DEBUG_PRINTLN(F("mqtt connection attempt..."));
  if (mqttClient.connect(mqttControllerName)) {
    DEBUG_PRINTLN(F("mqtt connected"));
    /*if (!client.subscribe("homebridge/from/response")) {
      DEBUG_PRINTLN(F("mqtt homebridge/from/response subscribe error"));  
      subscribed = false; 
    }*/
    if (!mqttClient.subscribe("homebridge/from/set")) {
      DEBUG_PRINTLN(F("mqtt subscribe error"));  
      //DEBUG_PRINTLN(F("mqtt homebridge/from/set subscribe error"));  
      //subscribed = false;
    }
    /*if (!client.subscribe("homebridge/to/set/reachability")) {
      DEBUG_PRINTLN(F("mqtt homebridge/to/set/reachability subscribe error")); 
      subscribed = false; 
    }*/
  } else {
    DEBUG_PRINTLN(F("mqtt not connected"));
  }
  //return client.connected() && subscribed;
  return mqttClient.connected();
}

void setup() {
  Serial.begin(115200);
  //EEPROM.write(0, 255); // reset once for initialization for new board
  byte eepromAccessoriesInitialized = EEPROM.read(0);
  DEBUG_PRINT(F("eepromAccessoriesInitialized = "));
  DEBUG_PRINTLN(eepromAccessoriesInitialized);
  if (eepromAccessoriesInitialized == 255) {
    DEBUG_PRINTLN(F("EEPROM accessories not initialized"));
    EEPROM.write(0, 1);
    // not initialized yet 
    for (byte i=0; i<MAX_COUNT_ACCESSORIES; i++) {
      int address = i*2+1;
      EEPROMWriteInt(address, 0);
    }
  } else {
    // already initialized
    //EEPROMUpdateInt(2*2+1, 0);  
    //EEPROMUpdateInt(112*2+1, 0);
    //EEPROMUpdateInt(113*2+1, 0);
    //EEPROMUpdateInt(25*2+1, 0);
    //EEPROMUpdateInt(1*2+1, 0);
    for (byte i=0; i<MAX_COUNT_ACCESSORIES; i++) {
      int address = i*2+1;
      int accessoryService = EEPROMReadInt(address);
      if (accessoryService != 0) {
        DEBUG_PRINT(F("service stored in EEPROM memory for accessory "));
        DEBUG_PRINT(i);
        DEBUG_PRINT(F(" = "));
        DEBUG_PRINTLN(accessoryService);
      }
    }
    // uncomment if you need to clear stored accessories service
    /*for (byte i=0; i<MAX_COUNT_ACCESSORIES; i++) {
      int address = i*2+1;
      int accessoryService = EEPROMReadInt(address);
      if (accessoryService != 0) {
        EEPROMUpdateInt(address, 0);  
        DEBUG_PRINT("service stored in EEPROM memory for accessory ");
        DEBUG_PRINT(i);
        DEBUG_PRINTLN(F(" cleared"));
      }
    }*/
  } 
  //EEPROM.write(513, 255); // reset once for initialization for new board
  byte eepromRepeatersInitialized = EEPROM.read(513);
  DEBUG_PRINT(F("eepromRepeatersInitialized = "));
  DEBUG_PRINTLN(eepromRepeatersInitialized);
  if (eepromRepeatersInitialized == 255) {
    DEBUG_PRINTLN(F("EEPROM repeaters not initialized"));
    EEPROM.write(513, 1);
    // not initialized yet 
    for (byte i=0; i<MAX_COUNT_REPEATERS; i++) {
      int address =  513 + i+1;
      EEPROMWriteByte(address, 255);
    }
  } else {
    // already initialized
    //EEPROMUpdateByte(513 + 19+1, 255);  
    //EEPROMUpdateByte(513 + 7+1, 255);  
    for (byte i=0; i<MAX_COUNT_REPEATERS; i++) {
      int address = 513 + i+1;
      byte repeaterId = EEPROMReadByte(address);
      if (repeaterId != 255) {
        DEBUG_PRINT(F("repeater id stored in EEPROM memory for accessory "));
        DEBUG_PRINT(i);
        DEBUG_PRINT(F(" = "));
        DEBUG_PRINTLN(repeaterId);
      }
    }
    // uncomment if you need to clear stored accessories repeaters
    /*for (byte i=0; i<MAX_COUNT_REPEATERS; i++) {
      int address = 513 + i+1;
      byte repeaterId = EEPROMReadByte(address);
      if (repeaterId != 0) {
        EEPROMUpdateByte(address, 255);  
        DEBUG_PRINT("repeater id stored in EEPROM memory for accessory ");
        DEBUG_PRINT(i);
        DEBUG_PRINTLN(F(" cleared"));
      }
    }*/
  }
  for (byte i=0; i<MAX_COUNT_ACCESSORIES; i++) {
    reachabilityTimer[i] = 0;
    accessoryState[i] = 0;
  }
  radio.begin();
  radio.setAutoAck(1);
  //radio.enableAckPayload();                     // Allow optional ack payloads
  //radio.enableDynamicPayloads();                // Ack payloads are dynamic payloads                
  //radio.setCRCLength(RF24_CRC_16);
  radio.setPALevel(paLevel);                  
  //radio.setDataRate(RF24_1MBPS);
  radio.setChannel(channel);
  radio.setRetries(30, 30);
  radio.setPayloadSize(sizeof(Payload));
  radio.openReadingPipe(1, CONTROLLER_RADIO_ID);
  radio.startListening();
  printf_begin();
  #ifdef DEBUG
    radio.printDetails();
  #endif   
  DEBUG_PRINT(F("radio "));
  if (radio.isChipConnected()) {
    DEBUG_PRINTLN(F("connected")); 
  } else {
    DEBUG_PRINTLN(F("not connected, reset board")); 
  }  
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(callback);
  Ethernet.begin(mac, ip);
  lastReachability = 0;
  lastConnectMqttAttempt = 0;
  lastReceiveRadioPayload = 0;
  DEBUG_PRINT(F("payload size ")); 
  DEBUG_PRINTLN(sizeof(Payload)); 
  // remove accessories TODO
  /*int address = 4*2+1;
  EEPROMUpdateInt(address, 0);
  address = 14*2+1;
  EEPROMUpdateInt(address, 0);*/
}

void loop(void) {
  unsigned long now = millis();
  boolean gotPayload = false;
  Payload payload;
  if (radio.available()) { 
    lastReceiveRadioPayload = now;
    radio.read(&payload, sizeof(Payload));
    if (checkControl(payload)) {
      gotPayload = true;    
      DEBUG_PRINTLN(F("got radio payload"));
    }
  }
  if (!mqttClient.connected()) {
    if (gotPayload) {
      DEBUG_PRINTLN(F("ignored, no mqtt connection"));
    }
    if (millisDifference(now, lastConnectMqttAttempt) > 5000) {
      lastConnectMqttAttempt = now;
      // Attempt to connect
      if (connectMqtt()) {
        lastConnectMqttAttempt = 0;
        lastReachability = now;
      }
    }
  } else {
    // MQTT Client connected
    if (millisDifference(now, lastReachability) > TIMER_REACHABILITY) {
      lastReachability = now;
      for (byte i=0; i<MAX_COUNT_ACCESSORIES; i++) {
        int address = i*2+1;
        int accessoryService = EEPROMReadInt(address);
        if (accessoryService > 0) {
          reachabilityTimer[i] = reachabilityTimer[i] + 1;
          if (reachabilityTimer[i] > TIMEOUT_REACHABILITY) {
            String accessoryName = String(i);
            DEBUG_PRINT(F("update accessory "));
            DEBUG_PRINT(accessoryName);
            DEBUG_PRINTLN(F(" reachability = false"));
            setAccessoryReachability(accessoryName, false);
            reachabilityTimer[i] = TIMEOUT_REACHABILITY + 1;
          } 
        }
      } 
    }
    if (gotPayload) {
      String accessoryName;
      String serviceName; 
      String value;
      // TODO temporary
      if ((payload.accessoryId>=0) && (payload.accessoryId<MAX_COUNT_ACCESSORIES)) {
        DEBUG_PRINT(F("from accessory with id "));
        DEBUG_PRINT(payload.accessoryId);
        DEBUG_PRINT(F(" and service "));  
        DEBUG_PRINTLN(payload.service);
        accessoryName = String(payload.accessoryId);
        int address = payload.accessoryId*2+1;
        int accessoryService = EEPROMReadInt(address);
        if (payload.id == ID_PAIRING) {
          DEBUG_PRINTLN(F("it's pairing payload")); 
          if (accessoryService != payload.service) {
            // remove existing accessory if service of accessory was changed
            if (accessoryService != 0) {
              DEBUG_PRINTLN(F("remove accessory"));
              removeAccessory(accessoryName);
              accessoryService = 0;
            }
            // add accessory if not added already
            if (accessoryService == 0) {
              boolean accessoryAdded = false;
              DEBUG_PRINT(F("add acessory "));
              DEBUG_PRINTLN(accessoryName);
              if (isServiceSupported(payload.service,SERVICE_MOTION)) {
                DEBUG_PRINTLN(F("add service MotionSensor"));
                serviceName = accessoryName + F(" MIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("MotionSensor"));      
                } else {
                  addAccessory(accessoryName, serviceName, F("MotionSensor")); 
                  accessoryAdded = true;
                }
                if (isServiceSupported(payload.service,SERVICE_LIGHT)) {
                  DEBUG_PRINTLN(F("add service MotionSensor for darkness"));
                  serviceName = accessoryName + F(" MIA D");
                  if (accessoryAdded) {
                    addService(accessoryName, serviceName, F("MotionSensor"));      
                  } else {
                    addAccessory(accessoryName, serviceName, F("MotionSensor")); 
                    accessoryAdded = true;
                  }  
                }                
              }
              if (isServiceSupported(payload.service,SERVICE_SWITCH)) {
                DEBUG_PRINTLN(F("add service Switch"));  
                serviceName = accessoryName + F(" SIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("Switch"));      
                } else {
                  addAccessory(accessoryName, serviceName, F("Switch")); 
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_OUTLET)) {
                DEBUG_PRINTLN(F("add service Outlet"));  
                serviceName = accessoryName + F(" OIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("Outlet"));      
                } else {
                  addAccessory(accessoryName, serviceName, F("Outlet")); 
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_LIGHT_BULB)) {
                DEBUG_PRINTLN(F("add service LightBulb"));  
                serviceName = accessoryName + F(" LBIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("Lightbulb"));      
                } else {
                  addAccessory(accessoryName, serviceName, F("Lightbulb")); 
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_LIGHT)) {
                DEBUG_PRINTLN(F("add service LightSensor"));
                serviceName = accessoryName + F(" LIA");
                if (accessoryAdded) {
                  addServiceWithCharacteristicProperties(accessoryName, serviceName, F("LightSensor"), F("CurrentAmbientLightLevel"), PAYLOAD_STRING_CURRENT_AMBIENT_LIGHT_LEVEL_PROPERTIES);  
                } else {
                  addAccessoryWithCharacteristicProperties(accessoryName, serviceName, F("LightSensor"), F("CurrentAmbientLightLevel"), PAYLOAD_STRING_CURRENT_AMBIENT_LIGHT_LEVEL_PROPERTIES);  
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_TEMPERATURE)) {
                DEBUG_PRINTLN(F("add service TemperatureSensor"));
                serviceName = accessoryName + F(" TIA");
                if (accessoryAdded) {
                  addServiceWithCharacteristicProperties(accessoryName, serviceName, F("TemperatureSensor"), F("CurrentTemperature"), PAYLOAD_STRING_CURRENT_TEMPERATURE_PROPERTIES);  
                } else {
                  addAccessoryWithCharacteristicProperties(accessoryName, serviceName, F("TemperatureSensor"), F("CurrentTemperature"), PAYLOAD_STRING_CURRENT_TEMPERATURE_PROPERTIES);  
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_HUMIDITY)) {
                DEBUG_PRINTLN(F("add service HumiditySensor"));
                serviceName = accessoryName + F(" HIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("HumiditySensor"));  
                } else {
                  addAccessory(accessoryName, serviceName, F("HumiditySensor"));  
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_AIR_QUALITY)) {
                DEBUG_PRINTLN(F("add service AirQualitySensor"));
                serviceName = accessoryName + F(" AQIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("AirQualitySensor"));  
                } else {
                  addAccessory(accessoryName, serviceName, F("AirQualitySensor"));  
                  accessoryAdded = true;
                }
                addCharacteristicDefault(accessoryName, serviceName, F("AirQualitySensor"), F("PM1Density"));
                addCharacteristicDefault(accessoryName, serviceName, F("AirQualitySensor"), F("PM2_5Density"));
                addCharacteristicDefault(accessoryName, serviceName, F("AirQualitySensor"), F("PM10Density"));
                DEBUG_PRINTLN(F("add service CarbonDioxideSensor"));
                serviceName = accessoryName + F(" CDXIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("CarbonDioxideSensor"));  
                } else {
                  addAccessory(accessoryName, serviceName, F("CarbonDioxideSensor"));  
                  accessoryAdded = true;
                }
                addCharacteristicDefault(accessoryName, serviceName, F("CarbonDioxideSensor"), F("CarbonDioxideLevel"));
              }
              if (isServiceSupported(payload.service,SERVICE_LEAK)) {
                DEBUG_PRINTLN(F("add service LeakSensor"));  
                serviceName = accessoryName + F(" LKIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("LeakSensor"));      
                } else {
                  addAccessory(accessoryName, serviceName, F("LeakSensor")); 
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_1_CONTACT)) {
                DEBUG_PRINTLN(F("add service ContactSensor 1"));  
                serviceName = accessoryName + F(" CIA 1");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("ContactSensor"));      
                } else {
                  addAccessory(accessoryName, serviceName, F("ContactSensor")); 
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_2_CONTACT)) {
                DEBUG_PRINTLN(F("add service ContactSensor 2"));  
                serviceName = accessoryName + F(" CIA 2");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("ContactSensor"));      
                } else {
                  addAccessory(accessoryName, serviceName, F("ContactSensor")); 
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_WINDOW_COVERING)) {
                DEBUG_PRINTLN(F("add service WindowCovering"));  
                serviceName = accessoryName + F(" WCIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("WindowCovering"));      
                } else {
                  addAccessory(accessoryName, serviceName, F("WindowCovering")); 
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_DOOR)) {
                DEBUG_PRINTLN(F("add service Door"));  
                serviceName = accessoryName + F(" DIA");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("Door"));      
                } else {
                  addAccessory(accessoryName, serviceName, F("Door")); 
                  accessoryAdded = true;
                }
              }
              if (isServiceSupported(payload.service,SERVICE_BATTERY_LEVEL)) {
                DEBUG_PRINTLN(F("add service BatteryService"));
                serviceName = accessoryName + F(" Battery");
                if (accessoryAdded) {
                  addService(accessoryName, serviceName, F("BatteryService"));    
                }
              }
              accessoryService = payload.service;
              reachabilityTimer[payload.accessoryId] = 0;
              EEPROMUpdateInt(address, accessoryService);
            }  
          }
          DEBUG_PRINTLN(F("setAccessoryInformation"));
          setAccessoryInformation(accessoryName, accessoryName);
          // store repeater id
          int address1 = 513 + payload.accessoryId+1;  
          if ((payload.value>0) && (payload.value<MAX_COUNT_REPEATERS)) { // TODO change to payload.value>=0 and in accessory send 255 instead of 0 for payload value in pairing request 
            DEBUG_PRINT(F("pairing via repeater "));
            DEBUG_PRINTLN(payload.value);
            EEPROMUpdateByte(address1, payload.value); 
          } else {
            DEBUG_PRINTLN(F("direct pairing (no repeater)"));
            EEPROMUpdateByte(address1, 255); 
          }
          // send pairing confirmation
          DEBUG_PRINT(F("send pairing confirmation "));
          if (sendPairingResponse(payload.accessoryId, payload.service)) {
            DEBUG_PRINTLN(F("ok")); 
            DEBUG_PRINT(F("update accessory "));
            DEBUG_PRINT(accessoryName);
            DEBUG_PRINTLN(F(" reachability = true"));
            setAccessoryReachability(accessoryName, true);
            reachabilityTimer[payload.accessoryId] = 0;
          } else {
            DEBUG_PRINTLN(F("failed"));  
          }
        } else {
          DEBUG_PRINTLN(F("it's data payload")); 
          if (isServiceSupported(accessoryService,payload.service)) {
            DEBUG_PRINTLN(F("service is paired"));   
            if (payload.id == ID_MOTION_ACTION) {
              DEBUG_PRINT(F("motion action ")); 
              DEBUG_PRINTLN(payload.value); 
              String characteristic = F("MotionDetected");
              if (isServiceSupported(accessoryService,SERVICE_LIGHT)) {
                if (payload.value == 2) {
                  serviceName = accessoryName + F(" MIA");
                  setCharacteristicValue(accessoryName, serviceName, characteristic, "1");    
                  serviceName = accessoryName + F(" MIA D");
                  setCharacteristicValue(accessoryName, serviceName, characteristic, "1");       
                } else if (payload.value == 1) {
                  serviceName = accessoryName + F(" MIA");
                  setCharacteristicValue(accessoryName, serviceName, characteristic, "1");    
                  serviceName = accessoryName + F(" MIA D");
                  setCharacteristicValue(accessoryName, serviceName, characteristic, "0");    
                } else {
                  serviceName = accessoryName + F(" MIA");
                  setCharacteristicValue(accessoryName, serviceName, characteristic, "0");  
                  serviceName = accessoryName + F(" MIA D");
                  setCharacteristicValue(accessoryName, serviceName, characteristic, "0");
                }
              } else {
                serviceName = accessoryName + F(" MIA");
                value = String(payload.value);
                setCharacteristicValue(accessoryName, serviceName, characteristic, value);  
              }                
            } else if ((payload.id == ID_LIGHT_LOW) || (payload.id == ID_LIGHT_MEDIUM) || (payload.id == ID_LIGHT_HIGH)) {
              int lux;
              DEBUG_PRINT(F("light level ")); 
              if (payload.id == ID_LIGHT_LOW) {
                lux = payload.value * KOEF_LIGHT_LOW; 
                DEBUG_PRINT(F("LOW ")); 
              } else if (payload.id == ID_LIGHT_MEDIUM) {
                lux = payload.value * KOEF_LIGHT_MEDIUM; 
                DEBUG_PRINT(F("MEDIUM ")); 
              } else {
                lux = payload.value * KOEF_LIGHT_HIGH;  
                DEBUG_PRINT(F("HIGH ")); 
              }
              DEBUG_PRINTLN(lux); 
              serviceName = accessoryName + F(" LIA");
              value = String(lux);
              setCharacteristicValue(accessoryName, serviceName, F("CurrentAmbientLightLevel"), value);
            } else if (payload.id == ID_TEMPERATURE) {
              float temperature = (payload.value / 2.0) - 50.0;
              DEBUG_PRINT(F("temperature ")); 
              DEBUG_PRINTLN(temperature); 
              serviceName = accessoryName + F(" TIA");
              value = String(temperature);
              setCharacteristicValue(accessoryName, serviceName, F("CurrentTemperature"), value);
            } else if (payload.id == ID_HUMIDITY) {
              float humidity = (payload.value / 2.0);
              DEBUG_PRINT(F("humidity ")); 
              DEBUG_PRINTLN(humidity); 
              serviceName = accessoryName + F(" HIA");
              value = String(humidity);
              setCharacteristicValue(accessoryName, serviceName, F("CurrentRelativeHumidity"), value);
            } else if (payload.id == ID_AIR_QUALITY) {
              int aq = payload.value;
              DEBUG_PRINT(F("air quality ")); 
              DEBUG_PRINTLN(aq);             
              serviceName = accessoryName + F(" AQIA");
              value = String(aq);
              setCharacteristicValue(accessoryName, serviceName, F("AirQuality"), value);
            } else if (payload.id == ID_PM_10_STANDARD) {
              int pm = payload.value;
              DEBUG_PRINT(F("pm 10 standard ")); 
              DEBUG_PRINTLN(pm);             
              serviceName = accessoryName + F(" AQIA");
              value = String(pm);
              setCharacteristicValue(accessoryName, serviceName, F("PM1Density"), value);
            } else if (payload.id == ID_PM_25_STANDARD) {
              int pm = payload.value;
              DEBUG_PRINT(F("pm 25 standard ")); 
              DEBUG_PRINTLN(pm);             
              serviceName = accessoryName + F(" AQIA");
              value = String(pm);
              setCharacteristicValue(accessoryName, serviceName, F("PM2_5Density"), value);
            } else if (payload.id == ID_PM_100_STANDARD) {
              int pm = payload.value;
              DEBUG_PRINT(F("pm 100 standard ")); 
              DEBUG_PRINTLN(pm);             
              serviceName = accessoryName + F(" AQIA");
              value = String(pm);
              setCharacteristicValue(accessoryName, serviceName, F("PM10Density"), value);
            } else if (payload.id == ID_PM_10_ENV) {
              int pm = payload.value;
              DEBUG_PRINT(F("pm 10 env ")); 
              DEBUG_PRINTLN(pm);             
              //serviceName = accessoryName + F(" AQIA");
              //value = String(pm);
              //setCharacteristicValue(accessoryName, serviceName, F("AirQuality"), value);
            } else if (payload.id == ID_PM_25_ENV) {
              int pm = payload.value;
              DEBUG_PRINT(F("pm 25 env ")); 
              DEBUG_PRINTLN(pm);             
              //serviceName = accessoryName + F(" AQIA");
              //value = String(pm);
              //setCharacteristicValue(accessoryName, serviceName, F("AirQuality"), value);
            } else if (payload.id == ID_PM_100_ENV) {
              int pm = payload.value;
              DEBUG_PRINT(F("pm 100 env ")); 
              DEBUG_PRINTLN(pm);             
              //serviceName = accessoryName + F(" AQIA");
              //value = String(aq);
              //setCharacteristicValue(accessoryName, serviceName, F("AirQuality"), value);
            } else if ((payload.id == ID_PPM_CO2_LOW) || (payload.id == ID_PPM_CO2_MEDIUM) || (payload.id == ID_PPM_CO2_HIGH)) {
              int ppm;
              DEBUG_PRINT(F("ppm CO2 ")); 
              if (payload.id == ID_PPM_CO2_LOW) {
                ppm = payload.value * KOEF_PPM_CO2_LOW; 
                DEBUG_PRINT(F("LOW ")); 
              } else if (payload.id == ID_PPM_CO2_MEDIUM) {
                ppm = payload.value * KOEF_PPM_CO2_MEDIUM; 
                DEBUG_PRINT(F("MEDIUM ")); 
              } else {
                ppm = payload.value * KOEF_PPM_CO2_HIGH;  
                DEBUG_PRINT(F("HIGH ")); 
              }
              DEBUG_PRINTLN(ppm);             
              serviceName = accessoryName + F(" CDXIA");
              value = String(ppm);
              setCharacteristicValue(accessoryName, serviceName, F("CarbonDioxideLevel"), value);
              if (ppm > 2000) {
                setCharacteristicValue(accessoryName, serviceName, F("CarbonDioxideDetected"), "1");  
              } else {
                setCharacteristicValue(accessoryName, serviceName, F("CarbonDioxideDetected"), "0");
              }              
            } else if (payload.id == ID_LEAK_STATE) {
              DEBUG_PRINT(F("leak state ")); 
              DEBUG_PRINTLN(payload.value); 
              serviceName = accessoryName + F(" LKIA");
              value = String(payload.value);
              setCharacteristicValue(accessoryName, serviceName, F("LeakDetected"), value);  
            } else if (payload.id == ID_SWITCH_ACTION) {
              DEBUG_PRINT(F("switch action ")); 
              DEBUG_PRINTLN(payload.value); 
              serviceName = accessoryName + F(" SIA");
              // value == 2 => reverse switch stored state
              if (payload.value == 2) {
                byte previousState = accessoryState[payload.accessoryId];
                if (previousState == 0) {
                  accessoryState[payload.accessoryId] = 1;   
                  value = String(1); 
                } else {
                  accessoryState[payload.accessoryId] = 0;    
                  value = String(0); 
                }
              // value == 3 => repeat switch stored state  
              } else if (payload.value == 3) {
                value = String(accessoryState[payload.accessoryId]);
              } else {
                accessoryState[payload.accessoryId] = payload.value; 
                value = String(payload.value); 
              }
              setCharacteristicValue(accessoryName, serviceName, F("On"), value);  
            } else if (payload.id == ID_CONTACT_1_STATE) {
              DEBUG_PRINT(F("contact 1 state ")); 
              DEBUG_PRINTLN(payload.value); 
              serviceName = accessoryName + F(" CIA 1");
              value = String(payload.value);
              setCharacteristicValue(accessoryName, serviceName, F("ContactSensorState"), value);    
            } else if (payload.id == ID_CONTACT_2_STATE) {
              DEBUG_PRINT(F("contact 2 state ")); 
              DEBUG_PRINTLN(payload.value); 
              serviceName = accessoryName + F(" CIA 2");
              value = String(payload.value);
              setCharacteristicValue(accessoryName, serviceName, F("ContactSensorState"), value);    
            } else if (payload.id == ID_WINDOW_COVERING_CURRENT_POSITION) {
              DEBUG_PRINT(F("window covering current position ")); 
              DEBUG_PRINTLN(payload.value); 
              serviceName = accessoryName + F(" WCIA");
              value = String(payload.value);
              setCharacteristicValue(accessoryName, serviceName, F("CurrentPosition"), value);
            } else if (payload.id == ID_WINDOW_COVERING_TARGET_POSITION) {
              DEBUG_PRINT(F("window covering target position ")); 
              DEBUG_PRINTLN(payload.value); 
              serviceName = accessoryName + F(" WCIA");
              value = String(payload.value);
              setCharacteristicValue(accessoryName, serviceName, F("TargetPosition"), value);
            } else if (payload.id == ID_WINDOW_COVERING_POSITION_STATE) {
              DEBUG_PRINT(F("window covering position state ")); 
              if (payload.value==WINDOW_COVERING_POSITION_STATE_DECREASING) {
                DEBUG_PRINTLN(F("DECREASING"));   
              } else if (payload.value==WINDOW_COVERING_POSITION_STATE_INCREASING) {
                DEBUG_PRINTLN(F("INCREASING"));   
              } else {
                DEBUG_PRINTLN(F("STOPPED"));   
              }               
              serviceName = accessoryName + F(" WCIA");
              value = String(payload.value);
              setCharacteristicValue(accessoryName, serviceName, F("PositionState"), value);
            } else if (payload.id == ID_DOOR_CURRENT_POSITION) {
              DEBUG_PRINT(F("door current position ")); 
              DEBUG_PRINTLN(payload.value); 
              serviceName = accessoryName + F(" DIA");
              value = String(payload.value);
              setCharacteristicValue(accessoryName, serviceName, F("CurrentPosition"), value);
            } else if (payload.id == ID_DOOR_TARGET_POSITION) {
              DEBUG_PRINT(F("door target position ")); 
              DEBUG_PRINTLN(payload.value); 
              serviceName = accessoryName + F(" DIA");
              value = String(payload.value);
              setCharacteristicValue(accessoryName, serviceName, F("TargetPosition"), value);
            } else if (payload.id == ID_DOOR_POSITION_STATE) {
              DEBUG_PRINT(F("door position state ")); 
              if (payload.value==DOOR_POSITION_STATE_DECREASING) {
                DEBUG_PRINTLN(F("DECREASING"));   
              } else if (payload.value==DOOR_POSITION_STATE_INCREASING) {
                DEBUG_PRINTLN(F("INCREASING"));   
              } else {
                DEBUG_PRINTLN(F("STOPPED"));   
              }               
              serviceName = accessoryName + F(" DIA");
              value = String(payload.value);
              setCharacteristicValue(accessoryName, serviceName, F("PositionState"), value);
            } else if (payload.id == ID_BATTERY_LEVEL) {
              DEBUG_PRINT(F("battery level ")); 
              DEBUG_PRINTLN(payload.value);
              serviceName = accessoryName + F(" Battery");
              setBatteryCharacteristics(accessoryName, serviceName, payload.value);  
            } else if (payload.id == ID_REACHABILITY_PING) {
              DEBUG_PRINTLN(F("reachability ping")); 
            } else if (payload.id == ID_TEST) {
              DEBUG_PRINT(F("TEST ")); 
              DEBUG_PRINTLN(payload.value);
            } else {
              DEBUG_PRINT(F("unknown payload id "));
              DEBUG_PRINT(payload.id); 
              DEBUG_PRINT(F(" "));
              DEBUG_PRINTLN(payload.value);  
            }
          }
          DEBUG_PRINT(F("update accessory "));
          DEBUG_PRINT(accessoryName);
          DEBUG_PRINTLN(F(" reachability = true"));
          setAccessoryReachability(accessoryName, true);
          reachabilityTimer[payload.accessoryId] = 0;
        }        
      }
    }
  }
  mqttClient.loop();
}

boolean addAccessory(String accessoryName, String serviceName, String service) {
  String stringPayload = PAYLOAD_STRING_NAME + accessoryName + PAYLOAD_STRING_SERVICE_NAME + serviceName + PAYLOAD_STRING_SERVICE + service + "\"}";
  const char* payload = stringPayload.c_str();
  return mqttClient.publish("homebridge/to/add", payload);    
}

boolean addAccessoryWithCharacteristicProperties(String accessoryName, String serviceName, String service, String characteristic, String characteristicProperties) {
  String stringPayload = PAYLOAD_STRING_NAME + accessoryName + PAYLOAD_STRING_SERVICE_NAME + serviceName + PAYLOAD_STRING_SERVICE + service + "\", \"" + characteristic + "\": " + characteristicProperties + "}";
  const char* payload = stringPayload.c_str();
  return mqttClient.publish("homebridge/to/add", payload);    
}

boolean removeAccessory(String accessoryName) {
  String stringPayload = PAYLOAD_STRING_NAME + accessoryName + "\"}";
  const char* payload = stringPayload.c_str();
  return mqttClient.publish("homebridge/to/remove", payload);    
}

boolean addService(String accessoryName, String serviceName, String service) {
  String stringPayload = PAYLOAD_STRING_NAME + accessoryName + PAYLOAD_STRING_SERVICE_NAME + serviceName + PAYLOAD_STRING_SERVICE + service + "\"}";
  const char* payload = stringPayload.c_str();
  return mqttClient.publish("homebridge/to/add/service", payload);    
}

boolean addServiceWithCharacteristicProperties(String accessoryName, String serviceName, String service, String characteristic, String characteristicProperties) {
  String stringPayload = PAYLOAD_STRING_NAME + accessoryName + PAYLOAD_STRING_SERVICE_NAME + serviceName + PAYLOAD_STRING_SERVICE + service + "\", \"" + characteristic + "\": " + characteristicProperties + "}";
  const char* payload = stringPayload.c_str();
  return mqttClient.publish("homebridge/to/add/service", payload);    
}

boolean addCharacteristicWithProperties(String accessoryName, String serviceName, String service, String characteristic, String characteristicProperties) {
  String stringPayload = PAYLOAD_STRING_NAME + accessoryName + PAYLOAD_STRING_SERVICE_NAME + serviceName + PAYLOAD_STRING_SERVICE + service + "\", \"" + characteristic + "\": " + characteristicProperties + "}";
  const char* payload = stringPayload.c_str();
  return mqttClient.publish("homebridge/to/add", payload);    
}

boolean addCharacteristicDefault(String accessoryName, String serviceName, String service, String characteristic) {
  return addCharacteristicWithProperties(accessoryName,serviceName,service,characteristic,"default");  
}

boolean setCharacteristicValue(String accessoryName, String serviceName, String characteristic, String value) {
  String stringPayload = PAYLOAD_STRING_NAME + accessoryName + PAYLOAD_STRING_SERVICE_NAME + serviceName + "\", \"characteristic\": \"" + characteristic + "\", \"value\": \"" + value + "\"}";
  const char* payload = stringPayload.c_str();
  return mqttClient.publish("homebridge/to/set", payload);    
}

boolean setAccessoryInformation(String accessoryName, String serialNumber) {
  String stringPayload = PAYLOAD_STRING_NAME + accessoryName + "\", \"manufacturer\": \"ElcoSOL\", \"model\": \"IA\", \"serialnumber\": \"" + serialNumber + "\", \"firmwarerevision\": \"1.0.0\"}";
  const char* payload = stringPayload.c_str();
  return mqttClient.publish("homebridge/to/set/accessoryinformation", payload);    
}

boolean setAccessoryReachability(String accessoryName, boolean value) {
  String stringPayload = PAYLOAD_STRING_NAME + accessoryName + "\", \"reachable\": ";
  if (value) {
    stringPayload = stringPayload + F("true");  
  } else {
    stringPayload = stringPayload + F("false");  
  }
  stringPayload = stringPayload + F("}");  
  const char* payload = stringPayload.c_str();
  return mqttClient.publish("homebridge/to/set/reachability", payload);      
}

boolean setBatteryCharacteristics(String accessoryName, String serviceName, byte payloadValue) {
  String value = String(payloadValue);
  setCharacteristicValue(accessoryName, serviceName, F("BatteryLevel"), value);  
  String valueLowBattery = F("0");
  if (payloadValue < LOW_BATTERY_VALUE) {
    valueLowBattery = F("1");  
  }
  setCharacteristicValue(accessoryName, serviceName, F("StatusLowBattery"), valueLowBattery);    
}

/*boolean sendPayload(byte accessoryId, Payload payload) {
  boolean result = false;
  int address = 513 + accessoryId+1;
  int repeaterId = EEPROMReadByte(address); 
  if ((repeaterId>0) && (repeaterId<MAX_COUNT_REPEATERS)) { // TODO change to repeaterId>=0 and in accessory send 255 instead of 0 for payload value in pairing request 
    DEBUG_PRINT(F(" via repeater "));
    DEBUG_PRINT(repeaterId);
    DEBUG_PRINT(F(" "));
    accessoryId = repeaterId;     
    addRepeaterService(&payload); 
  } else {
    DEBUG_PRINT(F("directly (no repeater) "));
  }
  delayBeforeSend(); 
  setControl(&payload);
  radio.stopListening();
  radio.openWritingPipe(accessoryId);
  if (radio.write(&payload, sizeof(Payload))) {
    result = true; 
  }
  radio.startListening();
  return result;  
}

boolean sendPairingResponse(byte accessoryId, int service) {
  Payload payloadPairingResponse = { accessoryId, service, ID_PAIRING, RESULT_OK, 0 };
  return sendPayload(accessoryId, payloadPairingResponse);
}

boolean sendSwitchAction(byte accessoryId, byte action) {
  Payload payloadSwitchAction = { accessoryId, SERVICE_SWITCH, ID_SWITCH_ACTION, action, 0 };
  return sendPayload(accessoryId, payloadSwitchAction);
}

boolean sendOutletAction(byte accessoryId, byte action) {
  Payload payloadOutletAction = { accessoryId, SERVICE_OUTLET, ID_OUTLET_ACTION, action, 0 };
  return sendPayload(accessoryId, payloadOutletAction);
}

boolean sendLightBulbAction(byte accessoryId, byte action) {
  Payload payloadLightBulbAction = { accessoryId, SERVICE_LIGHT_BULB, ID_LIGHT_BULB_ACTION, action, 0 };
  return sendPayload(accessoryId, payloadLightBulbAction);
}

boolean sendWindowCoveringTargetPosition(byte accessoryId, byte targetPosition) {
  Payload payloadWindowCoveringTargetPosition = { accessoryId, SERVICE_WINDOW_COVERING, ID_WINDOW_COVERING_TARGET_POSITION, targetPosition, 0 };
  return sendPayload(accessoryId, payloadWindowCoveringTargetPosition);
}*/

byte getRepeaterId(byte accessoryId) {
  int address = 513 + accessoryId+1;
  int repeaterId = EEPROMReadByte(address); 
  if ((repeaterId>0) && (repeaterId<MAX_COUNT_REPEATERS)) { // TODO change to repeaterId>=0 and in accessory send 255 instead of 0 for payload value in pairing request 
    return repeaterId;       
  } else {
    return accessoryId;
  }  
}

boolean sendPairingResponse(byte accessoryId, int service) {
  boolean result = false;
  delayBeforeSend();
  Payload payload = { accessoryId, service, ID_PAIRING, RESULT_OK, 0 };
  byte repeaterId = getRepeaterId(accessoryId);
  if (repeaterId != accessoryId) {
    DEBUG_PRINT(F(" via repeater "));
    DEBUG_PRINT(repeaterId);
    DEBUG_PRINT(F(" "));
    addRepeaterService(&payload);
   } else {
    DEBUG_PRINT(F("for "));
    DEBUG_PRINT(accessoryId);
    DEBUG_PRINT(F(" directly (no repeater) "));
  }
  setControl(&payload);
  radio.stopListening();
  radio.openWritingPipe(repeaterId);
  if (radio.write(&payload, sizeof(Payload))) {
    result = true; 
  }
  radio.startListening();
  return result;
}

boolean sendSwitchAction(byte accessoryId, byte action) {
  boolean result = false;
  delayBeforeSend();
  Payload payload = { accessoryId, SERVICE_SWITCH, ID_SWITCH_ACTION, action, 0 };
  byte repeaterId = getRepeaterId(accessoryId);
  if (repeaterId != accessoryId) {
    DEBUG_PRINT(F(" via repeater "));
    DEBUG_PRINT(repeaterId);
    DEBUG_PRINT(F(" "));
    addRepeaterService(&payload); 
  } else {
    DEBUG_PRINT(F("directly (no repeater) "));
  }
  setControl(&payload);
  radio.stopListening();
  radio.openWritingPipe(repeaterId);
  if (radio.write(&payload, sizeof(Payload))) {
    result = true; 
  }
  radio.startListening();
  return result;
}

boolean sendOutletAction(byte accessoryId, byte action) {
  boolean result = false;
  delayBeforeSend();
  Payload payload = { accessoryId, SERVICE_OUTLET, ID_OUTLET_ACTION, action, 0 };
  byte repeaterId = getRepeaterId(accessoryId);
  if (repeaterId != accessoryId) {
    DEBUG_PRINT(F(" via repeater "));
    DEBUG_PRINT(repeaterId);
    DEBUG_PRINT(F(" "));
    addRepeaterService(&payload); 
  } else {
    DEBUG_PRINT(F("directly (no repeater) "));
  }
  setControl(&payload);
  radio.stopListening();
  radio.openWritingPipe(repeaterId);
  if (radio.write(&payload, sizeof(Payload))) {
    result = true; 
  }
  radio.startListening();
  return result;
}

boolean sendLightBulbAction(byte accessoryId, byte action) {
  boolean result = false;
  delayBeforeSend();
  Payload payload = { accessoryId, SERVICE_LIGHT_BULB, ID_LIGHT_BULB_ACTION, action, 0 };
  byte repeaterId = getRepeaterId(accessoryId);
  if (repeaterId != accessoryId) {
    DEBUG_PRINT(F(" via repeater "));
    DEBUG_PRINT(repeaterId);
    DEBUG_PRINT(F(" "));
    addRepeaterService(&payload); 
  } else {
    DEBUG_PRINT(F("directly (no repeater) "));
  }
  setControl(&payload);
  radio.stopListening();
  radio.openWritingPipe(repeaterId);
  if (radio.write(&payload, sizeof(Payload))) {
    result = true; 
  }
  radio.startListening();
  return result;
}

boolean sendWindowCoveringTargetPosition(byte accessoryId, byte targetPosition) {
  boolean result = false;
  delayBeforeSend();
  Payload payload = { accessoryId, SERVICE_WINDOW_COVERING, ID_WINDOW_COVERING_TARGET_POSITION, targetPosition, 0 };
  byte repeaterId = getRepeaterId(accessoryId);
  if (repeaterId != accessoryId) {
    DEBUG_PRINT(F(" via repeater "));
    DEBUG_PRINT(repeaterId);
    DEBUG_PRINT(F(" "));
    addRepeaterService(&payload); 
  } else {
    DEBUG_PRINT(F("directly (no repeater) "));
  }
  setControl(&payload);
  radio.stopListening();
  radio.openWritingPipe(repeaterId);
  if (radio.write(&payload, sizeof(Payload))) {
    result = true; 
  }
  radio.startListening();
  return result;
}

boolean sendDoorTargetPosition(byte accessoryId, byte targetPosition) {
  boolean result = false;
  delayBeforeSend();
  Payload payload = { accessoryId, SERVICE_DOOR, ID_DOOR_TARGET_POSITION, targetPosition, 0 };
  byte repeaterId = getRepeaterId(accessoryId);
  if (repeaterId != accessoryId) {
    DEBUG_PRINT(F(" via repeater "));
    DEBUG_PRINT(repeaterId);
    DEBUG_PRINT(F(" "));
    addRepeaterService(&payload); 
  } else {
    DEBUG_PRINT(F("directly (no repeater) "));
  }
  setControl(&payload);
  radio.stopListening();
  radio.openWritingPipe(repeaterId);
  if (radio.write(&payload, sizeof(Payload))) {
    result = true; 
  }
  radio.startListening();
  return result;
}

void delayBeforeSend() {
  unsigned long now = millis();
  unsigned long diff = millisDifference(now, lastReceiveRadioPayload);
  if (diff < DELAY_BEFORE_SEND) {
    DEBUG_PRINT(F("(make delay before send for "));
    DEBUG_PRINT(DELAY_BEFORE_SEND - diff);
    DEBUG_PRINT(F(" milliseconds)"));
    delay(DELAY_BEFORE_SEND - diff);
  } else {
    DEBUG_PRINT(F("(no delay before send needed)"));
  }
  DEBUG_PRINT(F(" "));
}

boolean isServiceSupported(int services, int service) {
  return (services & service) == service;
}

void EEPROMWriteInt(int address, int value) {
  byte l = ((value >> 0) & 0xFF);
  byte h = ((value >> 8) & 0xFF);
  EEPROM.write(address, l);
  EEPROM.write(address+1, h);
}

void EEPROMUpdateInt(int address, int value) {
  byte l = ((value >> 0) & 0xFF);
  byte h = ((value >> 8) & 0xFF);
  EEPROM.update(address, l);
  EEPROM.update(address+1, h);
}

int EEPROMReadInt(int address) {
  byte l = EEPROM.read(address);
  byte h = EEPROM.read(address+1);
  return ((l << 0) & 0xFF) + ((h << 8) & 0xFF00);
}

void EEPROMWriteByte(int address, byte value) {
  EEPROM.write(address, value);
}

void EEPROMUpdateByte(int address, byte value) {
  EEPROM.update(address, value);
}

byte EEPROMReadByte(int address) {
  return EEPROM.read(address);
}

unsigned long millisDifference(unsigned long millis1, unsigned long millis2) {
  if (millis1 >= millis2) {
    return millis1 - millis2;
  } else {
    unsigned long max = 4294967295;
    return max - millis2 + millis1 + 1;
  }
}
