void setup() {
    // Pin Direction control
    pinMode(MOSFET_PIN, OUTPUT);
    pinMode(UPSW_PIN, INPUT);
    pinMode(DNSW_PIN, INPUT);
    pinMode(TEMP_PIN, INPUT);
    pinMode(VCC_PIN, INPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);

    digitalWrite(LED_GREEN_PIN, HIGH);
    analogWrite(MOSFET_PIN, 255); // VERY IMPORTANT, DONT CHANGE!

    attachInterrupt(DNSW_PIN, dnsw_change_isr, FALLING);
    attachInterrupt(UPSW_PIN, upsw_change_isr, FALLING);

    Serial.begin(9600);

    // Enable Fast PWM with no prescaler
    setFastPwm();
    setVREF();

    // Start-up Display
    debugprintln("Showing startup");
    showLogo();

    debugprintln("Checking sensors");
    setupSensors();

    debugprintln("Checking first boot");
    if (isFirstBoot() || !validateCRC()) {
        doSetup();
    }

    // Pull saved values from EEPROM
    max_temp_index = getMaxTempIndex();
    bed_resistance = getResistance();

    debugprintln("Entering main menu");
    mainMenu();
}