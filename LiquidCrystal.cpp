///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD) driver
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//
//  Changes and additions: Added an SPI-like serial driver
//  for the Noritake CUU class of Vacuum Florescent Displays.
//  Copyright (c) 2012, 2014 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 30 January 2014
//
//  Free software. Use, modify, distribute freely. No warranty is
//  given or implied. Absolutely NOT for medical, aerospace or any
//  other life-critical applications.
//
///////////////////////////////////////////////////////////////////////////////
//
// When the display powers up, it is configured as follows:
//
//	1. Display clear
//	2. Function set:
//		DL = 1; 8-bit interface data
//		N = 0; 1-line display
//		F = 0; 5x8 dot character font
//	3. Display on/off control:
//		D = 0; Display off
//		C = 0; Cursor off
//		B = 0; Blinking off
//	4. Entry mode set:
//		I/D = 1; Increment by 1
//		S = 0; No shift
//
//	Note, however, that resetting the Arduino doesn't reset the LCD, so we
//	can't assume that its in that state when a sketch starts (and the
//	LiquidCrystal constructor is called).
//
//  Note that in Serial mode, hardware reset DOES work IF the optional
//  reset enable jumper is closed on the Noritake display module and if
//  the reset pin (pin 6) is wired to an Arduino pin and if the reset
//  pin is defined in the LiquidCrystal constructor.
//
///////////////////////////////////////////////////////////////////////////////

#include "LiquidCrystal.h"

// serial interface, hardware reset not available
LiquidCrystal::LiquidCrystal (uint8_t sck, uint8_t stb, uint8_t siso) {
	init (255, sck, stb, siso, 255, 0, 0, 0, 0, 0, 0, 0);
}

// serial interface, hardware reset is enabled and available
LiquidCrystal::LiquidCrystal (uint8_t sck, uint8_t stb, uint8_t siso, uint8_t reset) {
	init (255, sck, stb, siso, reset, 0, 0, 0, 0, 0, 0, 0);
}

// parallel interface
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
	init (0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
	init (0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
	init (1, rs, rw, enable, 0, 0, 0, 0, d4, d5, d6, d7);
}

LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t enable,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
	init (1, rs, 255, enable, 0, 0, 0, 0, d4, d5, d6, d7);
}

void LiquidCrystal::init (
	uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
	if (fourbitmode == 255) { // 255 == serial mode

		fourbitmode = 0; // reset it to 8 bit mode

		_sck = rs;
		_stb = rw;
		_siso = enable;
		_reset = d0;

		_serial_cmd = 0b11111000; // r/w low (write), rs low
		_serial_mode = 1;

		digitalWrite (_sck, HIGH);
		digitalWrite (_stb, HIGH);
		digitalWrite (_siso, HIGH);
		digitalWrite (_reset, HIGH);

		pinMode (_sck, OUTPUT);
		pinMode (_stb, OUTPUT);
		pinMode (_siso, OUTPUT);
		pinMode (_reset, INPUT_PULLUP);

		if (_reset != 255) { // pulse the reset pin
			pinMode (_reset, OUTPUT);
			digitalWrite (_reset, LOW);
			_delay_ms (10);
			digitalWrite (_reset, HIGH);
			pinMode (_reset, INPUT_PULLUP);
		}

	} else { // parallel mode

		_rs_pin = rs;
		_rw_pin = rw;
		_enable_pin = enable;

		_data_pins[0] = d0;
		_data_pins[1] = d1;
		_data_pins[2] = d2;
		_data_pins[3] = d3;
		_data_pins[4] = d4;
		_data_pins[5] = d5;
		_data_pins[6] = d6;
		_data_pins[7] = d7;

		_serial_mode = 0;

		pinMode (_rs_pin, OUTPUT);
		pinMode (_enable_pin, OUTPUT);
		digitalWrite (_rs_pin, LOW);
		digitalWrite (_enable_pin, LOW);
	}

	if (fourbitmode) {
		_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	} else {
		_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
	}

	_delay_ms (15); // hd44780 manual pg. 45
	begin (16, 1); // default begin - overridden by user setup
	setBrightness (100);
}

