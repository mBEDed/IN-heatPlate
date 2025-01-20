inline bool isFirstBoot() {
    uint8_t first_boot = EEPROM.read(FIRSTTIME_BOOT_ADDR);
    debugprint("Got first boot flag: ");
    debugprintln(first_boot);
    return first_boot != 1;
}

inline void setFirstBoot() {
    EEPROM.write(FIRSTTIME_BOOT_ADDR, 1);
    updateCRC();
}

inline float getResistance() {
    float f;
    return EEPROM.get(RESISTANCE_INDEX_ADDR, f);
}

inline void setResistance(float resistance) {
    EEPROM.put(RESISTANCE_INDEX_ADDR, resistance);
    updateCRC();
}

inline void setMaxTempIndex(int index) {
    EEPROM.update(TEMP_INDEX_ADDR, index);
    updateCRC();
}

inline int getMaxTempIndex(void) { 
    return EEPROM.read(TEMP_INDEX_ADDR) % sizeof(max_temp_array); 
}