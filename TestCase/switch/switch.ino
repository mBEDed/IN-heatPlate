#include "pico/stdlib.h"
#include <stdio.h>

#define UPSW_PIN 6    
#define DNSW_PIN 7    
#define DEBOUNCE_TIME 50  // Debounce delay in milliseconds
#define LONG_PRESS_TIME 1000 // Long-press duration in milliseconds

typedef enum { BUTTON_RELEASED, BUTTON_PRESSED, BUTTON_LONG_PRESSED } button_state_t;

// Volatile variables to track button states
volatile button_state_t up_button_state = BUTTON_RELEASED;
volatile button_state_t dn_button_state = BUTTON_RELEASED;
volatile uint32_t up_press_time = 0;
volatile uint32_t dn_press_time = 0;
volatile uint32_t last_up_interrupt = 0;
volatile uint32_t last_dn_interrupt = 0;

// Interrupt Service Routine (ISR) for Down Switch
void dnsw_change_isr(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    if (now - last_dn_interrupt < DEBOUNCE_TIME) return;  // Debounce check
    last_dn_interrupt = now;

    if (events & GPIO_IRQ_EDGE_FALL) {  // Button Pressed
        dn_press_time = now;
        dn_button_state = BUTTON_PRESSED;
    } 
    else if (events & GPIO_IRQ_EDGE_RISE) {  // Button Released
        if (now - dn_press_time >= LONG_PRESS_TIME) {
            dn_button_state = BUTTON_LONG_PRESSED;
        } else {
            dn_button_state = BUTTON_RELEASED;
        }
    }
}

// Interrupt Service Routine (ISR) for Up Switch
void upsw_change_isr(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    if (now - last_up_interrupt < DEBOUNCE_TIME) return;  // Debounce check
    last_up_interrupt = now;

    if (events & GPIO_IRQ_EDGE_FALL) {  // Button Pressed
        up_press_time = now;
        up_button_state = BUTTON_PRESSED;
    } 
    else if (events & GPIO_IRQ_EDGE_RISE) {  // Button Released
        if (now - up_press_time >= LONG_PRESS_TIME) {
            up_button_state = BUTTON_LONG_PRESSED;
        } else {
            up_button_state = BUTTON_RELEASED;
        }
    }
}

// Function to test switch behavior
void test_switches() {
    printf("\nTesting Up and Down Switches...\n");

    while (1) {
        sleep_ms(100);  // Poll every 100ms

        if (up_button_state == BUTTON_PRESSED) {
            printf("✅ Up Button Press detected.\n");
            up_button_state = BUTTON_RELEASED;  // Reset state
        } else if (up_button_state == BUTTON_LONG_PRESSED) {
            printf("🔵 Up Button Long Press detected!\n");
            up_button_state = BUTTON_RELEASED;  // Reset state
        }

        if (dn_button_state == BUTTON_PRESSED) {
            printf("✅ Down Button Press detected.\n");
            dn_button_state = BUTTON_RELEASED;  // Reset state
        } else if (dn_button_state == BUTTON_LONG_PRESSED) {
            printf("🔵 Down Button Long Press detected!\n");
            dn_button_state = BUTTON_RELEASED;  // Reset state
        }
    }
}

int main() {
    stdio_init_all();  // Initialize serial output

    // Configure pins as input with pull-up resistors
    gpio_init(UPSW_PIN);
    gpio_set_dir(UPSW_PIN, GPIO_IN);
    gpio_pull_up(UPSW_PIN);

    gpio_init(DNSW_PIN);
    gpio_set_dir(DNSW_PIN, GPIO_IN);
    gpio_pull_up(DNSW_PIN);

    // Attach interrupts for button handling
    gpio_set_irq_enabled_with_callback(UPSW_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &upsw_change_isr);
    gpio_set_irq_enabled_with_callback(DNSW_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &dnsw_change_isr);

    test_switches();  // Start test loop

    return 0;
}
