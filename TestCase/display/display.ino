#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Define I2C pins for RP2040
#define OLED_SDA 20  // SDA on GPIO 20
#define OLED_SCL 21  // SCL on GPIO 21

// Create an OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting OLED Test...");

    // Set I2C pins for RP2040
    Wire.setSDA(OLED_SDA);
    Wire.setSCL(OLED_SCL);
    Wire.begin();

    // Initialize the OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED display not found!");
        while (1); // Halt the program if OLED is not detected
    }

    Serial.println("OLED Initialized Successfully!");

    // Clear the display
    display.clearDisplay();
    display.setTextSize(1);    
    display.setTextColor(SSD1306_WHITE);  
    display.setCursor(0, 10);
    display.println("OLED Test Successful!");
    display.display();
}

void loop() {
    // Blink test message on OLED
    display.clearDisplay();
    display.setCursor(10, 20);
    display.println("Hello, Engineers!");
    display.display();
    delay(1000);

    display.clearDisplay();
    display.setCursor(10, 20);
    display.println("Display Working!");
    display.display();
    delay(1000);
}
