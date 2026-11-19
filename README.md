# Smart Home on Arduino Mega, ATtiny84, nRF24L01 & Homebridge (MQTT)

A DIY retro smart home project with **Apple HomeKit** integration (built roughly 8 years ago, before modern ESP32/Matter ecosystems became mainstream). The system connects a two-story house using **Arduino Mega + Ethernet Shield + nRF24L01** controllers, a **Raspberry Pi** running **Homebridge** with **MQTT**, a fleet of **ATtiny84** nodes on homemade etched PCBs (with all copper-etch PDFs included in the repository), plus **ATtiny85** helper microcontrollers for I2C air-quality sensors and smooth servo easing.

> **Note:** An experimental XBee / ATtiny841 branch also exists in the codebase and achieved a noticeably better RF link on breadboard prototypes, but it never made it to custom PCBs and is left aside here.

## 1. Repository Structure

The repository preserves the original project directory layout inside the `trunk/` directory:

```text
trunk/
├── accessoryController/        # Floor controller firmware
│   └── ino/accessoryController/
├── attiny84/                   # Main wireless accessory node firmware
│   └── accessory/
│       └── ino/accessory/
├── attiny85/                   # Dedicated helper MCU sketches
│   ├── i2cMHZ/                 # CO2 sensor (MH-Z19) UART-to-I2C slave bridge
│   ├── i2cPMS/                 # Particulate matter (PMS5003) UART-to-I2C slave bridge
│   └── servo/                  # Standalone ServoEaser controller
├── common/
│   └── IA/                     # Shared protocol header (IA.h)
└── pdf/                        # Ready-to-print copper-etch PDFs & multi-board panels
    ├── 1_IAC-01_3_ACDC-01/
    ├── 4_IAC-01/
    ├── 6_ACDC-01/
    └── 7_IA-03/
```

## 2. Architecture

The workload is split so wired/Linux components handle networking, JSON payloads, and HomeKit state management, while wireless nodes stay lightweight and power-conscious:

* **Accessory Nodes (`ATtiny84` + `nRF24L01`):** read sensors, drive relays or window coverings, sleep in `SLEEP_MODE_PWR_DOWN`, and wake up via Watchdog Timer (`WDT`) or Pin Change Interrupts (`PCINT0` / `PCINT1`).
* **Helper Microcontrollers (`ATtiny85`):** offload UART-based air-quality sensors (`TinyWireSlave` bridges over I2C) and smooth servo motion (`ServoEaser`) from the main wireless node loop.
* **Floor Controllers (`Arduino Mega 2560` + `Ethernet Shield` + custom `nRF24L01` shield):**
  * **1st floor (`IAController`):** `CONTROLLER_RADIO_ID = 255`, static IP `192.168.178.210`, RF channel `76`.
  * **2nd floor (`IAController2`):** `CONTROLLER_RADIO_ID = 254`, static IP `192.168.178.209`, RF channel `83`.
  * Store paired services, repeater routes, and reachability timers in EEPROM.
* **HomeKit Bridge (`Raspberry Pi` + `Homebridge` + `MQTT`):** controllers connect over Ethernet to an MQTT broker (`192.168.178.160:1883`) and dynamically register, update, or remove accessories and services via `homebridge/to/...` while listening to `homebridge/from/set`.

## 3. Libraries & Dependencies

### Shared Protocol

* **`IA.h` (`trunk/common/IA/`):** custom shared header defining the binary `Payload` structure, service bitmasks, message/characteristic IDs, scaling constants, and I2C slave addresses.

### Floor Controllers (`Arduino Mega 2560`)

* **`RF24` (`SPI.h`, `nRF24L01.h`, `RF24.h`):** wireless communication with accessory nodes and repeaters.
* **`Ethernet` (`Ethernet.h`):** W5100/W5500 Ethernet Shield networking.
* **`PubSubClient` (`PubSubClient.h`):** MQTT client communicating with the Homebridge MQTT broker.
* **`EEPROM` (`EEPROM.h`):** persistent storage for paired accessory service masks and repeater routing tables.

### Main Accessory Nodes (`ATtiny84`)

