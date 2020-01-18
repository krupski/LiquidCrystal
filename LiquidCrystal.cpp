///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD/VFD) driver library
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//  Copyright (c) 2019 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 22 October 2019
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#include "LiquidCrystal.h"

// serial interface, without reset
LiquidCrystal::LiquidCrystal ( // 3
	uint8_t siso, uint8_t stb, uint8_t sck
) {
	init (MODE_S, siso, stb, sck, 0, 0, 0, 0, 0, 0, 0, 0);
}

// parallel interface 4 bits without R/W
LiquidCrystal::LiquidCrystal ( // 6
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_4, rs, NO_RW, en, 0, 0, 0, 0, d4, d5, d6, d7);
}

// parallel interface 4 bits with R/W
LiquidCrystal::LiquidCrystal ( // 7
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_4, rs, rw, en, 0, 0, 0, 0, d4, d5, d6, d7);
}

// parallel interface 8 bits without R/W
LiquidCrystal::LiquidCrystal ( // 10
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_8, rs, NO_RW, en, d0, d1, d2, d3, d4, d5, d6, d7);
}

// parallel interface 8 bits with R/W
LiquidCrystal::LiquidCrystal ( // 11
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_8, rs, rw, en, d0, d1, d2, d3, d4, d5, d6, d7);
}

