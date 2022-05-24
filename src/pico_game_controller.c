/*
 * M.I.N.I.G.E.K.I v2
 * @author defvs
 * Adapted from original by SpeedyPotato. See LICENSES for more info.
 */
#define PICO_GAME_CONTROLLER_C

#include <stdio.h>
#include <stdlib.h>

#include "bsp/board.h"
#include "controller_config.h"
#include "encoders.pio.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "ws2812.pio.h"

PIO pio, pio_1;
uint16_t slider_val;
uint16_t prev_slider_val;

bool prev_sw_val[SW_GPIO_SIZE];
uint64_t sw_timestamp[SW_GPIO_SIZE];

bool kbm_report;

uint64_t reactive_timeout_timestamp;

void (*loop_mode)();

bool joy_mode_check = true;

union {
	RGB_t rgb[WS2812B_LEDS];
	uint8_t raw[WS2812B_LEDS * 3];
} lights_report;

RGB_t rgb_reactive_current[WS2812B_LEDS];

/**
 * WS2812B RGB Assignment
 * @param pixel_grb The pixel color to set
 **/
/*static inline void put_pixel(uint32_t pixel_grb) {
	pio_sm_put_blocking(pio1, ENC_GPIO_SIZE, pixel_grb << 8u); // FIXME
}*/

/**
 * WS2812B RGB Format Helper
 **/
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
	return ((uint32_t) (r) << 8) | ((uint32_t) (g) << 16) | (uint32_t) (b);
}

/**
 * WS2812B Lighting
 * @param counter Current number of WS2812B cycles
 **/
void ws2812b_update(uint32_t counter) {
	if (time_us_64() - reactive_timeout_timestamp >= REACTIVE_TIMEOUT_MAX) {

	} else {
		// TODO
	}
}

/**
 * HID/Reactive Lights
 **/
void update_lights() {
	if (time_us_64() - reactive_timeout_timestamp >= REACTIVE_TIMEOUT_MAX) {
		// Reactive mode: we set the LEDs ourselves.
		// TODO: LED react to button press.
		RGB_t value;
		for (int i = 0; i < WS2812B_LEDS; ++i) {
			value = RGB_REACTIVE_VALUES[i];
			if (value.r == 0 && value.g == 0 && value.b <= 1) { // Special mode
				uint8_t calcValue;
				if (value.b == 0)
					calcValue = slider_val;
				else
					calcValue = (1<<7) - slider_val;
				rgb_reactive_current[i].r = rgb_reactive_current[i].g = rgb_reactive_current[i].b = calcValue;
			} else rgb_reactive_current[i] = value;
		}
	} else {
		// HID mode: copy RGB values from received report
		for (int i = 0; i < WS2812B_LEDS; i++) rgb_reactive_current[i] = lights_report.rgb[i];
	}
}

struct report {
	uint16_t buttons;
	uint8_t joy0;
	uint8_t joy1;
} report;

/**
 * Gamepad Mode
 **/
void joy_mode() {
	if (tud_hid_ready()) {
		uint16_t translate_buttons = 0;
		for (int i = SW_GPIO_SIZE - 1; i >= 0; i--) {
			if (!gpio_get(SW_GPIO[i]) &&
				time_us_64() - sw_timestamp[i] >= SW_DEBOUNCE_TIME_US) {
				translate_buttons =
						(translate_buttons << 1) | (!gpio_get(SW_GPIO[i]) ? 1 : 0);
			} else {
				translate_buttons <<= 1;
			}
		}
		report.buttons = translate_buttons;

		report.joy0 = slider_val;
		report.joy1 = 0;

		tud_hid_n_report(0x00, REPORT_ID_JOYSTICK, &report, sizeof(report));
	}
}

/**
 * Keyboard Mode
 **/
