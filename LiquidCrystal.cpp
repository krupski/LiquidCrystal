///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD) driver
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//
//  Changes and additions: Added an SPI-like serial driver
//  for the Noritake CUU class of Vacuum Florescent Displays
//  Copyright (c) 2012, 2015 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 19 July 2015
//
//  Free software. Use, modify, distribute freely. No warranty is
//  given or implied. Absolutely NOT for medical, aerospace or any
//  other life-critical applications.
//
///////////////////////////////////////////////////////////////////////////////

#include "LiquidCrystal.h"

// SPI serial interface, hardware reset not available
LiquidCrystal::LiquidCrystal (
	uint8_t siso, uint8_t stb, uint8_t sck
) {
	init (MODE_S, siso, stb, sck, NO_RST, 0, 0, 0, 0, 0, 0, 0);
}

// SPI serial interface, hardware reset is enabled and available (D0 pin is used for reset)
LiquidCrystal::LiquidCrystal (
	uint8_t siso, uint8_t stb, uint8_t sck, uint8_t reset
) {
	init (MODE_S, siso, stb, sck, reset, 0, 0, 0, 0, 0, 0, 0);
}

// parallel interface 4 bits without active r/w (must tie r/w low manually)
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_4, rs, NO_RW, en, 0, 0, 0, 0, d4, d5, d6, d7);
}

// parallel interface 4 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_4, rs, rw, en, 0, 0, 0, 0, d4, d5, d6, d7);
}

// parallel interface 8 bits without active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_8, rs, NO_RW, en, d0, d1, d2, d3, d4, d5, d6, d7);
}

// parallel interface 8 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_8, rs, rw, en, d0, d1, d2, d3, d4, d5, d6, d7);
}

