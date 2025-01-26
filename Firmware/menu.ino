#include "display.ino"
#include "eeprom.ino"

// Menu states
enum menu_state_t { MENU_IDLE, MENU_SELECT_PROFILE, MENU_HEAT, MENU_INC_TEMP, MENU_DEC_TEMP };

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