#include <DallasTemperature.h>
#include <OneWire.h>

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