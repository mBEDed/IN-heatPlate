#include <EEPROM.h>

// EEPROM Addresses
#define EEPROM_MAX_TEMP_INDEX 0
#define EEPROM_BED_RESISTANCE 1
#define EEPROM_PID_KP 2
#define EEPROM_PID_KI 6
#define EEPROM_PID_KD 10

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