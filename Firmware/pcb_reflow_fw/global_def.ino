#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <SPI.h>

// Version Definitions
static const PROGMEM float hw = 0.9;
static const PROGMEM float sw = 0.15;

// Screen Definitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C // I2C Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Create Display

// Pin Definitions
#define MOSFET_PIN PIN_PC3
#define UPSW_PIN PIN_PF3
#define DNSW_PIN PIN_PD4
#define TEMP_PIN PIN_PF2 // A2
#define VCC_PIN PIN_PF4  // A0
#define LED_GREEN_PIN PIN_PC5
#define LED_RED_PIN PIN_PC4
#define ONE_WIRE_BUS PIN_PD5

#define MOSFET_PIN_OFF 255

// Temperature Info
byte max_temp_array[] = {140, 150, 160, 170, 180};
byte max_temp_index = 0;
#define MAX_RESISTANCE 10.0
float bed_resistance = 1.88;
#define MAX_AMPERAGE 5.0
#define PWM_VOLTAGE_SCALAR 2.0

// EEPROM storage locations
#define CRC_ADDR 0
#define FIRSTTIME_BOOT_ADDR 4
#define TEMP_INDEX_ADDR 5
#define RESISTANCE_INDEX_ADDR 6
#define DIGITAL_TEMP_ID_ADDR 10

// Voltage Measurement Info
#define VOLTAGE_REFERENCE 1.5