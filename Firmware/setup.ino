#include <Adafruit_GFX.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>

// Version Definitions
static const float hw = 0.9;
static const float sw = 0.15;

// Screen Definitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C // I2C Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Create Display

// Pin Definitions
#define MOSFET_PIN 3
#define UPSW_PIN 4
#define DNSW_PIN 5
#define TEMP_PIN 26
#define VCC_PIN 27
#define LED_GREEN_PIN 6
#define LED_RED_PIN 7
#define ONE_WIRE_BUS 8

// Button states
enum single_button_state_t { BUTTON_PRESSED, BUTTON_RELEASED, BUTTON_NO_ACTION };

// Button interrupt state
volatile single_button_state_t up_button_state = BUTTON_NO_ACTION;
volatile single_button_state_t dn_button_state = BUTTON_NO_ACTION;
volatile unsigned long up_state_change_time = 0;
volatile unsigned long down_state_change_time = 0;

// Temperature Info
byte max_temp_array[] = {140, 150, 160, 170, 180};
byte max_temp_index = 0;
#define MAX_RESISTANCE 10.0
float bed_resistance = 1.88;

// Optional temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int sensor_count = 0;
DeviceAddress temp_addresses[3];

// Debugging
#define DEBUG
#ifdef DEBUG
#define debugprint(x) Serial.print(x);
#define debugprintln(x) Serial.println(x);
#else
#define debugprint(x)
#define debugprintln(x)
#endif

// Function Prototypes
void setupSensors();
void showLogo();
void mainMenu();
void doSetup();
void getResistanceFromUser  ();
void controlLED(bool heating);

// Setup function
void setup() {
    // Pin Direction control
    pinMode(MOSFET_PIN, OUTPUT);
    pinMode(UPSW_PIN, INPUT);
    pinMode(DNSW_PIN, INPUT);
    pinMode(TEMP_PIN, INPUT);
    pinMode(VCC_PIN, INPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);

    digitalWrite(LED_GREEN_PIN, HIGH);
    analogWrite(MOSFET_PIN, 255); // Set MOSFET to full power

    attachInterrupt(DNSW_PIN, dnsw_change_isr, FALLING);
    attachInterrupt(UPSW_PIN, upsw_change_isr, FALLING);

    Serial.begin(9600);

    // Start-up Display
    debugprintln("Showing startup");
    showLogo();

    debugprintln("Checking sensors");
    setupSensors();

    debugprintln("Checking first boot");
    if (isFirstBoot()) {
        doSetup();
    } else {
        loadSettings();
    }

    debugprintln("Entering main menu");
    mainMenu();
}