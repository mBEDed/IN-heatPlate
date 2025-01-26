#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Function to show logo on startup
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