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

// EEPROM Addresses
#define EEPROM_MAX_TEMP_INDEX 0
#define EEPROM_BED_RESISTANCE 1
#define EEPROM_PID_KP 2
#define EEPROM_PID_KI 6
#define EEPROM_PID_KD 10

// PID values
float kI = 0.2;
float kD = 0.25;
float kP = 8.0;
float I_clip = 220;
float error_I = 0;

// Function Prototypes
void setupSensors();
void showLogo();
void mainMenu();
void doSetup();
void getResistanceFromUser ();
void cancelledPB();
void cancelledTimer();
void coolDown();
void completed();
float getTemp();
float getVolts();
void stepPID(float target_temp, float current_temp, float last_temp, float dt, int min_pwm);
bool heat(byte max_temp, int profile_index);
void showHeatMenu(byte max_temp);
void clearMainMenu();
void showMainMenuLeft(int &x, int &y);
void showMainMenuRight();
uint8_t getProfile();
void displayProfileRight(int8_t cur_profile);
buttons_state_t getButtonsState();
void setResistance(float resistance);
float getResistance();
void setMaxTempIndex(int index);
int getMaxTempIndex(void);
bool isFirstBoot();
void setFirstBoot();
void saveSettings();
void loadSettings();
void calibrateSensor();
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

// Function to setup sensors
void setupSensors() {
    sensors.begin();
    sensor_count = sensors.getDeviceCount();
    debugprint("Number of sensors: ");
    debugprintln(sensor_count);
    for (int i = 0; i < sensor_count; i++) {
        sensors.getAddress(temp_addresses[i], i);
    }
}

// Button interrupt handlers
void dnsw_change_isr() {
    dn_button_state = BUTTON_PRESSED;
    down_state_change_time = millis();
}

void upsw_change_isr() {
    up_button_state = BUTTON_PRESSED;
    up_state_change_time = millis();
}

// Show logo on startup
void showLogo() {
    unsigned long start_time = millis();
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    while (start_time + 2000 > millis()) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        // Assuming logo is defined elsewhere
        display.drawBitmap(0, 0, logo, logo_width, logo_height, SSD1306_WHITE);
        display.setCursor(80, 16);
        display.print(F("S/W V"));
        display.print(sw, 1);
        display.setCursor(80, 24);
        display.print(F("H/W V"));
        display.print(hw, 1);
        display.display();
        buttons_state_t cur_button = getButtonsState();
        if (cur_button == BUTTONS_BOTH_PRESS) {
            doSetup();
            return;
        }
    }
}

// Perform setup
inline void doSetup() {
    debugprintln("Performing setup");
    getResistanceFromUser ();
    calibrateSensor();
    setFirstBoot();
}

// Get resistance from user
inline void getResistanceFromUser () {
    float resistance = 1.88;
    while (1) {
        clearMainMenu();
        display.setCursor(3, 4);
        display.print(F("Resistance"));
        display.drawLine(3, 12, 79, 12, SSD1306_WHITE);
        display.setCursor(3, 14);
        display.print(F("UP/DN: change"));
        display.setCursor(3, 22);
        display.print(F("BOTH: choose"));
        buttons_state_t button = getButtonsState();
        if (button == BUTTONS_UP_PRESS) {
            resistance += 0.01;
        } else if (button == BUTTONS_DN_PRESS) {
            resistance -= 0.01;
        } else if (button == BUTTONS_BOTH_PRESS) {
            setResistance(resistance);
            saveSettings(); // Save resistance to EEPROM
            return;
        }
        resistance = constrain(resistance, 0, MAX_RESISTANCE);

        display.setCursor(90, 12);
        display.print(resistance);
        display.display();
    }
}