void key_mode() {
	if (tud_hid_ready()) {  // Wait for ready, updating mouse too fast hampers
		// movement
		/*------------- Keyboard -------------*/
		uint8_t nkro_report[32] = {0};
		for (int i = 0; i < SW_GPIO_SIZE; i++) {
			if (!gpio_get(SW_GPIO[i]) &&
				time_us_64() - sw_timestamp[i] >= SW_DEBOUNCE_TIME_US) {
				uint8_t bit = SW_KEYCODE[i] % 8;
				uint8_t byte = (SW_KEYCODE[i] / 8) + 1;
				if (SW_KEYCODE[i] >= 240 && SW_KEYCODE[i] <= 247) {
					nkro_report[0] |= (1 << bit);
				} else if (byte > 0 && byte <= 31) {
					nkro_report[byte] |= (1 << bit);
				}
			}
		}

		/*------------- Mouse -------------*/
		// find the delta between previous and current slider_val
		uint16_t delta = 0;
		delta = (slider_val - prev_slider_val);
		prev_slider_val = slider_val;

		if (kbm_report) {
			tud_hid_n_report(0x00, REPORT_ID_KEYBOARD, &nkro_report,
							 sizeof(nkro_report));
		} else {
			tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta * MOUSE_SENS,0, 0, 0);
		}
		// Alternate reports
		kbm_report = !kbm_report;
	}
}

/**
 * Update Input States
 * Note: Switches are pull up, negate value
 **/
void update_inputs() {
	for (int i = 0; i < SW_GPIO_SIZE; i++) {
		// If switch gets pressed, record timestamp
		if (prev_sw_val[i] == false && !gpio_get(SW_GPIO[i]) == true) {
			sw_timestamp[i] = time_us_64();
		}
		prev_sw_val[i] = !gpio_get(SW_GPIO[i]);
	}
	slider_val = adc_read();
}

/**
 * Second Core Runnable
 **/
void core1_entry() {
	uint32_t counter = 0;
	while (1) {
		ws2812b_update(++counter);
		sleep_ms(5);
	}
}

/**
 * Initialize Board Pins
 **/
void init() {
	// LED Pin on when connected
/*
	gpio_init(25);
	gpio_set_dir(25, GPIO_OUT);
	gpio_put(25, 1);
*/

	// Set up the state machine for encoders
	pio = pio0;
	uint offset = pio_add_program(pio, &encoders_program);

	reactive_timeout_timestamp = time_us_64();

	// Set up WS2812B
	pio_1 = pio1;
	uint offset2 = pio_add_program(pio_1, &ws2812_program);
	// FIXME
	// ws2812_program_init(pio_1, ENC_GPIO_SIZE, offset2, WS2812B_GPIO, 800000,false);

	// Setup Button GPIO
	for (int i = 0; i < SW_GPIO_SIZE; i++) {
		prev_sw_val[i] = false;
		sw_timestamp[i] = 0;
		gpio_init(SW_GPIO[i]);
		gpio_set_function(SW_GPIO[i], GPIO_FUNC_SIO);
		gpio_set_dir(SW_GPIO[i], GPIO_IN);
		gpio_pull_up(SW_GPIO[i]);
	}

	// Setup ADC
	adc_init();
	adc_gpio_init(ADC_PIN);
	adc_select_input(ADC_PIN - 26);

	// Set listener bools
	kbm_report = false;

	// Joy/KB Mode Switching
	if (!gpio_get(SW_GPIO[0])) {
		loop_mode = &key_mode;
		joy_mode_check = false;
	} else {
		loop_mode = &joy_mode;
		joy_mode_check = true;
	}

	// Disable RGB
	if (gpio_get(SW_GPIO[8])) {
		multicore_launch_core1(core1_entry);
	}
}

/**
 * Main Loop Function
 **/
int main(void) {
	board_init();
	init();
	tusb_init();

	while (1) {
		tud_task();  // tinyusb device task
		update_inputs();
		loop_mode();
		update_lights();
	}
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
							   hid_report_type_t report_type, uint8_t *buffer,
							   uint16_t reqlen) {
	// TODO not Implemented
	(void) itf;
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) reqlen;

	return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
						   hid_report_type_t report_type, uint8_t const *buffer,
						   uint16_t bufsize) {
	(void) itf;
	if (report_id == 2 && report_type == HID_REPORT_TYPE_OUTPUT &&
		bufsize >= sizeof(lights_report))  // light data
	{
		size_t i = 0;
		for (i; i < sizeof(lights_report); i++) {
			lights_report.raw[i] = buffer[i];
		}
		reactive_timeout_timestamp = time_us_64();
	}
}