void LiquidCrystal::init ( // 12
	uint8_t bitmode,
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {

	uint8_t n, x;

	_bit_mode = bitmode; // 4 bit (0x04), 8 bit (0x08) or serial (0xFF) mode flag

	if (_bit_mode == MODE_S) { // 0xFF == serial mode

		_bit_mode = MODE_8; // reset it to 8 bit mode
		_serial_mode = 1; // flag "we are in serial mode"

		// serial command byte template (Noritake CU20049-UW2J manual pg. 12)
		// bit [7...3] = 1
		// bit [2] = read/write (1=read,0=write)
		// bit [1] = register select (1=data,0=command)
		// bit [0] = 0
		_serial_cmd = ((_RSBIT|_RWBIT|_SYNC));

		n = digitalPinToPort (rs); // SISO pin is on RS
		_SIO_BIT = digitalPinToBitMask (rs);
		_SIO_PIN = portInputRegister (n);
		_SIO_PORT = portOutputRegister (n);
		_SIO_DDR = portModeRegister (n);
		*_SIO_PORT |= _SIO_BIT; // default pullup enabled
		*_SIO_DDR &= ~_SIO_BIT; // default SISO is input

		n = digitalPinToPort (rw); // STROBE pin is on RW
		_STB_BIT = digitalPinToBitMask (rw);
		_STB_PORT = portOutputRegister (n);
		_STB_DDR = portModeRegister (n);
		*_STB_PORT |= _STB_BIT; // default STROBE not asserted
		*_STB_DDR |= _STB_BIT; // default STROBE is output

		n = digitalPinToPort (en); // SCLOCK pin is on EN
		_SCK_BIT = digitalPinToBitMask (en);
		_SCK_PORT = portOutputRegister (n);
		_SCK_DDR = portModeRegister (n);
		*_SCK_PORT |= _SCK_BIT; // default SCK idles high
		*_SCK_DDR |= _SCK_BIT; // default SCK pin is output

	} else { // parallel mode

		const uint8_t data_pin[] = {
			d0, d1, d2, d3, d4, d5, d6, d7
		};

		_serial_mode = 0; // flag "not serial mode"
		_rw_pin = rw; // global copy of r/w

		n = digitalPinToPort (rs); // register select
		_RS_BIT = digitalPinToBitMask (rs); // get bitmasks for parallel I/O
		_RS_PORT = portOutputRegister (n); // get output ports
		_RS_DDR = portModeRegister (n); // get DDR registers
		*_RS_PORT |= _RS_BIT; // default RS is "data"
		*_RS_DDR |= _RS_BIT; // default RS is output

		n = digitalPinToPort (en); // enable
		_EN_BIT = digitalPinToBitMask (en);
		_EN_PORT = portOutputRegister (n);
		_EN_DDR = portModeRegister (n);
		*_EN_PORT &= ~_EN_BIT; // default ENABLE not asserted
		*_EN_DDR |= _EN_BIT; // default ENABLE is output

		if (_rw_pin != NO_RW) { // read/write
			n = digitalPinToPort (rw);
			_RW_BIT = digitalPinToBitMask (rw);
			_RW_PORT = portOutputRegister (n);
			_RW_DDR = portModeRegister (n);
			*_RW_PORT &= ~_RW_BIT; // default R/W is "write"
			*_RW_DDR |= _RW_BIT; // default R/W is output
		}

		x = 8;

		while (x--) { // set PARALLEL DDR (4 or 8 bits)
			n = digitalPinToPort (data_pin[x]);
			_BIT_MASK[x] = digitalPinToBitMask (data_pin[x]); // get bitmask
			_DATA_DDR[x] = portModeRegister (n); // get DDR register
			_DATA_PORT[x] = portOutputRegister (n); // get output port register
			*_DATA_DDR[x] |= _BIT_MASK[x]; // data bits = output
			// if we are in 4 bit mode then only set d7...d4
			if (x == _bit_mode) {
				break;
			}
		}
	}

	begin (); // default LCD setup
}

void LiquidCrystal::begin (uint8_t cols, uint8_t rows, uint8_t dotsize)
{
	uint8_t x;

	_num_cols = cols;
	_num_rows = rows;

	// setup default DDRAM offsets
	setRowOffsets (0x00, 0x40, 0x14, 0x54);
//  use this one if LCD/VFD lines 2 and 3 don't line up properly
//	setRowOffsets (0x00, 0x40, 0x10, 0x50);

	// we need at least 40ms after power rises above 2.7V before sending commands.
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 50));

	x = _bit_mode; // save actual bitmode
	_bit_mode = MODE_8; // 8 bit

	// build _displayMode template
	// default: increment mode, no shift
	_displayMode = (ENTRYMODESET | INCREMENT);

	// build _displayControl template
	// default: display off, cursor off, blink off
	_displayControl = (DISPLAYCTRL);

	// build _displayCursor template
	// default: cursor move, cursor moves right
	_displayCursor = (CURSORSHIFT | MOVERIGHT);

	// build _displayFunction template
	// default: 8 bit, 1 line, 5 x 8 character
	_displayFunction = (FUNCTIONSET | BITMODE8);

	// send reset sequence
	_sendCmd (_displayFunction);
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 5)); // wait more than 4.1 ms

	_sendCmd (_displayFunction);
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 200)); // wait more than 100 us

	_sendCmd (_displayFunction);
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 200)); // wait more than 100 us

	if (x == MODE_4) { // if actual bitmode is 4 then clear the 8 bit flag
		_displayFunction &= ~BITMODE8;
	}

	if (_num_rows > 1) {
		_displayFunction |= LINES2;
	}

	if (dotsize) {
		_displayFunction |= DOTS5X10;
	}

	// finish display reset
	_sendCmd (_displayFunction); // set the interface bit mode
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 200));

	_bit_mode = x; // now driver uses 4 or 8 bits

	_sendCmd (_displayMode); // entry mode set
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 200));

	clearScreen();  // clear display
	setDisplay (1); // turn display on
}

// this only works with a VFD
void LiquidCrystal::setBrightness (uint8_t pct)
{
	uint8_t brite = 0x03;
	uint16_t x = 1000;

	// constrain percent
	pct = (pct > 100) ? 100 : pct;

	// shut off HV inverter & filament on VFD displays
	// if brightness of "0" is selected.
	if (!pct) {
		setDisplay (0);
		return;

	} else {
		setDisplay (1);
	}

	// multiply everything by 10 so fractional
	// numbers are calculated as integers
	while (x && brite) {
		if ((pct * 10) > (x + 5)) {
			brite--;
		}

		x -= (1000 / 4); // 4 brightness steps
	}

	// execute a function set
//	_sendCmd (FUNCTIONSET);
	_sendCmd (_displayFunction);
	// send the brightness control bits - 0b00:100%, 0b01:75%, 0b10:50%, 0b11:25%
	_sendData (brite); // set brightness (VFD only)
}