// Main menu logic
inline void mainMenu() {
    menu_state_t cur_state = MENU_IDLE;
    int x = 0;   // Display change counter
    int y = 200; // Display change max (modulus used below)
    uint8_t profile_index = 0;

    while (1) {
        switch (cur_state) {
        case MENU_IDLE: {
            clearMainMenu();
            buttons_state_t cur_button = getButtonsState();

            if (cur_button == BUTTONS_BOTH_PRESS) {
                cur_state = MENU_SELECT_PROFILE;
            } else if (cur_button == BUTTONS_UP_PRESS) {
                cur_state = MENU_INC_TEMP;
            } else if (cur_button == BUTTONS_DN_PRESS) {
                cur_state = MENU_DEC_TEMP;
            }
        } break;
        case MENU_SELECT_PROFILE: {
            debugprintln("getting thermal profile");
            profile_index = getProfile();
            cur_state = MENU_HEAT;
        } break;
        case MENU_HEAT: {
            if (!heat(max_temp_array[max_temp_index], profile_index)) {
                cancelledPB();
                coolDown();
            } else {
                coolDown();
                completed();
            }
            cur_state = MENU_IDLE;
        } break;
        case MENU_INC_TEMP: {
            if (max_temp_index < sizeof(max_temp_array) - 1) {
                max_temp_index++;
                debugprintln("incrementing max temp");
                setMaxTempIndex(max_temp_index);
                saveSettings(); // Save max temp index to EEPROM
            }
            cur_state = MENU_IDLE;
        } break;
        case MENU_DEC_TEMP: {
            if (max_temp_index > 0) {
                max_temp_index--;
                debugprintln("decrementing max temp");
                setMaxTempIndex(max_temp_index);
                saveSettings(); // Save max temp index to EEPROM
            }
            cur_state = MENU_IDLE;
        } break;
        }

        // Change Display (left-side)
        showMainMenuLeft(x, y);

        // Update Display (right-side)
        showMainMenuRight();
    }
}

// Function to save settings to EEPROM
void saveSettings() {
    EEPROM.write(EEPROM_MAX_TEMP_INDEX, max_temp_index);
    EEPROM.put(EEPROM_BED_RESISTANCE, bed_resistance);
    EEPROM.put(EEPROM_PID_KP, kP);
    EEPROM.put(EEPROM_PID_KI, kI);
    EEPROM.put(EEPROM_PID_KD, kD);
}

// Function to load settings from EEPROM
void loadSettings() {
    max_temp_index = EEPROM.read(EEPROM_MAX_TEMP_INDEX);
    EEPROM.get(EEPROM_BED_RESISTANCE, bed_resistance);
    EEPROM.get(EEPROM_PID_KP, kP);
    EEPROM.get(EEPROM_PID_KI, kI);
    EEPROM.get(EEPROM_PID_KD, kD);
}

// Function to calibrate the temperature sensor
void calibrateSensor() {
    // Placeholder for calibration logic
    // You can implement a routine to compare with a known accurate thermometer
    debugprintln("Calibrating sensor...");
}

// Function to control the LED based on heating state
void controlLED(bool heating) {
    if (heating) {
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(LED_RED_PIN, HIGH);
    } else {
        digitalWrite(LED_GREEN_PIN, HIGH);
        digitalWrite(LED_RED_PIN, LOW);
    }
}

// Function to get the current temperature
float getTemp() {
    debugprint("Temps: ");
    float t = 0;
    for (byte i = 0; i < 100; i++) { // Poll TEMP_PIN reading 100 times
        t += analogRead(TEMP_PIN);
    }
    t /= 100.0; // average
    t *= VOLTAGE_REFERENCE / 1024.0; // voltage
    t = (t - 1.365) / ((.301 - 1.365) / (150.0 - 25.0)) + 25.0;

    // Estimate true bed temperature based on thermal gradient
    float estimated_temp = t * ANALOG_APPROXIMATION_SCALAR + ANALOG_APPROXIMATION_OFFSET;
    debugprint(estimated_temp);
    debugprint(" ");

    sensors.requestTemperatures();
    for (int i = 0; i < sensor_count; i++) {
        float temp_in = sensors.getTempC(temp_addresses[i]);
        debugprint(temp_in);
        debugprint(" ");
    }
    debugprintln();

    return max(t, estimated_temp);
}

// Function to get the current voltage
float getVolts() {
    float v = 0;
    for (byte i = 0; i < 20; i++) { // Poll Voltage reading 20 times
        v += analogRead(VCC_PIN);
    }
    v /= 20;

    float vin = (v / 1023.0) * 1.5;
    debugprint("voltage at term: ");
    debugprintln(vin);
    vin = (vin / 0.090981) + 0.3;
    return vin;
}

