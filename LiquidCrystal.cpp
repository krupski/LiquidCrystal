///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD) driver library
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//  Copyright (c) 2015 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 02 August 2015
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

// SPI serial interface, hardware reset not available
LiquidCrystal::LiquidCrystal (
	uint8_t siso, uint8_t stb, uint8_t sck
) {
	init (MODE_S, siso, stb, sck, NO_RST, 0, 0, 0, 0, 0, 0, 0, 0);
}

// SPI serial interface, hardware reset is enabled and available (D0 pin is used for reset)
LiquidCrystal::LiquidCrystal (
	uint8_t siso, uint8_t stb, uint8_t sck, uint8_t reset
) {
	init (MODE_S, siso, stb, sck, reset, 0, 0, 0, 0, 0, 0, 0, 0);
}

// parallel interface 4 bits without active r/w (must tie r/w low manually)
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_4, rs, NO_RW, en, 0, 0, 0, 0, d4, d5, d6, d7, 0);
}

// parallel interface 4 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_4, rs, rw, en, 0, 0, 0, 0, d4, d5, d6, d7, 0);
}

// parallel interface 4 bits with active r/w and active reset
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
	uint8_t v0
) {
	init (MODE_4, rs, rw, en, 0, 0, 0, 0, d4, d5, d6, d7, v0);
}

// parallel interface 8 bits without active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_8, rs, NO_RW, en, d0, d1, d2, d3, d4, d5, d6, d7, 0);
}

// parallel interface 8 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) {
	init (MODE_8, rs, rw, en, d0, d1, d2, d3, d4, d5, d6, d7, 0);
}

// parallel interface 8 bits with active r/w and active reset
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
	uint8_t v0
) {
	init (MODE_8, rs, rw, en, d0, d1, d2, d3, d4, d5, d6, d7, v0);
}