void LiquidCrystal::home (void)
{
	_sendCmd (RETURNHOME);
	// wait more than 1.52 ms
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 5));
	setCursor (0, 0);
}

void LiquidCrystal::clearScreen (void)
{
	clear();
}

void LiquidCrystal::clear (void)
{
	_sendCmd (CLEARDISPLAY);
	// wait more than 1.52 ms
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 5));
	setCursor (0, 0);
}

// if using a 16x4 LCD/VFD and line 3 & 4 are not placed correctly try:
//    this ---> setRowOffsets(0x00, 0x40, 0x14, 0x54);
// or this ---> setRowOffsets(0x00, 0x40, 0x10, 0x50);
void LiquidCrystal::setRowOffsets (uint8_t row0, uint8_t row1, uint8_t row2, uint8_t row3)
{
	_row_offsets[0] = row0;
	_row_offsets[1] = row1;
	_row_offsets[2] = row2;
	_row_offsets[3] = row3;
}

void LiquidCrystal::setLine (double x, double y)
{
	setCursor (x, y);
}

void LiquidCrystal::getLine (double &x, double &y)
{
	x = (double)(_cur_x);
	y = (double)(_cur_y);
}

void LiquidCrystal::setCursor (uint8_t x, uint8_t y)
{
	_cur_x = x; // record cursor X pos
	_cur_y = y; // record cursor Y pos
	_sendCmd (SETDDRAMADDR | (_cur_x + _row_offsets[_cur_y]));
}

void LiquidCrystal::getCursor (uint8_t &x, uint8_t &y)
{
	x = _cur_x;
	y = _cur_y;
}

void LiquidCrystal::pushCursor (void)
{
	getCursor (_save_x, _save_y);
}

void LiquidCrystal::popCursor (void)
{
	setCursor (_save_x, _save_y);
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

void LiquidCrystal::noUnderline (void)
{
	setUnderline (0);
}

void LiquidCrystal::underline (void)
{
	setUnderline (1);
}

// This will 'left justify' text from the cursor
void LiquidCrystal::noAutoscroll (void)
{
	setAutoscroll (0);
}

// This will 'right justify' text from the cursor
void LiquidCrystal::autoscroll (void)
{
	setAutoscroll (1);
}

// Turn the display on/off (quickly)
// NOTE: Also shuts off the cathode!
void LiquidCrystal::setDisplay (uint8_t on)
{
	on ? _displayControl |= DISPLAYON : _displayControl &= ~DISPLAYON;
	_sendCmd (_displayControl);
}

// Turns the underline cursor on/off
void LiquidCrystal::setUnderline (uint8_t on)
{
	on ? _displayControl |= CURSORON : _displayControl &= ~CURSORON;
	_sendCmd (_displayControl);
}

// Turn on and off the blinking cursor
void LiquidCrystal::setBlink (uint8_t on)
{
	on ? _displayControl |= BLINKON : _displayControl &= ~BLINKON;
	_sendCmd (_displayControl);
}

void LiquidCrystal::setAutoscroll (uint8_t on)
{
	on ? _displayMode |= DISPLAYSHIFT : _displayMode &= ~DISPLAYSHIFT;
	_sendCmd (_displayMode);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal::scrollDisplayLeft (void)
{
	_displayCursor |= (CURSORSHIFT | DISPLAYMOVE | MOVERIGHT);
	_displayCursor &= ~MOVERIGHT;
	_sendCmd (_displayCursor);
}

void LiquidCrystal::scrollDisplayRight (void)
{
	_displayCursor |= (CURSORSHIFT | DISPLAYMOVE | MOVERIGHT);
	_sendCmd (_displayCursor);
}

// This is for text that flows Left to Right
void LiquidCrystal::leftToRight (void)
{
	_displayMode |= INCREMENT;
	_sendCmd (_displayMode);
}

// This is for text that flows Right to Left
void LiquidCrystal::rightToLeft (void)
{
	_displayMode &= ~INCREMENT;
	_sendCmd (_displayMode);
}

// custom bitmaps in SRAM
void LiquidCrystal::createChar (uint8_t addr, const char *bitmap)
{
	createChar (addr, (const uint8_t *)(bitmap));
}

// custom bitmaps in SRAM
void LiquidCrystal::createChar (uint8_t addr, const uint8_t *bitmap)
{
	uint8_t n;

	_clearChar (addr); // erase old LCD/VFD data

	_sendCmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_sendData (bitmap[n]); // 8 bytes to a char (but only 5 bits)
	}

	home();  // make sure cursor isn't fubar
}

// custom bitmaps in PROGMEM
void LiquidCrystal::createChar_P (uint8_t addr, const char *bitmap)
{
	createChar_P (addr, (const uint8_t *)(bitmap));
}

void LiquidCrystal::createChar_P (uint8_t addr, const uint8_t *bitmap)
{
	uint8_t n;

	_clearChar (addr); // erase old LCD/VFD data

	_sendCmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_sendData (pgm_read_byte (bitmap + n));
	}

	home();  // make sure cursor isn't fubar
}

