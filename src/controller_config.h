#ifndef CONTROLLER_CONFIG_H
#define CONTROLLER_CONFIG_H

#define SW_GPIO_SIZE 12               // Number of switches
#define MOUSE_SENS 1                  // Mouse sensitivity multiplier
#define SW_DEBOUNCE_TIME_US 4000      // Switch debounce delay in us
#define REACTIVE_TIMEOUT_MAX 1000000  // HID to reactive timeout in us
#define WS2812B_LEDS 10           // Number of WS2812B LEDs
#define ADC_PIN 26

#ifdef PICO_GAME_CONTROLLER_C

typedef struct {
	uint8_t r, g, b;
} RGB_t;

// MODIFY KEYBINDS HERE, MAKE SURE LENGTHS MATCH SW_GPIO_SIZE
const uint8_t SW_KEYCODE[] = {HID_KEY_D, HID_KEY_F, HID_KEY_J, HID_KEY_K,
                              HID_KEY_C, HID_KEY_M, HID_KEY_A, HID_KEY_B,
                              HID_KEY_1, HID_KEY_E, HID_KEY_G};
const uint8_t SW_GPIO[] = {
    4, 5, 6, 12, 13, 14, 18, 19, 20, 27, 28, 29,
};
const uint8_t WS2812B_GPIO = 24;

// Value of 0, 0, X will be reactive on the slider position. 0,0,0 for left, 0,0,1 for right section
const RGB_t RGB_REACTIVE_VALUES[WS2812B_LEDS] = {
		{201, 130, 217}, // LEFT_WALL
		{0,0,0}, // Left programmable
		{255, 84, 84}, // LEFT_R
		{107, 255, 84}, // LEFT_G
		{84, 127, 255}, // LEFT_B
		{255, 84, 84}, // RIGHT_R
		{107, 255, 84}, // RIGHT_G
		{84, 127, 255}, // RIGHT_B
		{0,0,1}, // Right programmable
		{201, 130, 217}, // RIGHT_WALL
};

#endif

extern bool joy_mode_check;

#endif