void LiquidCrystal::init (
	uint8_t bitmode, uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
	uint8_t v0
) {

	uint8_t n, x;

	_bit_mode = bitmode; // 4 bit (0x04), 8 bit (0x08) or serial (0xFF) mode flag

	if (_bit_mode == MODE_S) { // 0xFF == serial mode

		_bit_mode = MODE_8; // reset it to 8 bit mode
		_serial_mode = 1; // flag "we are in serial mode"
		_reset_pin = d0; // alternate use of pin

		// template for serial command. note that bit 1 (RS) and bit 2 (RW)
		// are dynamically set or cleared as required by the driver itself.
		_serial_cmd = (0b11111110);
		_serial_cmd &= ~RSBIT; // initial setting RS = LOW = command
		_serial_cmd &= ~RWBIT; // initial setting RW = LOW = write

		n = digitalPinToPort (rs); // SISO pin is on RS
		_SIO_BIT = digitalPinToBitMask (rs);
		_SIO_PIN = portInputRegister (n);
		_SIO_PORT = portOutputRegister (n);
		_SIO_DDR = portModeRegister (n);
		*_SIO_PORT |= _SIO_BIT; // set SISO pin high
		*_SIO_DDR &= ~_SIO_BIT; // SISO pin is in "input_pullup" mode

		n = digitalPinToPort (rw); // STROBE pin is on RW
		_STB_BIT = digitalPinToBitMask (rw);
		_STB_PORT = portOutputRegister (n);
		_STB_DDR = portModeRegister (n);
		*_STB_PORT |= _STB_BIT;
		*_STB_DDR |= _STB_BIT;

		n = digitalPinToPort (en); // SCLOCK pin is on EN
		_SCK_BIT = digitalPinToBitMask (en);
		_SCK_PORT = portOutputRegister (n);
		_SCK_DDR = portModeRegister (n);
		*_SCK_PORT |= _SCK_BIT;
		*_SCK_DDR |= _SCK_BIT;

		if (_reset_pin != NO_RST) { // if we are using reset...
			n = digitalPinToPort (d0); // RESET pin is on D0
			_RST_BIT = digitalPinToBitMask (d0);
			_RST_PORT = portOutputRegister (n);
			_RST_DDR = portModeRegister (n);
			*_RST_PORT &= ~_RST_BIT; // lower reset pin
			*_RST_DDR |= _RST_BIT; // set it as output (assert reset)
			_delay_usec (1000); // delay 1 msec
			*_RST_PORT |= _RST_BIT; // raise reset pin
			*_RST_DDR &= ~_RST_BIT; // set it as input_pullup (de-assert reset)
		}

	} else { // parallel mode

		static const uint8_t data_pin[] = {
			d0, d1, d2, d3, d4, d5, d6, d7
		};

		_serial_mode = 0; // flag "not serial mode"
		_rw_pin = rw; // global copy of r/w
		_reset_pin = v0; // alternate use of pin

		n = digitalPinToPort (rs);
		_RS_BIT = digitalPinToBitMask (rs); // get bitmasks for parallel I/O
		_RS_PORT = portOutputRegister (n); // get output ports
		_RS_DDR = portModeRegister (n); // get DDR registers
		*_RS_PORT &= ~_RS_BIT; // initial setting RS = LOW = command
		*_RS_DDR |= _RS_BIT; // ddr = output

		n = digitalPinToPort (en);
		_EN_BIT = digitalPinToBitMask (en);
		_EN_PORT = portOutputRegister (n);
		_EN_DDR = portModeRegister (n);
		*_EN_PORT &= ~_EN_BIT; // initial = low
		*_EN_DDR |= _EN_BIT; // ddr = output

		if (_rw_pin != NO_RW) {
			n = digitalPinToPort (rw);
			_RW_BIT = digitalPinToBitMask (rw);
			_RW_PORT = portOutputRegister (n);
			_RW_DDR = portModeRegister (n);
			*_RW_PORT &= ~_RW_BIT; // initial setting RW = LOW = write
			*_RW_DDR |= _RW_BIT; // ddr = output
		}

		if (_reset_pin) { // if we are using reset...
			n = digitalPinToPort (v0); // RESET pin is on v0
			_RST_BIT = digitalPinToBitMask (v0);
			_RST_PORT = portOutputRegister (n);
			_RST_DDR = portModeRegister (n);
			*_RST_PORT &= ~_RST_BIT; // lower reset pin
			*_RST_DDR |= _RST_BIT; // set it as output (assert reset)
			_delay_usec (1000); // delay 1 msec
			*_RST_PORT |= _RST_BIT; // raise reset pin
			*_RST_DDR &= ~_RST_BIT; // set it as input_pullup (de-assert reset)
		}

		x = 8;
		while (x--) {
			n = digitalPinToPort (data_pin[x]);
			_BIT_MASK[x] = digitalPinToBitMask (data_pin[x]); // get bitmask
			_DATA_DDR[x] = portModeRegister (n); // get DDR register
			_DATA_PORT[x] = portOutputRegister (n); // get output port register
			*_DATA_PORT[x] &= ~_BIT_MASK[x]; // set port low
			*_DATA_DDR[x] |= _BIT_MASK[x]; // set port as output

			// if we are in 4 bit mode then only enable d7...d4 as outputs
			if (x == _bit_mode) {
				break;
			}
		}
	}

	begin (16, 1);
}