// custom bitmaps in EEPROM
void LiquidCrystal::createChar_E (uint8_t addr, const char *bitmap)
{
	createChar_E (addr, (const uint8_t *)(bitmap));
}

// custom bitmaps in EEPROM
void LiquidCrystal::createChar_E (uint8_t addr, const uint8_t *bitmap)
{
	uint8_t n;

	_clearChar (addr); // erase old LCD/VFD data

	_sendCmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_sendData (eeprom_read_byte ((const uint8_t *)(bitmap + n)));
	}

	home();  // make sure cursor isn't fubar
}

size_t LiquidCrystal::write (int c)
{
	// support non-uint8_t writes
	return write ((uint8_t)(c));
}

size_t LiquidCrystal::write (uint8_t c)
{
	switch (c) {

		default: {
			_sendData (c);

			if (_cur_x < (_num_cols - 1)) { // if next col pos isn't at end
				_cur_x++;

			} else {
				_cur_x = 0;

				if (_cur_y < (_num_rows - 1)) { // need new row
					_cur_y++;

				} else {
					_cur_x = 0;
					_cur_y = 0;
				}
			}

			setCursor (_cur_x, _cur_y);
			break;
		}

		case '\b': {
			return _backSpace();
		}

		case '\t': {
			return _doTabs (4);
		}

		case '\n': {
			return _lineFeed();
		}

		case '\f': {
			clearScreen();
			return 0;
		}

		case '\r': {
			return _carriageReturn();
		}
	}

	return ((size_t)(1));
}

void LiquidCrystal::_clearChar (uint8_t addr)
{
	uint8_t n;

	_sendCmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_sendData (0); // erase old
	}
}

size_t LiquidCrystal::_backSpace (void)
{
	uint8_t _tmp_x = _cur_x;
	uint8_t _tmp_y = _cur_y;

	if (_tmp_x) {
		_tmp_x--;

	} else {
		_tmp_x = (_num_cols - 1);

		if (_tmp_y) {
			_tmp_y--;

		} else {
			_tmp_y = (_num_rows - 1);
		}
	}

	setCursor (_tmp_x, _tmp_y);
	write ((uint8_t)(0x20));
	setCursor (_tmp_x, _tmp_y);

	return 0;
}

size_t LiquidCrystal::_lineFeed (void)
{
	if (_cur_y < (_num_rows - 1)) {
		_cur_y++;

	} else {
		_cur_y = 0;
	}

	setCursor (_cur_x, _cur_y);
	return  0;
}

size_t LiquidCrystal::_carriageReturn (void)
{
	setCursor (0, _cur_y);
	return 0;
}

