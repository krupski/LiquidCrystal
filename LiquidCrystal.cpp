///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD) driver
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//
//  Changes and additions: Added an SPI-like serial driver
//  for the Noritake CUU class of Vacuum Florescent Displays
//  Copyright (c) 2012, 2014 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 30 December 2014
//
//  Free software. Use, modify, distribute freely. No warranty is
//  given or implied. Absolutely NOT for medical, aerospace or any
//  other life-critical applications.
//
///////////////////////////////////////////////////////////////////////////////
//
//  When the display powers up, it is configured as follows:
//
//  1. Display clear
//  2. Function set:
//     DL = 1; 8-bit interface data
//     N = 0; 1-line display
//     F = 0; 5x8 dot character font
//  3. Display on/off control:
//     D = 0; Display off
//     C = 0; Cursor off
//     B = 0; Blinking off
//  4. Entry mode set:
//     I/D = 1; Increment by 1
//     S = 0; No shift
//
//  Note, however, that resetting the Arduino doesn't reset the LCD, so we
//  can't assume that its in that state when a sketch starts (and the
//  LiquidCrystal constructor is called). So begin () does a software
//  reset and display config is setup.
//
//  Note that in Serial mode, hardware reset DOES work IF the optional
//  reset enable jumper is closed on the Noritake display module and if
//  the reset pin (pin 6) is wired to an Arduino pin and if the reset
//  pin is defined in the LiquidCrystal constructor.
//
///////////////////////////////////////////////////////////////////////////////

#include "LiquidCrystal.h"

// serial interface, hardware reset not available
LiquidCrystal::LiquidCrystal (uint8_t sck, uint8_t stb, uint8_t siso)
{
	init (MODE_S, sck, stb, siso, NO_RST, 0, 0, 0, 0, 0, 0, 0);
}

// serial interface, hardware reset is enabled and available (D0 pin is used for reset)
LiquidCrystal::LiquidCrystal (uint8_t sck, uint8_t stb, uint8_t siso, uint8_t reset)
{
	init (MODE_S, sck, stb, siso, reset, 0, 0, 0, 0, 0, 0, 0);
}

// parallel interface 4 bits without active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t enable,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init (MODE_4, rs, NO_RW, enable, 0, 0, 0, 0, d4, d5, d6, d7);
}

// parallel interface 4 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init (MODE_4, rs, rw, enable, 0, 0, 0, 0, d4, d5, d6, d7);
}

// parallel interface 8 bits without active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init (MODE_8, rs, NO_RW, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

// parallel interface 8 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init (MODE_8, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