// Function to heat the soldering plate
bool heat(byte max_temp, int profile_index) {
    showHeatMenu(max_temp);
    delay(3000);

    float t; // Used to store current temperature
    float v; // Used to store current voltage

    unsigned long profile_max_time = millis() / 1000 + (8 * 60);
    unsigned long step_start_time = (millis() / 1000);
    int current_step = 0;

    int x = 0;  // Heat Animate Counter
    int y = 80; // Heat Animate max (modulus used below)

    float start_temp = getTemp();
    float goal_temp = profiles[profile_index].fraction[0] * max_temp;
    float step_runtime = profiles[profile_index].seconds[0];
    float last_time = 0;
    float last_temp = getTemp();
    error_I = 0;

    controlLED(true); // Turn on heating LED

    while (1) {
        if (getButtonsState() != BUTTONS_NO_PRESS) {
            analogWrite(MOSFET_PIN, 0);
            controlLED(false); // Turn off heating LED
            debugprintln("cancelled");
            return 0;
        }

        if (millis() / 1000 > profile_max_time) {
            analogWrite(MOSFET_PIN, 0);
            controlLED(false); // Turn off heating LED
            debugprintln("exceeded time");
            cancelledTimer();
            return 0;
        }

        t = getTemp();
        v = getVolts();
        float max_possible_amperage = v / bed_resistance;
        float vmax = (MAX_AMPERAGE * bed_resistance) * PWM_VOLTAGE_SCALAR;
        int min_PWM = 255 - ((vmax * 255.0) / v);
        min_PWM = constrain(min_PWM, 0, 255);
        debugprint("Min PWM: ");
        debugprintln(min_PWM);
        debugprintln(bed_resistance);

        float time_into_step = ((float)millis() / 1000.0) - (float)step_start_time;
        float target_temp = min(((goal_temp - start_temp) * (time_into_step / step_runtime)) + start_temp, goal_temp);

        stepPID(target_temp, t, last_temp, time_into_step - last_time, min_PWM);
        last_time = time_into_step;

        if (time_into_step >= step_runtime) {
            if (abs(t - goal_temp) < 2.5) {
                current_step++;
                if (current_step == profiles[profile_index].points) {
                    analogWrite(MOSFET_PIN, 0);
                    controlLED(false); // Turn off heating LED
                    return 1;
                }
                last_time = 0.0;
                start_temp = t;
                goal_temp = profiles[profile_index].fraction[current_step] * max_temp;
                step_runtime = profiles[profile_index].seconds[current_step] - profiles[profile_index].seconds[current_step - 1];
                step_start_time = millis() / 1000.0;
            }
        }

        heatAnimate(x, y, v, t, target_temp);
    }
}

// PID control function
void stepPID(float target_temp, float current_temp, float last_temp, float dt, int min_pwm) {
    float error = target_temp - current_temp;
    float D = (current_temp - last_temp) / dt;

    error_I += error * dt * kI;
    error_I = constrain(error_I, 0, I_clip);

    float PWM = 255.0 - (error * kP + D * kD + error_I);
    PWM = constrain(PWM, min_pwm, 255);

    debugprintln("PID");
    debugprintln(dt);
    debugprintln(error);
    debugprintln(error_I);
    debugprint("PWM: ");
    debugprintln(PWM);
    analogWrite(MOSFET_PIN, (int)PWM);
}

// Function to animate heating display
void inline heatAnimate(int &x, int &y, float v, float t, float target) {
    display.clearDisplay();
    display.drawBitmap(0, 3, heat_animate, heat_animate_width, heat_animate_height, SSD1306_WHITE);
    display.drawBitmap(112, 3, heat_animate, heat_animate_width, heat_animate_height, SSD1306_WHITE);
    display.fillRect(0, 3, heat_animate_width, heat_animate_height * (y - x) / y, SSD1306_BLACK);
    display.fillRect(112, 3, heat_animate_width, heat_animate_height * (y - x) / y, SSD1306_BLACK);
    x = (x + 1) % y; // Heat animate increment and modulus

    display.setTextSize(2);
    display.setCursor(22, 4);
    display.print(F("HEATING"));
    display.setTextSize(1);
    display.setCursor(20, 24);
    display.print(F("~"));
    display.print(v, 1);
    display.print(F("V"));
    if (t >= 100) {
        display.setCursor(63, 24);
    } else if (t >= 10) {
        display.setCursor(66, 24);
    } else {
        display.setCursor(69, 24);
    }
    display.print(F("~"));
    display.print(t, 0);
    display.print(F("C"));
    display.print(F("/"));
    display.print(target, 0);
    display.print(F("C"));
    display.display();
}