// move cursor to next tab stop (4 places = 1 "tab")
size_t LiquidCrystal::_doTabs (uint8_t _tab_size)
{
	size_t n = 0;

	if (! (_cur_x % _tab_size)) {
		n += write ((uint8_t)(0x20));
	}

	while (_cur_x % _tab_size) {
		n += write ((uint8_t)(0x20));
	}

	return n;
}

void LiquidCrystal::_sendCmd (uint8_t cmd)
{
	_send (cmd, _CMD); // rs = low
	// wait more than 37 us because we can't check busy
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 50));
}

void LiquidCrystal::_sendData (uint8_t dat)
{
	_send (dat, _DATA); // rs = high
	// wait more than 37 us because we can't check busy
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 50));
}

// write either command or data determined by rs (register select)
// if rs = 1 then we are transferring text or command param data
// if rs = 0 then we are sending an HD44780 command
// PARALLEL mode: R/W is set to write if defined, else
//   it must pulled low MANUALLY
// SERIAL mode: R/W is controlled by a command bit and it is set low.
void LiquidCrystal::_send (uint8_t c, uint8_t rs)
{
	if (_serial_mode) { // set or clear RS bit in serial command byte
		*_SIO_DDR |= _SIO_BIT; // set SIO pin as output
		rs ? _serial_cmd |= _RSBIT : _serial_cmd &= ~_RSBIT; // register select bit
		_serial_cmd &= ~_RWBIT; // write mode
		*_STB_PORT &= ~_STB_BIT; // assert strobe
		_serialSend (_serial_cmd); // send command via serial
		_serialSend (c); // send data via serial
		*_STB_PORT |= _STB_BIT; // de-assert strobe
		*_SIO_DDR &= ~_SIO_BIT; // set SIO pin as input

	} else {
		rs ? *_RS_PORT |= _RS_BIT : *_RS_PORT &= ~_RS_BIT; // register select bit

		if (_bit_mode == MODE_4) {
			_send4bits (c >> 4); // send top half of byte
			_send4bits (c & 0x0F); // send bottom half of byte

		} else {
			_send8bits (c); // send command or data via parallel
		}
	}
}

// parallel 4 bit mode (we send top 4 bits, then bottom 4)
void LiquidCrystal::_send4bits (uint8_t c)
{
	uint8_t n = 4; // bit count

	while (n--) { // 4 bits parallel
		(c & (1 << n)) ? *_DATA_PORT[(n+4)] |= _BIT_MASK[(n+4)] : *_DATA_PORT[(n+4)] &= ~_BIT_MASK[(n+4)];
	}

	*_EN_PORT |= _EN_BIT; // pulse enable high...
	__builtin_avr_delay_cycles (F_CPU / _USEC); // ...for 1 usec...
	*_EN_PORT &= ~_EN_BIT; // ...latches data
}

// parallel 8 bit mode (we send all 8 bits at once)
void LiquidCrystal::_send8bits (uint8_t c)
{
	uint8_t n = 8; // bit count

	while (n--) { // 8 bits parallel
		(c & (1 << n)) ? *_DATA_PORT[n] |= _BIT_MASK[n] : *_DATA_PORT[n] &= ~_BIT_MASK[n];
	}

	*_EN_PORT |= _EN_BIT; // pulse enable high...
	__builtin_avr_delay_cycles (F_CPU / _USEC); // ...for 1 usec...
	*_EN_PORT &= ~_EN_BIT; // ...latches data
}

void LiquidCrystal::_serialSend (uint8_t c)
{
	uint8_t n = 8;

	while (n--) {
		__builtin_avr_delay_cycles (F_CPU / (_USEC / 5));
		*_SCK_PORT &= ~_SCK_BIT; // set sck low
		(c & (1 << n)) ? *_SIO_PORT |= _SIO_BIT : *_SIO_PORT &= ~_SIO_BIT; // write bit
		*_SCK_PORT |= _SCK_BIT; // set sck high
	}
}

// end of LiquidCrystal.cpp