void LiquidCrystal::init (
	uint8_t bitmode, uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	_delay_ms (50); // wait more than 40 msec after powerup
	_bit_mode = bitmode;

	if (_bit_mode == MODE_S) { // 0xFF == serial mode
		_bit_mode = MODE_8; // reset it to 8 bit mode
		_sck = rs;
		_stb = rw;
		_siso = enable;
		_reset = d0;
		_serial_cmd = (HEAD | RWBIT | RSBIT);
		_serial_mode = 1;
		digitalWrite (_sck, HIGH);
		digitalWrite (_stb, HIGH);
		digitalWrite (_siso, HIGH);
		digitalWrite (_reset, HIGH);
		pinMode (_sck, OUTPUT);
		pinMode (_stb, OUTPUT);
		pinMode (_siso, INPUT_PULLUP);
		pinMode (_reset, INPUT_PULLUP);

		if (_reset != NO_RST) {
			pinMode (_reset, OUTPUT);
			digitalWrite (_reset, LOW);
			_delay_ms (1);
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
		digitalWrite (_rs_pin, CMD);
		digitalWrite (_rw_pin, READ);
		digitalWrite (_enable_pin, LOW);
		pinMode (_rs_pin, OUTPUT);
		pinMode (_rw_pin, INPUT); // default not used unless enabled below
		pinMode (_enable_pin, OUTPUT);

		if (_rw_pin != NO_RW) {
			pinMode (_rw_pin, OUTPUT);
		}
	}
}

void LiquidCrystal::begin (uint8_t cols, uint8_t rows, uint8_t dotsize)
{
	_numcols = cols;
	_numrows = rows;
	// software reset (Hitachi manual pg. 45 & 46)
	// this is always written in 8 bit mode therefore we
	// use "_write8bits" instead of "_send"
	_write8bits (LCD_FUNCTIONSET | LCD_8BITMODE);
	_delay_ms (5); // wait more than 4.1ms (pg 45)
	_write8bits (LCD_FUNCTIONSET | LCD_8BITMODE);
	_delay_ms (1); // wait more than 100 us (pg 45)
	_write8bits (LCD_FUNCTIONSET | LCD_8BITMODE);
	_delay_ms (1); // wait more than 100 us (pg 45)
	// build setup byte for function set
	_displayFunction = LCD_FUNCTIONSET; // init
	_displayFunction |= ((_bit_mode == MODE_4) ? LCD_4BITMODE : LCD_8BITMODE); // set 4 or 8 bit interface
	_displayFunction |= ((_numrows == 1) ? LCD_1LINE : LCD_2LINE); // LCD 1 line or more
	_displayFunction |= (dotsize && (_numrows == 1) ? LCD_5x10DOTS : LCD_5x8DOTS); // tall bitmap if supported
	_write8bits (_displayFunction); // set 4/8 bit mode, lines and font

	///////////////////////////////////////////////////////////////////////
	// from here on we need to use "_send_cmd" to automatically use      //
	// the 8 bit or 4 bit interface as setup.                            //
	// note: busy flag is valid from here after but we don't use it :(   //
	///////////////////////////////////////////////////////////////////////

	// build setup byte for display control
	_displayControl = LCD_DISPLAYCONTROL; // init
	_displayControl |= (LCD_BLINKOFF | LCD_CURSOROFF | LCD_DISPLAYON); // cursor & blink off, display on
	_send_cmd (_displayControl); // send displaycontrol command
	clearScreen (); // clear screen
	// ready to go from here!
}

void LiquidCrystal::setBrightness (uint8_t percent)
{
	uint8_t _brightness = 0b11;
	uint16_t i = 1000;
	percent = (percent > 100) ? 100 : percent;

	while (i && _brightness) {
		if ((percent * 10) > (i + 5)) {
			_brightness--;
		}

		i -= 250;
	}

	_send_cmd (_displayFunction); // execute a function set
	_send_dat (_brightness); // set brightness
}

void LiquidCrystal::home (void)
{
	_send_cmd (LCD_RETURNHOME);
	_delay_ms (2);
}

void LiquidCrystal::clear (void)
{
	clearScreen ();
}

void LiquidCrystal::clearScreen (void)
{
	_send_cmd (LCD_CLEARDISPLAY);
	_delay_ms (2);
}

void LiquidCrystal::setLine (uint8_t x, uint8_t y)
{
	setCursor (x, y);
}

void LiquidCrystal::setCursor (uint8_t x, uint8_t y)
{
	uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

	if ((x < _numcols) && (y < _numrows)) {
		_send_cmd (LCD_SETDDRAMADDR | (x + row_offsets[y]));
		_cur_x = x; // "real" cursor X position
		_cur_y = y; // "real" cursor Y position

	} else {
		setCursor (0, 0);
	}
}

void LiquidCrystal::getCursor (uint8_t &x, uint8_t &y)
{
	x = _cur_x;
	y = _cur_y;
}

// compatibility with original LiquidCrystal functions
void LiquidCrystal::noDisplay (void)
{
	setDisplay (0);
}

void LiquidCrystal::display (void)
{
	setDisplay (1);
}

void LiquidCrystal::noCursor (void)
{
	setUnderline (0);
}

void LiquidCrystal::cursor (void)
{
	setUnderline (1);
}

void LiquidCrystal::noBlink (void)
{
	setBlink (0);
}

void LiquidCrystal::blink (void)
{
	setBlink (1);
}

// Turn the display on/off (quickly)
void LiquidCrystal::setDisplay (uint8_t on)
{
	on ? _displayControl |= LCD_DISPLAYON : _displayControl &= ~LCD_DISPLAYON;
	_send_cmd (LCD_DISPLAYCONTROL | _displayControl);
}

// Turns the underline cursor on/off
void LiquidCrystal::setUnderline (uint8_t on)
{
	on ? _displayControl |= LCD_CURSORON : _displayControl &= ~LCD_CURSORON;
	_send_cmd (LCD_DISPLAYCONTROL | _displayControl);
}

// Turn on and off the blinking cursor
void LiquidCrystal::setBlink (uint8_t on)
{
	on ? _displayControl |= LCD_BLINKON : _displayControl &= ~LCD_BLINKON;
	_send_cmd (LCD_DISPLAYCONTROL | _displayControl);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createChar (uint8_t location, const uint8_t *charmap)
{
	uint8_t i;
	location &= 0x0f; // we only have 8 locations 0-7 or 8-F
	_send_cmd (LCD_SETCGRAMADDR | (location * 8));

	for (i = 0; i < 8; i++) {
		_send_dat (* (charmap + i)); // 8 bytes to a char (but only 5 bits)
	}
}

void LiquidCrystal::createChar_P (uint8_t location, const uint8_t *charmap)
{
	uint8_t i;
	location &= 0x0f; // we only have 8 locations 0-7 or 8-F
	_send_cmd (LCD_SETCGRAMADDR | (location * 8));

	for (i = 0; i < 8; i++) {
		_send_dat (pgm_read_byte (charmap + i));
	}
}

// private functions
void LiquidCrystal::_send_cmd (uint8_t data)
{
	_send (data, CMD);
}

void LiquidCrystal::_send_dat (uint8_t data)
{
	write (data);
}

inline size_t LiquidCrystal::write (uint8_t data)
{
	switch (data) {
		case 0x08: {
				return _backSpace ();
			}

		case 0x0a: {
				return _lineFeed ();
			}

		case 0x0c: {
				clearScreen ();
				return 0;
			}

		case 0x0d: {
				return _carriageReturn ();
			}

		default: {
				break;
			}
	}

	_send (data, DAT);
	_cur_x++;

	if (_cur_x < _numcols) { // if next col pos isn't at end
		setCursor (_cur_x, _cur_y); // move it right, keep same row

	} else {
		if ((_cur_y + 1) < _numrows) { // need new row
			setCursor (0, _cur_y + 1);

		} else {
			setCursor (0, 0); // otherwise, home 0,0
		}
	}

	return 1;
}

size_t LiquidCrystal::_backSpace (void)
{
	uint8_t _tmp_x = _cur_x;
	uint8_t _tmp_y = _cur_y;

	if (_tmp_x) {
		// backup 1 char
		setCursor ((_tmp_x - 1), _tmp_y);
		write (0x20);
		setCursor ((_tmp_x - 1), _tmp_y);

	} else {
		// wrap from end -> beginning
		setCursor ((_numcols - 1), (_tmp_y) ? (_tmp_y - 1) : (_numrows - 1));
		write (0x20);
		setCursor ((_numcols - 1), (_tmp_y) ? (_tmp_y - 1) : (_numrows - 1));
	}

	return (size_t) -1;
}

size_t LiquidCrystal::_lineFeed (void)
{
	if ((_cur_y + 1) > _numrows) {
		setCursor (_cur_x, 0); // wrap bottom back to top

	} else {
		setCursor (_cur_x, (_cur_y + 1)); // cursor down one line
	}

	return (size_t) 1;
}

size_t LiquidCrystal::_carriageReturn (void)
{
	setCursor (0, _cur_y);
	return 0;
}

void LiquidCrystal::_setRW (uint8_t rw)
{
	if (_serial_mode) {
		rw ? _serial_cmd |= RWBIT : _serial_cmd &= ~RWBIT;

	} else {
		if (_rw_pin != NO_RW) {
			digitalWrite (_rw_pin, rw);
		}
	}
}

// write either command or data
void LiquidCrystal::_send (uint8_t data, uint8_t mode)
{
	if (_serial_mode) {
		mode ? _serial_cmd |= RSBIT : _serial_cmd &= ~RSBIT;

	} else {
		digitalWrite (_rs_pin, mode);
	}

	if (_bit_mode == MODE_4) {
		_write4bits (data >> 4);
		_write4bits (data);

	} else {
		_write8bits (data);
	}
}

void LiquidCrystal::_write4bits (uint8_t data)
{
	uint8_t n = 4;
	_setRW (WRITE);

	while (n--) {
		pinMode (_data_pins[n + 4], OUTPUT);
		digitalWrite (_data_pins[n + 4], (data & _BV (n)) ? HIGH : LOW);
	}

	digitalWrite (_enable_pin, HIGH);
	asm volatile (
		"\tnop\n"
		"\tnop\n"
		"\tnop\n"
		"\tnop\n"
		"\tnop\n"
		"\tnop\n"
	);
	digitalWrite (_enable_pin, LOW);
	_setRW (READ);
}

void LiquidCrystal::_write8bits (uint8_t data)
{
	uint8_t n = 8;

	if (_serial_mode) {
		_setRW (WRITE);
		digitalWrite (_stb, LOW);
		_serial_IO (_serial_cmd);
		_serial_IO (data);
		digitalWrite (_stb, HIGH);
		_setRW (READ);

	} else {
		_setRW (WRITE);

		while (n--) {
			pinMode (_data_pins[n], OUTPUT);
			digitalWrite (_data_pins[n], (data & _BV (n)) ? HIGH : LOW);
		}

		digitalWrite (_enable_pin, HIGH);
		asm volatile (
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
		);
		digitalWrite (_enable_pin, LOW);
		_setRW (READ);
	}
}

// serial I/O for Noritake CUU displays
// note: sck, stb, siso, and optionally reset are already defined
// this is like SPI, but using only one pin for both R and W
// currently received data isn't used anywhere
uint8_t LiquidCrystal::_serial_IO (uint8_t data)
{
	uint8_t bits = 8;

	while (bits--) {
		digitalWrite (_sck, LOW); // setup pin for data in
		pinMode (_siso, OUTPUT); // set IO pin as output
		digitalWrite (_siso, (data & _BV (bits))); // write to pin
		digitalWrite (_sck, HIGH); // clock the bit in
		pinMode (_siso, INPUT); // setup pin for data in
		digitalRead (_siso) ? data |= _BV (bits) : data &= ~_BV (bits); // read the pin
	}

	return data;
}

// end of LiquidCrystal.cpp
