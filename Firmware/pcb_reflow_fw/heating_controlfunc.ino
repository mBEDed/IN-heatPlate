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

bool heat(byte max_temp, int profile_index) {
    // Heating Display
    showHeatMenu(max_temp);
    delay(3000);

    float t; // Used to store current temperature
    float v; // Used to store current voltage

    unsigned long profile_max_time = millis() / 1000 + (8 * 60);
    unsigned long step_start_time = (millis() / 1000);
    int current_step = 0;

    // Other control variables
    int x = 0;  // Heat Animate Counter
    int y = 80; // Heat Animate max (modulused below)

    float start_temp = getTemp();
    float goal_temp = profiles[profile_index].fraction[0] * max_temp;
    float step_runtime = profiles[profile_index].seconds[0];
    float last_time = 0;
    float last_temp = getTemp();
    error_I = 0;

    while (1) {
        // Cancel heat, don't even wait for uppress so we don't risk missing it during the loop
        if (getButtonsState() != BUTTONS_NO_PRESS) {
            analogWrite(MOSFET_PIN, MOSFET_PIN_OFF);
            debugprintln("cancelled");
            return 0;
        }

        // Check Heating not taken more than 8 minutes
        if (millis() / 1000 > profile_max_time) {
            analogWrite(MOSFET_PIN, MOSFET_PIN_OFF);
            debugprintln("exceeded time");
            cancelledTimer();
            return 0;
        }

        // Measure Values
        t = getTemp();
        v = getVolts();
        float max_possible_amperage = v / bed_resistance;
        float vmax = (MAX_AMPERAGE * bed_resistance) * PWM_VOLTAGE_SCALAR;
        int min_PWM = 255 - ((vmax * 255.0) / v);
        min_PWM = constrain(min_PWM, 0, 255);
        debugprint("Min PWM: ");
        debugprintln(min_PWM);
        debugprintln(bed_resistance);

        // Determine what target temp is and PID to it
        float time_into_step = ((float)millis() / 1000.0) - (float)step_start_time;
        float target_temp = min(
            ((goal_temp - start_temp) * (time_into_step / step_runtime)) + start_temp, goal_temp);

        stepPID(target_temp, t, last_temp, time_into_step - last_time, min_PWM);
        last_time = time_into_step;

        // if we finish the step timewise
        if (time_into_step >= step_runtime) {
            if (abs(t - goal_temp) < TARGET_TEMP_THRESHOLD) {
                current_step++;
                if (current_step == profiles[profile_index].points) {
                    analogWrite(MOSFET_PIN, MOSFET_PIN_OFF);
                    return 1;
                }
                last_time = 0.0;
                start_temp = t;
                goal_temp = profiles[profile_index].fraction[current_step] * max_temp;
                step_runtime = profiles[profile_index].seconds[current_step] -
                               profiles[profile_index].seconds[current_step - 1];
                step_start_time = millis() / 1000.0;
            }
        }

        heatAnimate(x, y, v, t, target_temp);
    }
}

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