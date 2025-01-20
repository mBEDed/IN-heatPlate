// -------------------- Function prototypes -----------------------------------
void inline heatAnimate(int &x, int &y, float v, float t, float target_temp);
void setupSensors();
void showLogo();
void mainMenu();
bool validateCRC();
void setCRC(uint32_t new_crc);
uint32_t eepromCRC(void);
float getTemp();
float getVolts();