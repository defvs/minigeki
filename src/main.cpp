#include "Arduino.h"
#include "HID-Project.h"
#include "settings.h"

unsigned long pollingLastTime = 0;

void setup() {
	Gamepad.begin();

	// Enable input on pins
	DDRB &= ~0b01111110;
	DDRC &= ~0b01000000;
	DDRD &= ~0b10010011;
	DDRE &= ~0b01000000;

	// Enable pull-up on pins
	DDRB |= 0b01111110;
	DDRC |= 0b01000000;
	DDRD |= 0b10010011;
	DDRE |= 0b01000000;
}

void loop() {
	unsigned long currentTime = micros();
	if (currentTime - pollingLastTime >= POLLING_INTERVAL_US) {
		pollingLastTime = currentTime;
		uint32_t buttons = 0;
		// Simple way to concatenate the buttons, order is not important, all we want is fast execution
		buttons |= PINB;
		buttons |= PINC << 1;
		buttons |= PINE >> 6;
		buttons |= DDRD << 8;
		Gamepad.buttons(buttons);
		Gamepad.write();
	}
}