* **`ATTinyCore`:** Arduino core for `ATtiny24/44/84` (`Chip = ATtiny84`, `Pin Mapping = Counterclockwise`, `LTO = enabled`, programmed via `Arduino as ISP` at `8 MHz`, or `4 MHz` for `DHT11`).
* **`RF24` (`SPI.h`, `nRF24L01.h`, `RF24.h`):** nRF24L01 transceiver driver.
* **`SoftI2CMaster` (`SoftI2CMaster.h`):** software I2C master used to poll the `ATtiny85` air-quality/CO2 slave bridges.
* **`DHT` (`DHT.h`):** digital temperature and humidity sensor support (`DHT11` / `DHT22`).
* **`EEPROM` (`EEPROM.h`):** persistent node configuration/state storage.
* **AVR Standard Libraries (`avr/sleep.h`, `avr/wdt.h`, `avr/interrupt.h`, `avr/power.h`):** deep sleep (`SLEEP_MODE_PWR_DOWN`), Watchdog wake-ups, pin-change interrupts, and peripheral power reduction.

### Helper Nodes (`ATtiny85`)

* **`TinyWireSlave` (`TinyWireSlave.h`):** turns `ATtiny85` into an I2C slave (`0x4` for PMS5003, `0x5` for MH-Z19) responding to multi-byte register reads.
* **`SoftwareSerial` (`SoftwareSerial.h`):** UART communication (`9600 bps`) with `MH-Z19` CO2 and `PMS5003` particulate matter sensors.
* **`Servo` & `ServoEaser` (`Servo.h`, `ServoEaser.h`):** smooth eased servo positioning on the standalone door/actuator helper node (`servo.ino`), paired with `EEPROM.h`.

## 4. Hardware & Custom PCBs

* **Controller Stack:** Arduino Mega 2560 topped with an Ethernet Shield and a custom `nRF24L01` board. Using a short, well-shielded USB cable is recommended when flashing/powering with an antenna-equipped radio module.
* **Radio Module Quality Note:** inexpensive `nRF24L01` clones caused intermittent packet drops and unstable link behavior across PA levels; switching to genuine shielded ("green with shell") modules resolved the RF stability issues completely.
* **Homemade `ATtiny84` PCBs:** single-sided custom PCBs designed for DIY etching (`10nF` decoupling capacitors, `C2`, transistor/resistor stage `Q3`/`R2`, and peripheral headers).
* **Etching PDFs Included:** all copper-etch PDF templates (`*_etch_copper_bottom.pdf` and multi-board gang panels across `IAC-01`, `ACDC-01`, and `IA-03` layouts) are included in the `trunk/pdf/` directory.

## 5. Binary Protocol (`IA.h`) & 1-Byte Encoding

`IA.h` defines a compact fixed-size packet:

```cpp
struct Payload { byte accessoryId; int service; byte id; byte value; byte control; };
```

* **`accessoryId`** — node ID (`MAX_COUNT_ACCESSORIES = 200` on the controller).
* **`service`** — bitmask combining supported services (`|`): motion, switch, temperature, battery level, light, outlet, leak, lightbulb, window covering, contact 1, contact 2, repeater, air quality, humidity, and door.
* **`id` & `value`** — message/characteristic identifier and a single `byte` (`0..255`) payload, paired with a simple additive checksum `control` (`accessoryId + service + id + value`).

To fit fractional and wide-range readings into one byte:

* **Temperature:** encoded as `(temp + 50.0) * 2.0` (`0.5 °C` resolution, avoiding negative bytes) and decoded on the controller as `(value / 2.0) - 50.0`.
* **Humidity:** encoded as `humidity * 2.0` (`0.5%` steps) and divided by `2.0` on the controller.
* **Illuminance (lux) & CO2 (ppm):** scaled by `LOW` / `MEDIUM` / `HIGH` multipliers (`1.0 / 10.0 / 100.0` for lux; `10.0 / 20.0 / 40.0` for CO2) using dedicated message IDs.
* **Switch actions (`ID_SWITCH_ACTION`):** supports `0`/`1` as well as `2` (toggle stored state) and `3` (resend stored state).

## 6. Supported Services, Helper Nodes & Node Types

Based on `IA.h`, the controller sketch, the per-node configurations (`ACCESSORY_ID` 1–27 plus test node 112), and the helper sketches, the codebase supports:

