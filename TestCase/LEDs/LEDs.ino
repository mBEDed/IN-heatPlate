#include <Arduino.h>

// Define LED pin connections
#define RED_LED_PIN 8   // Red LED on GPIO 8
#define GREEN_LED_PIN 9  // Green LED on GPIO 9

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting LED Test...");

    // Set LED pins as OUTPUT
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
}

void loop() {
    Serial.println("Testing LEDs...");

    // Test Red LED
    digitalWrite(RED_LED_PIN, HIGH);
    delay(500);
    Serial.print("Red LED ON | Pin State: ");
    Serial.println(digitalRead(RED_LED_PIN));

    digitalWrite(RED_LED_PIN, LOW);
    delay(500);
    Serial.print("Red LED OFF | Pin State: ");
    Serial.println(digitalRead(RED_LED_PIN));

    // Test Green LED
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay(500);
    Serial.print("Green LED ON | Pin State: ");
    Serial.println(digitalRead(GREEN_LED_PIN));

    digitalWrite(GREEN_LED_PIN, LOW);
    delay(500);
    Serial.print("Green LED OFF | Pin State: ");
    Serial.println(digitalRead(GREEN_LED_PIN));

    Serial.println("------------------------");
    delay(1000);
}
