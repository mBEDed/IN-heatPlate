inline void setupSensors() {
    sensors.begin();
    sensor_count = sensors.getDeviceCount();
    debugprint("Looking for sensors, found: ");
    debugprintln(sensor_count);
    for (int i = 0; i < min(sensor_count, sizeof(temp_addresses)); i++) {
        sensors.getAddress(temp_addresses[i], i);
    }
}