* **Relays (`SERVICE_OUTLET`, `SERVICE_LIGHT_BULB`):** outlet and lightbulb control (`On`) with configurable active-level inversion (`reverseRelayValue`).
* **Switches / Buttons (`SERVICE_SWITCH`):** interrupt-driven (`PCINT1`) state/action reporting (`On`).
* **Motion & Light (`SERVICE_MOTION`, `SERVICE_LIGHT`):** PIR motion sensor plus an LDR light sensor (with optional `PIN_POWER` switching to save power); exposes `MotionDetected` (`MIA`), an optional darkness motion service (`MIA D`), and `CurrentAmbientLightLevel` (`LIA`).
* **Temperature & Humidity (`SERVICE_TEMPERATURE`, `SERVICE_HUMIDITY`):** analog temperature reading (using `INTERNAL` reference + calibration offsets) or `DHT11`/`DHT22` (`TIA`, `HIA`).
* **Air Quality, CO2 & I2C Bridges (`SERVICE_AIR_QUALITY`):** `MQ-135` air quality buckets (1–5) plus dedicated `ATtiny85` `TinyWireSlave` I2C helper nodes that read UART sensors via `SoftwareSerial` and serve values on request:
  * **CO2 I2C slave (`trunk/attiny85/i2cMHZ`, address `0x5`):** polls `0xFF 0x01 0x86...` over `SoftwareSerial` every 5 seconds, verifies the checksum, and returns 16-bit `PPM_CO2` in two I2C bytes.
  * **Particulate matter I2C slave (`trunk/attiny85/i2cPMS`, address `0x4`):** reads 32-byte `0x42`-framed payloads over `SoftwareSerial`, validates the checksum, and serves `PM_10_STANDARD`, `PM_25_STANDARD`, `PM_100_STANDARD`, and `*_ENV` values over I2C.
* **Leak & Contact Sensors (`SERVICE_LEAK`, `SERVICE_1_CONTACT`, `SERVICE_2_CONTACT`):** interrupt-woken leak (`LKIA`) and single/dual contact sensors (`CIA 1`, `CIA 2`) with multi-send retries (`SendCount = 3`).
* **Window Coverings (`SERVICE_WINDOW_COVERING`):** step calculation from motor `RPM`, shaft diameter, and travel length (`mm`), dual-pin motor control (`IN1`/`IN2`), limit interrupts, and manual switch handling (`WCIA`).
* **Battery Reporting (`SERVICE_BATTERY_LEVEL`):** reports `BatteryLevel` (%) and `StatusLowBattery` (below `15%`).
* **Repeaters (`SERVICE_REPEATER`):** forwards packets between distant nodes and the controller using routes persisted in EEPROM.
* **Door & Standalone Servo Easer (`SERVICE_DOOR`, ID 112 & `trunk/attiny85/servo`):** test door logic with a servo and two ultrasonic sensors (noting `Servo`/`SoftwareServo` and interrupt conflicts with the radio), alongside a standalone `ATtiny85` helper (`trunk/attiny85/servo`) driven by a control pin (`0` / `90` deg with `DOOR_SERVO_DEGREES_CORRECTION = 7` and `MSEC_90_DEGREE = 3500`), `ServoEaser`, EEPROM angle persistence, and open/closed LEDs.

## 7. Homebridge Lifecycle & Power Notes

* **Dynamic Homebridge Sync:** on `ID_PAIRING`, the controller checks whether an accessory's service mask changed—removing and recreating it if needed (`homebridge/to/remove`, `homebridge/to/add`, `homebridge/to/add/service`), setting accessory info, and pushing characteristic updates.
* **Reachability Tracking:** registered accessories are tracked with a periodic timer (`TIMER_REACHABILITY` / `TIMEOUT_REACHABILITY`) and marked unreachable in `homebridge/to/set/reachability` when silent, resetting on incoming data or `ID_REACHABILITY_PING`.
* **Radio Listening Trade-off:** because `nRF24L01` draws roughly `14 mA` in receive/listening mode, battery-powered sensor nodes power down the radio between transmissions (except during pairing), keeping listening enabled only for mains/active nodes like relays, window coverings, and repeaters.