// Function to handle cancellation
void cancelledPB() {
    display.clearDisplay();
    display.drawRoundRect(22, 0, 84, 32, 2, SSD1306_WHITE);
    display.setCursor(25, 4);
    display.print(F("  CANCELLED"));
    display.display();
    delay(2000);
}

// Function to handle timeout
void cancelledTimer() {
    int x = 0;   // Display change counter
    int y = 150; // Display change max (modulus used below)

    while (getButtonsState() == BUTTONS_NO_PRESS) {
        display.clearDisplay();
        display.drawRoundRect(22, 0, 84, 32, 2, SSD1306_WHITE);
        display.setCursor(25, 4);
        display.print(F("  TIMED OUT"));
        display.drawLine(25, 12, 103, 12, SSD1306_WHITE);

        if (x < (y * 0.3)) {
            display.setCursor(25, 14);
            display.println(" Took longer");
            display.setCursor(25, 22);
            display.println(" than 5 mins");
        } else if (x < (y * 0.6)) {
            display.setCursor(28, 14);
            display.println("Try a higher");
            display.setCursor(25, 22);
            display.println(" current PSU");
        } else {
            display.setCursor(25, 14);
            display.println(" Push button");
            display.setCursor(25, 22);
            display.println("  to return");
        }
        x = (x + 1) % y; // Display change increment and modulus

        display.setTextSize(3);
        display.setCursor(5, 4);
        display.print(F("!"));
        display.setTextSize(3);
        display.setCursor(108, 4);
        display.print(F("!"));
        display.setTextSize(1);
        display.display();
        delay(50);
    }
}

// Function to cool down
void coolDown() {
    float t = getTemp(); // Used to store current temperature

    while (getButtonsState() == BUTTONS_NO_PRESS && t > 45.00) {
        display.clearDisplay();
        display.drawRoundRect(22, 0, 84, 32, 2, SSD1306_WHITE);
        display.setCursor(25, 4);
        display.print(F("  COOL DOWN"));
        display.drawLine(25, 12, 103, 12, SSD1306_WHITE);
        display.setCursor(25, 14);
        display.println("  Still Hot");
        t = getTemp();
        if (t >= 100) {
            display.setCursor(49, 22);
        } else {
            display.setCursor(52, 22);
        }
        display.print(F("~"));
        display.print(t, 0);
        display.print(F("C"));
        display.setTextSize(3);
        display.setCursor(5, 4);
        display.print(F("!"));
        display.setTextSize(3);
        display.setCursor(108, 4);
        display.print(F("!"));
        display.setTextSize(1);
        display.display();
    }
}

// Function to indicate completion
void completed() {
    display.clearDisplay();
    display.drawRoundRect(22, 0, 84, 32, 2, SSD1306_WHITE);
    display.setCursor(25, 4);
    display.print(F("  COMPLETED  "));
    display.drawLine(25, 12, 103, 12, SSD1306_WHITE);
    display.setCursor(25, 14);
    display.println(" Push button");
    display.setCursor(25, 22);
    display.println("  to return");
    display.drawBitmap(0, 9, tick, tick_width, tick_height, SSD1306_WHITE);
    display.drawBitmap(112, 9, tick, tick_width, tick_height, SSD1306_WHITE);
    display.display();

    while (getButtonsState() == BUTTONS_NO_PRESS) {
    }
}

// Function to get the profile
inline uint8_t getProfile() {
    uint8_t cur_profile = 0;
    while (1) {
        clearMainMenu();
        display.setCursor(3, 4);
        display.print(F("Pick profile"));
        display.drawLine(3, 12, 79, 12, SSD1306_WHITE);
        display.setCursor(3, 14);
        display.print(F(" UP/DN: cycle"));
        display.setCursor(3, 22);
        display.print(F(" BOTH: choose"));
        buttons_state_t cur_button = getButtonsState();
        if (cur_button == BUTTONS_BOTH_PRESS) {
            clearMainMenu();
            return cur_profile;
        } else if (cur_button == BUTTONS_DN_PRESS) {
            cur_profile--;
        } else if (cur_button == BUTTONS_UP_PRESS) {
            cur_profile++;
        }
        cur_profile %= NUM_PROFILES;
        displayProfileRight(cur_profile);
        display.display();
    }
}

