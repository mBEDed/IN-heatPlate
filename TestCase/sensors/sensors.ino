#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// RP2040 Pin Definitions (Update as needed)
#define ANALOG_SENSOR_PIN 26  // Analog temperature sensor pin
#define DIGITAL_SENSOR_1_PIN 4 // First digital temperature sensor
#define DIGITAL_SENSOR_2_PIN 5 // Second digital temperature sensor

// OneWire instances for separate digital sensors
OneWire oneWire1(DIGITAL_SENSOR_1_PIN);
OneWire oneWire2(DIGITAL_SENSOR_2_PIN);

// DallasTemperature objects for each sensor
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Starting Temperature Sensor Test...");

    // Initialize digital sensors
    sensor1.begin();
    sensor2.begin();

    Serial.println("Digital Sensors Initialized.");

    // Check if sensors are detected
    sensor1.requestTemperatures();
    sensor2.requestTemperatures();
    
    float temp1 = sensor1.getTempCByIndex(0);
    float temp2 = sensor2.getTempCByIndex(0);

    if (temp1 == DEVICE_DISCONNECTED_C) {
        Serial.println("Error: First digital sensor not detected!");
    } else {
        Serial.println("First digital sensor detected successfully.");
    }

    if (temp2 == DEVICE_DISCONNECTED_C) {
        Serial.println("Error: Second digital sensor not detected!");
    } else {
        Serial.println("Second digital sensor detected successfully.");
    }
}

void loop() {
    // Read Analog Sensor
    int analog_value = analogRead(ANALOG_SENSOR_PIN);
    float voltage = analog_value * (3.3 / 4095.0);  // RP2040 uses 12-bit ADC (0-4095)
    float analog_temperature = (voltage - 0.5) * 100.0;  // Example conversion, update for your sensor

    Serial.print("Analog Sensor Temp: ");
    Serial.print(analog_temperature);
    Serial.println(" °C");

    // Read Digital Sensors
    sensor1.requestTemperatures();
    sensor2.requestTemperatures();

    float temp1 = sensor1.getTempCByIndex(0);
    float temp2 = sensor2.getTempCByIndex(0);

    Serial.print("Digital Sensor 1 Temp: ");
    Serial.print(temp1);
    Serial.println(" °C");

    Serial.print("Digital Sensor 2 Temp: ");
    Serial.print(temp2);
    Serial.println(" °C");

    if (temp1 == DEVICE_DISCONNECTED_C) {
        Serial.println("Error: Sensor 1 not connected!");
    }

    if (temp2 == DEVICE_DISCONNECTED_C) {
        Serial.println("Error: Sensor 2 not connected!");
    }

    Serial.println("------------------------");
    delay(2000);
}