void LiquidCrystal::init (
	uint8_t bitmode, uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {

	uint8_t x, dpp;
	uint8_t data_pin[] = {
		d0, d1, d2, d3, d4, d5, d6, d7,
	};

	_delay_us (50000); // wait more than 40 msec after powerup
	_bit_mode = bitmode; // 4 bit (0x04), 8 bit (0x08) or serial (0xFF) mode flag

	if (_bit_mode == MODE_S) { // 0xFF == serial mode

		_bit_mode = MODE_8; // reset it to 8 bit mode
		_serial_mode = 1; // flag "we are in serial mode"
		_reset_pin = d0; // alternate use of pin

		// template for serial command. note that bit 2 (RW) and bit 1 (RS)
		// are dynamically set or cleared as required by the driver itself.
		_serial_cmd = (0b11111110);

		dpp = digitalPinToPort (rs); // SISO pin is on RS
		_SIO_BIT  = digitalPinToBitMask (rs);
		_SIO_PIN  = portInputRegister  (dpp);
		_SIO_PORT = portOutputRegister (dpp);
		_SIO_DDR  = portModeRegister   (dpp);
		*_SIO_PORT |= _SIO_BIT; // set SISO pin high
		*_SIO_DDR &= ~_SIO_BIT; // SISO pin is in "input_pullup" mode

		dpp = digitalPinToPort (rw); // STROBE pin is on RW
		_STB_BIT  = digitalPinToBitMask (rw);
		_STB_PORT = portOutputRegister (dpp);
		_STB_DDR  = portModeRegister   (dpp);
		*_STB_PORT |= _STB_BIT;
		*_STB_DDR  |= _STB_BIT;

		dpp = digitalPinToPort (en); // SCLOCK pin is on EN
		_SCK_BIT  = digitalPinToBitMask (en);
		_SCK_PORT = portOutputRegister (dpp);
		_SCK_DDR  = portModeRegister   (dpp);
		*_SCK_PORT |= _SCK_BIT;
		*_SCK_DDR  |= _SCK_BIT;

		dpp = digitalPinToPort (d0); // RESET pin is on D0
		_RST_BIT  = digitalPinToBitMask (d0);
		_RST_PORT = portOutputRegister (dpp);
		_RST_DDR  = portModeRegister   (dpp);

		if (_reset_pin != NO_RST) { // if we are using reset...
			*_RST_PORT &= ~_RST_BIT; // lower reset pin
			*_RST_DDR |= _RST_BIT; // set it as output (assert reset)
			_delay_us (5000); // delay 5 msec
			*_RST_PORT |= _RST_BIT; // raise reset pin
			*_RST_DDR &= ~_RST_BIT; // set it as input_pullup (de-assert reset)
		}

	} else { // parallel mode

		_serial_mode = 0; // flag "not serial mode"
		_rw_pin = rw; // global copy of r/w

		dpp = digitalPinToPort (rs);
		_RS_BIT = digitalPinToBitMask (rs); // get bitmasks for parallel I/O
		_RS_PORT = portOutputRegister (dpp); // get output ports
		_RS_DDR = portModeRegister (dpp); // get DDR registers
		*_RS_PORT |= _RS_BIT; // initial = high
		*_RS_DDR |= _RS_BIT; // ddr = output

		dpp = digitalPinToPort (rw);
		_RW_BIT = digitalPinToBitMask (rw);
		_RW_PORT = portOutputRegister (dpp);
		_RW_DDR = portModeRegister (dpp);
		*_RW_PORT |= _RW_BIT; // initial = high

		dpp = digitalPinToPort (en);
		_EN_BIT = digitalPinToBitMask (en);
		_EN_PORT = portOutputRegister (dpp);
		_EN_DDR = portModeRegister (dpp);
		*_EN_PORT &= ~_EN_BIT; // initial = low
		*_EN_DDR |= _EN_BIT; // ddr = output

		x = 8;
		while (x--) {
			dpp = digitalPinToPort (data_pin[x]);
			_BIT_MASK[x] = digitalPinToBitMask (data_pin[x]); // get bitmask
			_DATA_DDR[x] = portModeRegister (dpp); // get DDR register
			_DATA_PORT[x] = portOutputRegister (dpp); // get output port register
			*_DATA_DDR[x] |= _BIT_MASK[x]; // set port as output
			if (x == _bit_mode) { // only do 7...4 if 4 bit mode
				break;
			}
		}

		if (_rw_pin != NO_RW) {
			*_RW_DDR |= _RW_BIT;

		} else {
			*_RW_DDR &= ~_RW_BIT;
		}
	}
}

void LiquidCrystal::begin (uint8_t cols, uint8_t rows, uint8_t dotsize)
{
	_numcols = cols;
	_numrows = rows;

	// software reset (Hitachi manual pg. 45 & 46) this is always written
	// in 8 bit mode therefore we use "_transfer8bits" instead of "_send"
	_transfer8bits (LCD_FUNCTIONSET | LCD_8BITMODE);
	_delay_us (5000); // wait more than 4.1ms (pg 45)

	_transfer8bits (LCD_FUNCTIONSET | LCD_8BITMODE);
	_delay_us (200); // wait more than 100 us (pg 45)

	_transfer8bits (LCD_FUNCTIONSET | LCD_8BITMODE);
	_delay_us (200); // wait more than 100 us (pg 45)

	// build setup byte for function set
	_displayFunction = LCD_FUNCTIONSET; // 0x20
	_displayFunction |= (_bit_mode == MODE_4 ? LCD_4BITMODE : LCD_8BITMODE); // set 4 or 8 bit interface
	_displayFunction |= (_numrows == 1 ? LCD_1LINE : LCD_2LINE); // LCD 1 line or more
	_displayFunction |= ((dotsize && (_numrows == 1)) ? LCD_5x10DOTS : LCD_5x8DOTS); // tall bitmap if supported

	_transfer8bits (_displayFunction); // set 4/8 bit mode, lines and font

	///////////////////////////////////////////////////////////////////////
	// from here on we need to use "_send_cmd" to automatically use //
	// the 8 bit or 4 bit interface as setup. //
	// note: busy flag is valid from here after but we don't use it :( //
	///////////////////////////////////////////////////////////////////////
	// build setup byte for cursor move stuff
	_displayEntryMode = LCD_ENTRYMODESET; // 0x04
	_displayEntryMode |= LCD_ENTRYINC; // set cursor to increment
	_displayEntryMode |= LCD_ENTRYNOSHIFT; // no display shift

	_send_cmd (_displayEntryMode); // send command

	// build setup byte for display control
	_displayControl = LCD_DISPLAYCTRL; // 0x08
	_displayControl |= LCD_BLINKOFF; // blink off
	_displayControl |= LCD_CURSOROFF; // underline cursor off

	_send_cmd (_displayControl); // send displaycontrol command

	// build setup byte for cursor control
	_displayCursor = LCD_CURSORSHIFT; // 0x10
	_displayCursor |= LCD_MOVELEFT; // display moves left IF shift enabled
	_displayCursor |= LCD_CURSORMOVE; // cursor moves (display does not shift)

	_send_cmd (_displayCursor); // send displaycursor command
	setBrightness (100); // set default brightness & turn on display
	clearScreen();  // clear screen
	// ready to go from here!
}

void LiquidCrystal::setBrightness (uint8_t percent)
{
	uint8_t _brightness = 0x03;
	uint16_t i = 1000;

	percent = (percent > 100) ? 100 : percent;

	// shut off HV inverter & filament on VFD displays
	// if brightness of "0" is selected.
	setDisplay (percent ? 1 : 0);

	// multiply everything by 10 so fractional
	// numbers are calculated as integers
	while (i && _brightness) {
		if ((percent * 10) > (i + 5)) {
			_brightness--;
		}

		i -= 250;
	}

	_send_cmd (_displayFunction); // execute a function set
	_send_data (_brightness); // set brightness
}

void LiquidCrystal::home (void)
{
	_send_cmd (LCD_RETURNHOME);
	setCursor (0, 0);
	_delay_ms (16); // max 15.2 ms!!!
}

void LiquidCrystal::clear (void)
{
	clearScreen();
}

void LiquidCrystal::clearScreen (void)
{
	_send_cmd (LCD_CLEARDISPLAY);
	setCursor (0, 0);
	_delay_ms (16); // max 15.2 ms!!!
}

void LiquidCrystal::setLine (uint8_t x, uint8_t y)
{
	setCursor (x, y);
}

void LiquidCrystal::setCursor (uint8_t x, uint8_t y)
{
	uint8_t row_offsets[] = {
		0x00, 0x40, 0x14, 0x54,
	};

	if ((x < _numcols) && (y < _numrows)) {
		_cur_x = x; // "real" cursor X position
		_cur_y = y; // "real" cursor Y position
		_send_cmd (LCD_SETDDRAMADDR | (x + row_offsets[y]));
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
	_send_cmd (LCD_DISPLAYCTRL | _displayControl);
}

// Turns the underline cursor on/off
void LiquidCrystal::setUnderline (uint8_t on)
{
	on ? _displayControl |= LCD_CURSORON : _displayControl &= ~LCD_CURSORON;
	_send_cmd (LCD_DISPLAYCTRL | _displayControl);
}

// Turn on and off the blinking cursor
void LiquidCrystal::setBlink (uint8_t on)
{
	on ? _displayControl |= LCD_BLINKON : _displayControl &= ~LCD_BLINKON;
	_send_cmd (LCD_DISPLAYCTRL | _displayControl);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
// Note: "print"ing 0x00 will not work because 0 is end of line.
// use "write" instead.
void LiquidCrystal::createChar (uint8_t location, const uint8_t *charmap)
{
	uint8_t i;
	location &= 0x07; // we only have 8 locations 0-7
	_send_cmd (LCD_SETCGRAMADDR | (location * 8));
	for (i = 0; i < 8; i++) {
		_send_data (* (charmap + i)); // 8 bytes to a char (but only 5 bits)
	}
	home (); // make sure cursor isn't fubar
}

void LiquidCrystal::createChar_P (uint8_t location, const uint8_t *charmap)
{
	// convert 16 bit PROGMEM pointer into a 32 bit address
	createChar_P (location, (const uint32_t) charmap);
}

void LiquidCrystal::createChar_P (uint8_t location, const uint32_t charmap)
{
	uint8_t i;
	location &= 0x07; // we only have 8 locations 0-7
	_send_cmd (LCD_SETCGRAMADDR | (location * 8));
	for (i = 0; i < 8; i++) {
		_send_data (PGM_READ (charmap + i));
	}
	home (); // make sure cursor isn't fubar
}

// private functions
void LiquidCrystal::_send_cmd (uint8_t data)
{
	_send (data, LOW);
}

void LiquidCrystal::_send_data (uint8_t data)
{
	_send (data, HIGH);
}

// dummies for stream compatability
int LiquidCrystal::available (void)
{
	return 0;
}

int LiquidCrystal::peek (void)
{
	return 0;
}

int LiquidCrystal::read (void)
{
	return 0;
}

void LiquidCrystal::flush (void)
{
}

size_t LiquidCrystal::write (uint8_t data)
{
	switch (data) {
		case 0x08: {
			return _backSpace();
		}

		case 0x0a: {
			return _lineFeed();
		}

		case 0x0c: {
			clearScreen();
			return 0;
		}

		case 0x0d: {
			return _carriageReturn();
		}

		default: {
			break;
		}
	}

	_send_data (data);
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
		setCursor (_tmp_x - 1, _tmp_y);
		write (0x20);
		setCursor (_tmp_x - 1, _tmp_y);

	} else {
		// wrap from end -> beginning
		setCursor (_numcols - 1, _tmp_y ? _tmp_y - 1 : _numrows - 1);
		write (0x20);
		setCursor (_numcols - 1, _tmp_y ? _tmp_y - 1 : _numrows - 1);
	}

	return (size_t) - 1;
}

size_t LiquidCrystal::_lineFeed (void)
{
	if ((_cur_y + 1) > _numrows) {
		setCursor (_cur_x, 0); // wrap bottom back to top

	} else {
		setCursor (_cur_x, _cur_y + 1); // cursor down one line
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
			rw ? *_RW_PORT |= _RW_BIT : *_RW_PORT &= ~_RW_BIT;
		}
	}
}

// write either command or data
void LiquidCrystal::_send (uint8_t data, uint8_t rs)
{
	if (_serial_mode) { // set or clear RS bit in serial command byte
		rs ? _serial_cmd |= RSBIT : _serial_cmd &= ~RSBIT;

	} else { // set or clear RS pin (parallel mode)
		rs ? *_RS_PORT |= _RS_BIT : *_RS_PORT &= ~_RS_BIT;
	}

	if (_bit_mode == MODE_4) {
		_transfer4bits (data >> 4); // send top half
		_transfer4bits (data & 0x0F); // send bottom half

	} else {
		_transfer8bits (data); // send all 8 bits
	}
}

// 4 bit mode is only for HD44780 "nibble mode". SPI transfers are always 8 bit.
void LiquidCrystal::_transfer4bits (uint8_t data)
{
	uint8_t n = 4; // bit count (parallel mode only)
	_setRW (_WRITE); // set pin to "write"
	while (n--) { // 4 bits parallel
		(data & _BV (n)) ? *_DATA_PORT[n + 4] |= _BIT_MASK[n + 4] : *_DATA_PORT[n + 4] &= ~_BIT_MASK[n + 4];
	}
	*_EN_PORT |= _EN_BIT; // assert "enable" (strobe in the data nibble)
	_delay_ns (500); // min 450 ns (hd44780u datasheet pg. 49)
	*_EN_PORT &= ~_EN_BIT; // de-assert "enable"
	_setRW (_READ); // set pin to "read"
}

void LiquidCrystal::_transfer8bits (uint8_t data)
{
	uint8_t n = 8; // bit count (parallel mode only)
	_setRW (_WRITE); // set bit or pin to "write" as appropriate
	if (_serial_mode) {
		_serial_IO ((_serial_cmd << 8) | data); // send command & data as one 16 bit packet
	} else {
		while (n--) { // 8 bits parallel
			(data & _BV (n)) ? *_DATA_PORT[n] |= _BIT_MASK[n] : *_DATA_PORT[n] &= ~_BIT_MASK[n];
		}
		*_EN_PORT |= _EN_BIT; // assert "enable" (strobe in the data byte)
		_delay_ns (500); // min 450 ns (hd44780u datasheet pg. 49)
		*_EN_PORT &= ~_EN_BIT; // de-assert "enable"
	}
	_setRW (_READ); // set bit or pin to "read" as appropriate
}

// serial I/O for Noritake CUU displays
// this is like SPI, but using only one pin for both read
// and write. currently received data isn't used anywhere
uint8_t LiquidCrystal::_serial_IO (uint16_t data)
{
	uint8_t bits = 16;
	*_STB_PORT &= ~_STB_BIT; // assert strobe (chip select)
	while (bits--) { // write out bits 15 thru 0
		*_SCK_PORT &= ~_SCK_BIT; // sck low
		*_SIO_DDR |= _SIO_BIT; // siso DDR as output
		data & _BV (bits) ? *_SIO_PORT |= _SIO_BIT : *_SIO_PORT &= ~_SIO_BIT; // send 0 or 1 as appropriate
		*_SCK_PORT |= _SCK_BIT; // sck high
		*_SIO_DDR &= ~_SIO_BIT; // siso DDR as input
		*_SIO_PIN ? data |= _BV (bits) : data &= ~_BV (bits); // read the pin
	}
	*_STB_PORT |= _STB_BIT; // de-assert strobe (chip select)
	// we always leave the data line as an input upon exit
	return data;
}

// end of LiquidCrystal.cpp