// Function to display the profile
inline void displayProfileRight(int8_t cur_profile) {
    int cur_x = 90;
    int cur_y = 30;
    float x_dist = SCREEN_WIDTH - 90 - 8;
    display.setCursor(cur_x, cur_y);
    float total_seconds = (int)profiles[cur_profile].seconds[profiles[cur_profile].points - 1];

    for (int i = 0; i < profiles[cur_profile].points; i++) {
        int x_next = (int)((profiles[cur_profile].seconds[i] / total_seconds) * x_dist) + 90;
        int y_next = 30 - (int)(profiles[cur_profile].fraction[i] * 28.0);
        display.drawLine(cur_x, cur_y, x_next, y_next, SSD1306_WHITE);
        cur_x = x_next;
        cur_y = y_next;
    }
    display.drawLine(cur_x, cur_y, SCREEN_WIDTH - 2, 30, SSD1306_WHITE);
}

// Function to clear the main menu
inline void clearMainMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.drawRoundRect(0, 0, 83, 32, 2, SSD1306_WHITE);
}

// Function to show the left side of the main menu
inline void showMainMenuLeft(int &x, int &y) {
    if (x < (y * 0.5)) {
        display.setCursor(3, 4);
        display.print(F("PRESS BUTTONS"));
        display.drawLine(3, 12, 79, 12, SSD1306_WHITE);
        display.setCursor(3, 14);
        display.print(F(" Change  MAX"));
        display.setCursor(3, 22);
        display.print(F(" Temperature"));
    } else {
        display.setCursor(3, 4);
        display.print(F("HOLD  BUTTONS"));
        display.drawLine(3, 12, 79, 12, SSD1306_WHITE);
        display.setCursor(3, 18);
        display.print(F("Begin Heating"));
    }
    x = (x + 1) % y; // Display change increment and modulus
}

// Function to show the right side of the main menu
inline void showMainMenuRight() {
    display.setCursor(95, 6);
    display.print(F("TEMP"));
    display.setCursor(95, 18);
    display.print(max_temp_array[max_temp_index]);
    display.print(F("C"));
    display.display();
}

// Function to show the heating menu
inline void showHeatMenu(byte max_temp) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(22, 4);
    display.print(F("HEATING"));
    display.setTextSize(1);
    display.setCursor(52, 24);
    display.print(max_temp);
    display.print(F("C"));
    display.display();
}

// Function to get the button state
#define BUTTON_PRESS_TIME 50
buttons_state_t getButtonsState() {
    single_button_state_t button_dn;
    single_button_state_t button_up;
    unsigned long button_dn_time;
    unsigned long button_up_time;

    noInterrupts();
    button_dn = dn_button_state;
    button_up = up_button_state;
    button_dn_time = down_state_change_time;
    button_up_time = up_state_change_time;
    interrupts();

    unsigned long cur_time = millis();
    buttons_state_t state = BUTTONS_NO_PRESS;

    if (button_dn == BUTTON_PRESSED && button_up == BUTTON_PRESSED &&
        abs(button_dn_time - button_up_time) < BUTTON_PRESS_TIME) {
        if (cur_time - button_dn_time > BUTTON_PRESS_TIME &&
            cur_time - button_up_time > BUTTON_PRESS_TIME) {
            state = BUTTONS_BOTH_PRESS;
            noInterrupts();
            dn_button_state = BUTTON_NO_ACTION;
            up_button_state = BUTTON_NO_ACTION;
            interrupts();
        }
    } else if (button_up == BUTTON_PRESSED && cur_time - button_up_time > BUTTON_PRESS_TIME) {
        state = BUTTONS_UP_PRESS;
        noInterrupts();
        up_button_state = BUTTON_NO_ACTION;
        interrupts();
    } else if (button_dn == BUTTON_PRESSED && cur_time - button_dn_time > BUTTON_PRESS_TIME) {
        state = BUTTONS_DN_PRESS;
        noInterrupts();
        dn_button_state = BUTTON_NO_ACTION;
        interrupts();
    }

    return state;
}

// Main loop
void loop() {
    // Not used
}