void LiquidCrystal::begin (uint8_t cols, uint8_t lines, uint8_t dotsize)
{
	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}

	_numcols = cols;
	_numlines = lines;

	// for some 1 line displays you can select a 10 pixel high font
	if ( (dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	//put the LCD into 4 bit or 8 bit mode
	if (! (_displayfunction & LCD_8BITMODE)) {
		// send init commands in 8 bit mode
		_write8bits (LCD_FUNCTIONSET | LCD_8BITMODE);
		_delay_us (4100); // wait min 4.1ms (pg 45)
		// second try
		_write8bits (LCD_FUNCTIONSET | LCD_8BITMODE);
		_delay_us (100); // wait 100 us
		// third go!
		_write8bits (LCD_FUNCTIONSET | LCD_8BITMODE);
		_delay_us (100); // wait 100 us
		// finally, set to 4-bit interface
		_write8bits (LCD_FUNCTIONSET | LCD_4BITMODE);

	} else {
		// this is according to the hitachi HD44780 datasheet
		// page 45 figure 23
		// Send function set command sequence
		_command (LCD_FUNCTIONSET | _displayfunction);
		_delay_us (4100); // wait more than 4.1ms
		// second try
		_command (LCD_FUNCTIONSET | _displayfunction);
		_delay_us (100);
		// third go
		_command (LCD_FUNCTIONSET | _displayfunction);
		_delay_us (100);
	}

	// finally, set # lines, font size, etc.
	_command (LCD_FUNCTIONSET | _displayfunction);
	// turn the display on with no cursor or blinking default
	setDisplay (1); // display on
	setCursor (0); // cursor off
	setBlink (0); // blink off
	// clear screen
	clearScreen ();
	// Initialize to default text direction
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// set the entry mode
	_command (LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::setBrightness (uint8_t percent)
{
	uint8_t x;
	uint8_t brightness = 0x03;
	uint16_t i = 1000;
	percent = (percent > 100) ? 100 : percent;

	while (i && brightness) {
		if ( (percent * 10) > (i + 5)) {
			brightness--;
		}

		i -= 250;
	}

	_command (LCD_FUNCTIONSET | _displayfunction);
	_send (brightness, DAT);
}

void LiquidCrystal::clearScreen (void)
{
	_command (LCD_CLEARDISPLAY);
	_delay_us (2000);
}

// set cursor to row and column based on font size
void LiquidCrystal::setLine (uint8_t x, uint8_t y)
{
	setCursor (x, y);
}

void LiquidCrystal::setCursor (uint8_t x, uint8_t y)
{
	uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	_command (LCD_SETDDRAMADDR | (x + row_offsets[y]));
}

// Turn the display on/off (quickly)
void LiquidCrystal::setDisplay (uint8_t on) {
	on ? _displaycontrol |= LCD_DISPLAYON : _displaycontrol &= ~LCD_DISPLAYON;
	_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal::setCursor (uint8_t on) {
	on ? _displaycontrol |= LCD_CURSORON : _displaycontrol &= ~LCD_CURSORON;
	_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal::setBlink (uint8_t on)
{
	on ? _displaycontrol |= LCD_BLINKON : _displaycontrol &= ~LCD_BLINKON;
	_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createChar (uint8_t location, const uint8_t *charmap)
{
	uint8_t i;
	location &= 0x0f; // we only have 8 locations 0-7 or 8-F
	_command (LCD_SETCGRAMADDR | (location * 8));
	for (i = 0; i < 8; i++) {
		write ( *(charmap + i));
	}
}

void LiquidCrystal::createChar_P (uint8_t location, const uint8_t *charmap)
{
	uint8_t i;
	location &= 0x0f; // we only have 8 locations 0-7 or 8-F
	_command (LCD_SETCGRAMADDR | (location * 8));
	for (i = 0; i < 8; i++) {
		write (pgm_read_byte (charmap + i));
	}
}

// private functions
// set various functions
inline void LiquidCrystal::_command (uint8_t value)
{
	_send (value, CMD);
}

inline size_t LiquidCrystal::write (uint8_t value)
{
	_send (value, DAT);
	return 1; // assume sucess
}

void LiquidCrystal::_setRW (uint8_t rw)
{
	if (_serial_mode) {
		rw ? _serial_cmd |= RWBIT : _serial_cmd &= ~RWBIT;

	} else if (_rw_pin != 255) {
		pinMode (_rw_pin, OUTPUT);
		digitalWrite (_rw_pin, rw);
	}
}

// write either command or data
void LiquidCrystal::_send (uint8_t value, uint8_t mode)
{
	if (_serial_mode) {
		mode ? _serial_cmd |= RSBIT : _serial_cmd &= ~RSBIT;

	} else {
		digitalWrite (_rs_pin, mode);
	}

	if (_displayfunction & LCD_8BITMODE) {
		_write8bits (value);

	} else {
		_write4bits (value >> 4);
		_write4bits (value);
	}
}

void LiquidCrystal::_write4bits (uint8_t value)
{
	uint8_t n = 4;

	while (n--) {
		pinMode (_data_pins[4 + n], OUTPUT);
		digitalWrite (_data_pins[4 + n], (value & _BV (n)) ? 1 : 0);
	}

	_setRW (WRITE);
	digitalWrite (_enable_pin, HIGH);
	digitalWrite (_enable_pin, LOW);
	_setRW (READ);
}

void LiquidCrystal::_write8bits (uint8_t value)
{
	uint8_t n = 8;

	if (_serial_mode) {

		_setRW (WRITE);
		digitalWrite (_stb, LOW);
		_sendSerial (_serial_cmd);
		_sendSerial (value);
		digitalWrite (_stb, HIGH);
		_setRW (READ);

	} else {

		while (n--) {
			pinMode (_data_pins[n], OUTPUT);
			digitalWrite (_data_pins[n], (value & _BV (n)) ? 1 : 0);
		}

		_setRW (WRITE);
		digitalWrite (_enable_pin, HIGH);
		digitalWrite (_enable_pin, LOW);
		_setRW (READ);
	}
}

// serial I/O for Noritake CUU displays
void LiquidCrystal::_sendSerial (uint8_t data)
{
	uint8_t bits = 8;
	pinMode (_siso, OUTPUT);
	while (bits--) {
		digitalWrite (_sck, LOW);
		digitalWrite (_siso, (data & _BV (bits)));
		digitalWrite (_sck, HIGH);
	}
	pinMode (_siso, INPUT);
}

// ---- end of LiquidCrystal.cpp ---- //