void LiquidCrystal::begin (uint8_t cols, uint8_t rows, uint8_t dotsize)
{
	uint8_t x;

	_numcols = cols;
	_numrows = rows;

	// setup default DDRAM offsets
	setRowOffsets (0x00, 0x40, 0x14, 0x54);

	_displayFunction = 0; // init template

	// build the _displayFunction template
	if (_bit_mode == MODE_4) {
		_displayFunction |= (LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS);

	} else {
		_displayFunction |= (LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS);
	}

	if ((dotsize != LCD_5x8DOTS) && (_numrows == 1)) {
		_displayFunction |= LCD_5x10DOTS;
	}

	if (_numrows > 1) {
		_displayFunction |= LCD_2LINE;
	}

	// we need at least 40ms after power rises above 2.7V before sending
	// commands. Arduino can turn on way before 4.5V so we'll wait 50
	_delay_usec (50000);

	x = _bit_mode; // save actual bitmode

	// send initialization in 8 bit mode via _send_cmd
	if (_bit_mode == MODE_4) {
		_bit_mode = MODE_8; // force reset to be an 8 bit write
	}

	// send reset sequence
	_send_cmd (LCD_FUNCTIONSET | LCD_8BITMODE);
	_delay_usec (4500); // wait min 4.1 ms

	_send_cmd (LCD_FUNCTIONSET | LCD_8BITMODE);
	_delay_usec (4500); // wait min 4.1 ms

	_send_cmd (LCD_FUNCTIONSET | LCD_8BITMODE);
	_delay_usec (125); // wait min 100 us

	// finish display reset
	_send_cmd (LCD_FUNCTIONSET | _displayFunction);

	_bit_mode = x; // restore actual bitmode

	///////////////////////////////////////////////////////////////////
	/////////// from here on _send_cmd auto-selects bitmode ///////////
	///////////////////////////////////////////////////////////////////

	// build _displayControl template
	_displayControl = 0; // init template
	_displayControl |= (LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
	_send_cmd (LCD_DISPLAYCONTROL | _displayControl);

	// build _displayEntryMode template
	_displayEntryMode = 0; // init template
	_displayEntryMode |= LCD_ENTRYLEFT | LCD_ENTRYSHIFTDEC;
	_send_cmd (LCD_ENTRYMODESET | _displayEntryMode);

	setBrightness (100);
	// we're ready to go!
	clearScreen ();
}

void LiquidCrystal::setBrightness (uint8_t pct)
{
	uint8_t brite = 0b11;
	uint16_t x = 1000;

	// constrain percent
	pct = (pct > 100) ? 100 : pct;

	// shut off HV inverter & filament on VFD displays
	// if brightness of "0" is selected.
	setDisplay (pct ? 1 : 0);

	// multiply everything by 10 so fractional
	// numbers are calculated as integers
	while (x && brite) {
		if ((pct * 10) > (x + 5)) {
			brite--;
		}

		x -= (1000 / 4); // 4 brightness steps
	}

	// execute a function set
	_send_cmd (LCD_FUNCTIONSET | _displayFunction);
	// send the brightness control bits - 0b00:100%, 0b01:75%, 0b10:50%, 0b11:25%
	_send_data (brite); // set brightness (VFD only)
}

void LiquidCrystal::home (void)
{
	_send_cmd (LCD_RETURNHOME);
	_delay_usec (20000); // min 15.2 ms!!!
	setCursor (0, 0);
}

void LiquidCrystal::clearScreen (void)
{
	clear ();
}

void LiquidCrystal::clear (void)
{
	_send_cmd (LCD_CLEARDISPLAY);
	_delay_usec (20000); // min 15.2 ms!!!
	setCursor (0, 0);
}

// if using a 16x4 LCD and line 3 & 4 are not placed correctly try:
//    this ---> setRowOffsets(0x00, 0x40, 0x14, 0x54);
// or this ---> setRowOffsets(0x00, 0x40, 0x10, 0x50);
void LiquidCrystal::setRowOffsets (uint8_t row0, uint8_t row1, uint8_t row2, uint8_t row3)
{
	_row_offsets[0] = row0;
	_row_offsets[1] = row1;
	_row_offsets[2] = row2;
	_row_offsets[3] = row3;
}

void LiquidCrystal::setLine (uint8_t x, uint8_t y)
{
	setCursor (x, y);
}

void LiquidCrystal::setCursor (uint8_t x, uint8_t y)
{
	_cur_x = (x % _numcols); // record cursor X pos
	_cur_y = (y % _numrows); // record cursor Y pos

	_send_cmd (LCD_SETDDRAMADDR | (_cur_x + _row_offsets[_cur_y]));
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

void LiquidCrystal::setAutoscroll (uint8_t on)
{
	on ? _displayEntryMode |= LCD_ENTRYSHIFTINC : _displayEntryMode &= ~LCD_ENTRYSHIFTINC;
	_send_cmd (LCD_ENTRYMODESET | _displayEntryMode);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal::scrollDisplayLeft (void)
{
	_send_cmd (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LiquidCrystal::scrollDisplayRight (void)
{
	_send_cmd (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal::leftToRight (void)
{
	_displayEntryMode |= LCD_ENTRYLEFT;
	_send_cmd (LCD_ENTRYMODESET | _displayEntryMode);
}

// This is for text that flows Right to Left
void LiquidCrystal::rightToLeft (void)
{
	_displayEntryMode &= ~LCD_ENTRYLEFT;
	_send_cmd (LCD_ENTRYMODESET | _displayEntryMode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
// Note: "print"ing 0x00 will not work because 0 is end of line.
// use "write" instead.
void LiquidCrystal::createChar (uint8_t location, const uint8_t *charmap)
{
	uint8_t n;
	location &= 0x07; // we only have 8 locations 0-7

	_send_cmd (LCD_SETCGRAMADDR | (location * 8));

	for (n = 0; n < 8; n++) {
		_send_data (*(charmap + n)); // 8 bytes to a char (but only 5 bits)
	}

	home();  // make sure cursor isn't fubar
}

void LiquidCrystal::createChar_P (uint8_t location, const uint8_t *charmap)
{
	// convert 16 bit PROGMEM pointer into a 32 bit address
	createChar_P (location, (const uint32_t) charmap);
}

void LiquidCrystal::createChar_P (uint8_t location, const uint32_t charmap)
{
	uint8_t n;
	location &= 0x07; // we only have 8 locations 0-7

	_send_cmd (LCD_SETCGRAMADDR | (location * 8));

	for (n = 0; n < 8; n++) {
		_send_data (PGM_READ (charmap + n));
	}

	home();  // make sure cursor isn't fubar
}

// private functions
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
	// void so return nothing
}

LiquidCrystal::operator bool()
{
	return true;
}

///////////////////////////////////////////////
//     note 0x00...0x07 are custom chars     //
///////////////////////////////////////////////
size_t LiquidCrystal::write (uint8_t data)
{
	switch (data) {

		case 0x08: {
			return _backSpace ();
		}

		case 0x09: {
			return _doTabs (4);
			break;
		}

		case 0x0A: {
			return _lineFeed ();
		}

		case 0x0C: {
			clearScreen();
			return 0;
		}

		case 0x0D: {
			return _carriageReturn ();
		}

		default: {
			break; // must be printable - fall through and do it
		}
	}

	_send_data (data);
	_cur_x++;

	if (_cur_x < _numcols) { // if next col pos isn't at end
		// nothing
	} else {
		_cur_x = 0;
		_cur_y++;
		if (_cur_y < _numrows) { // need new row
			// nothing
		} else {
			_cur_x = 0;
			_cur_y = 0;
		}
	}

	setCursor (_cur_x, _cur_y);

	return 1;
}

void LiquidCrystal::_delay_usec (uint32_t delay)
{
	// should be 16 nops, but is only 9 to
	// account for loop counting overhead.
	while (delay--) {
		__asm__ __volatile__ (
			" nop\n" " nop\n" " nop\n"
			" nop\n" " nop\n" " nop\n"
			" nop\n" " nop\n" " nop\n"
		);
	}
}

size_t LiquidCrystal::_backSpace (void)
{
	uint8_t _tmp_x, _tmp_y;

	_tmp_x = _cur_x;
	_tmp_y = _cur_y;

	if (_tmp_x) {
		_tmp_x--;

	} else {
		_tmp_x = (_numcols - 1);

		if (_tmp_y) {
			_tmp_y--;
		} else {
			_tmp_y = (_numrows - 1);
		}
	}

	setCursor (_tmp_x, _tmp_y);
	write (0x20);
	setCursor (_tmp_x, _tmp_y);

	return 0;
}

size_t LiquidCrystal::_lineFeed (void)
{
	_cur_y++;
	if (_cur_y > _numrows) {
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
		n += write (0x20);
	}

	while (_cur_x % _tab_size) {
		n += write (0x20);
	}

	return n;
}

void LiquidCrystal::_send_cmd (uint8_t data)
{
	_send (data, _CMD); // rs = low
}

void LiquidCrystal::_send_data (uint8_t data)
{
	_send (data, _DATA); // rs = high
}

// write either command or data determined by rs (register select)
// if rs = 1 then we are transferring text or command param data
// if rs = 0 then we are sending an HD44780 command
// RW is set low (write) for SPI, it's set low (write) for parallel
// ONLY IF the RW pin is selected, defined and used.
void LiquidCrystal::_send (uint8_t data, uint8_t rs)
{
	if (_serial_mode) { // set or clear RS bit in serial command byte
		rs ? _serial_cmd |= RSBIT : _serial_cmd &= ~RSBIT;

	} else { // set or clear RS pin (parallel mode)
		rs ? *_RS_PORT |= _RS_BIT : *_RS_PORT &= ~_RS_BIT;
	}

	if (_bit_mode == MODE_4) {
		_transfer4bits (data >> 4); // send top half of byte
		_transfer4bits (data & 0x0F); // send bottom half of byte

	} else {
		if (_serial_mode) {
			*_STB_PORT &= ~_STB_BIT; // assert strobe
			_transfer_SPI (_serial_cmd); // send command via SPI
			_transfer_SPI (data); // send data via SPI
			*_STB_PORT |= _STB_BIT; // de-assert strobe
		} else {
			_transfer8bits (data); // send command or data via parallel
		}
	}
}

// 4 bit mode (we send top 4 bits, then bottom 4)
void LiquidCrystal::_transfer4bits (uint8_t data)
{
	uint8_t n = 4; // bit count
	while (n--) { // 4 bits parallel
		(data & _BV(n)) ? *_DATA_PORT[n + 4] |= _BIT_MASK[n + 4] : *_DATA_PORT[n + 4] &= ~_BIT_MASK[n + 4];
	}
	_pulseEnable (); // clock in the data
}

// 8 bit mode (we send all 8 bits at once)
void LiquidCrystal::_transfer8bits (uint8_t data)
{
	uint8_t n = 8; // bit count
	while (n--) { // 8 bits parallel
		(data & _BV(n)) ? *_DATA_PORT[n] |= _BIT_MASK[n] : *_DATA_PORT[n] &= ~_BIT_MASK[n];
	}
	_pulseEnable (); // clock in the data
}

// pulse the "enable" pin to clock in data
void LiquidCrystal::_pulseEnable (void)
{
	_delay_usec (1);
	*_EN_PORT |= _EN_BIT; // assert "enable" (strobe in the data byte)
	_delay_usec (1);
	*_EN_PORT &= ~_EN_BIT; // de-assert "enable"
	_delay_usec (100);
}

// serial I/O for Noritake CUU displays
// this is like SPI, but using only one pin for both read
// and write. currently received data isn't used anywhere
uint8_t LiquidCrystal::_transfer_SPI (uint8_t data)
{
	uint8_t bits = 8;
	while (bits--) { // write out bits
		*_SCK_PORT &= ~_SCK_BIT; // set sck low (latch read, begin write)
		*_SIO_DDR |= _SIO_BIT; // set siso DDR as output
		data &_BV(bits) ? *_SIO_PORT |= _SIO_BIT : *_SIO_PORT &= ~_SIO_BIT; // write bit
		*_SCK_PORT |= _SCK_BIT; // set sck high (latch write, begin read)
		*_SIO_DDR &= ~_SIO_BIT; // set siso DDR as input
		*_SIO_PIN ? data |= _BV(bits) : data &= ~_BV(bits); // read bit
	}
	return data;
}

// end of LiquidCrystal.cpp
