///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD) driver library
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//  Copyright (c) 2015 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 31 January 2016
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
)
{
	init (MODE_S, siso, stb, sck, NO_RST, 0, 0, 0, 0, 0, 0, 0, NO_RST);
}

// SPI serial interface, hardware reset is enabled and available (D0 pin is used for reset)
LiquidCrystal::LiquidCrystal (
	uint8_t siso, uint8_t stb, uint8_t sck, uint8_t reset
)
{
	init (MODE_S, siso, stb, sck, reset, 0, 0, 0, 0, 0, 0, 0, NO_RST);
}

// parallel interface 4 bits without active r/w (must tie r/w low manually)
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
)
{
	init (MODE_4, rs, NO_RW, en, 0, 0, 0, 0, d4, d5, d6, d7, NO_RST);
}

// parallel interface 4 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
)
{
	init (MODE_4, rs, rw, en, 0, 0, 0, 0, d4, d5, d6, d7, NO_RST);
}

// parallel interface 4 bits with active r/w and active reset
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
	uint8_t v0
)
{
	init (MODE_4, rs, rw, en, 0, 0, 0, 0, d4, d5, d6, d7, v0);
}

// parallel interface 8 bits without active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
)
{
	init (MODE_8, rs, NO_RW, en, d0, d1, d2, d3, d4, d5, d6, d7, NO_RST);
}

// parallel interface 8 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
)
{
	init (MODE_8, rs, rw, en, d0, d1, d2, d3, d4, d5, d6, d7, NO_RST);
}

// parallel interface 8 bits with active r/w and active reset
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
	uint8_t v0
)
{
	init (MODE_8, rs, rw, en, d0, d1, d2, d3, d4, d5, d6, d7, v0);
}

void LiquidCrystal::init (
	uint8_t bitmode, uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
	uint8_t v0
)
{

	uint8_t n, x;

	_bit_mode = bitmode; // 4 bit (0x04), 8 bit (0x08) or serial (0xFF) mode flag

	if (_bit_mode == MODE_S) { // 0xFF == serial mode

		_bit_mode = MODE_8; // reset it to 8 bit mode
		_serial_mode = 1; // flag "we are in serial mode"
		_reset_pin = d0; // alternate use of pin

		// serial command byte template (Noritake CU20049-UW2J manual pg. 12)
		// bit [7...3] = 1
		// bit [2] = read/write (1=read,0=write)
		// bit [1] = register select (1=data,0=command)
		// bit [0] = 0
		_serial_cmd = 0b11111000;

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
			__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e3))*1); // delay 1 msec
			*_RST_PORT |= _RST_BIT; // raise reset pin
			*_RST_DDR &= ~_RST_BIT; // set it as input_pullup (de-assert reset)
		}

	} else { // parallel mode

		const uint8_t data_pin[] = {
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

		if (_reset_pin != NO_RST) { // if we are using reset...
			n = digitalPinToPort (v0); // RESET pin is on v0
			_RST_BIT = digitalPinToBitMask (v0);
			_RST_PORT = portOutputRegister (n);
			_RST_DDR = portModeRegister (n);
			*_RST_PORT &= ~_RST_BIT; // lower reset pin
			*_RST_DDR |= _RST_BIT; // set it as output (assert reset)
			__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e3))*1); // delay 1 msec
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

void LiquidCrystal::begin (int8_t cols, int8_t rows, int8_t dotsize)
{
	uint8_t x;

	_numcols = cols;
	_numrows = rows;

	// setup default DDRAM offsets
	setRowOffsets (0x00, 0x40, 0x14, 0x54);

	// we need at least 40ms after power rises above 2.7V before sending
	// commands. Arduino can turn on way before 4.5V so we'll wait 250
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e3))*250);

	x = _bit_mode; // save actual bitmode
	_bit_mode = MODE_8; // force reset to be 8 bit

	// build _displayMode template
	// default: increment mode, no shift
	_displayMode = (ENTRYMODESET | INCREMENT);

	// build _displayControl template
	// default: display off, cursor off, blink off
	_displayControl = (DISPLAYCONTROL);

	// build _displayCursor template
	// default: cursor move, cursor moves right
	_displayCursor = (CURSORSHIFT | MOVERIGHT);

	// build _displayFunction template
	// default: 8 bit, 1 line, 5 x 8 character
	_displayFunction = (FUNCTIONSET | BITMODE8);

	// send reset sequence
	_send_cmd (_displayFunction);
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e3))*50);

	_send_cmd (_displayFunction);
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e3))*20);

	_send_cmd (_displayFunction);
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e3))*20);

	if (x == MODE_4) { // if actual bitmode is 4 then clear the 8 bit flag
		_displayFunction &= ~BITMODE8;
	}

	if (_numrows > 1) {
		_displayFunction |= LINES2;
	}

	if (dotsize) {
		_displayFunction |= DOTS5X10;
	}

	// finish display reset
	_send_cmd (_displayFunction); // set the interface bit mode
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e3))*20);

	_bit_mode = x; // now driver uses 4 or 8 bits

	_send_cmd (_displayMode); // entry mode set

	x = 8;

	while (x--) {
		clearChar (x); // clear CGRAM and init translation table
	}


	setDisplay (1); // turn display on
	clearScreen (); // clear display
}

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
	_send_cmd (FUNCTIONSET);
	// send the brightness control bits - 0b00:100%, 0b01:75%, 0b10:50%, 0b11:25%
	_send_data (brite); // set brightness (VFD only)
}

void LiquidCrystal::home (void)
{
	_send_cmd (RETURNHOME);
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e3))*20); // min 15.2 ms!!!
	setCursor (0, 0);
}

void LiquidCrystal::clearScreen (void)
{
	clear ();
}

void LiquidCrystal::clear (void)
{
	_send_cmd (CLEARDISPLAY);
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e3))*20); // min 15.2 ms!!!
	setCursor (0, 0);
}

// if using a 16x4 LCD and line 3 & 4 are not placed correctly try:
//    this ---> setRowOffsets(0x00, 0x40, 0x14, 0x54);
// or this ---> setRowOffsets(0x00, 0x40, 0x10, 0x50);
void LiquidCrystal::setRowOffsets (int8_t row0, int8_t row1, int8_t row2, int8_t row3)
{
	_row_offsets[0] = row0;
	_row_offsets[1] = row1;
	_row_offsets[2] = row2;
	_row_offsets[3] = row3;
}

void LiquidCrystal::setLine (int8_t x, int8_t y)
{
	setCursor (x, y);
}

void LiquidCrystal::setCursor (int8_t x, int8_t y)
{
	_cur_x = x; // record cursor X pos
	_cur_y = y; // record cursor Y pos
	_send_cmd (SETDDRAMADDR | (_cur_x + _row_offsets[_cur_y]));
}

void LiquidCrystal::getCursor (int8_t &x, int8_t &y)
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
	_send_cmd (_displayControl);
}

// Turns the underline cursor on/off
void LiquidCrystal::setUnderline (uint8_t on)
{
	on ? _displayControl |= CURSORON : _displayControl &= ~CURSORON;
	_send_cmd (_displayControl);
}

// Turn on and off the blinking cursor
void LiquidCrystal::setBlink (uint8_t on)
{
	on ? _displayControl |= BLINKON : _displayControl &= ~BLINKON;
	_send_cmd (_displayControl);
}

void LiquidCrystal::setAutoscroll (uint8_t on)
{
	on ? _displayMode |= DISPLAYSHIFT : _displayMode &= ~DISPLAYSHIFT;
	_send_cmd (_displayMode);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal::scrollDisplayLeft (void)
{
	_displayCursor |= (CURSORSHIFT | DISPLAYMOVE | MOVERIGHT);
	_displayCursor &= ~MOVERIGHT;
	_send_cmd (_displayCursor);
}

void LiquidCrystal::scrollDisplayRight (void)
{
	_displayCursor |= (CURSORSHIFT | DISPLAYMOVE | MOVERIGHT);
	_send_cmd (_displayCursor);
}

// This is for text that flows Left to Right
void LiquidCrystal::leftToRight (void)
{
	_displayMode |= INCREMENT;
	_send_cmd (_displayMode);
}

// This is for text that flows Right to Left
void LiquidCrystal::rightToLeft (void)
{
	_displayMode &= ~INCREMENT;
	_send_cmd (_displayMode);
}

void LiquidCrystal::clearChar (uint8_t addr)
{
	uint8_t n;

	addr %= 8; // constrain address to 0...7

	_send_cmd (SETCGRAMADDR | (addr * 8)); // select CG ram slot

	for (n = 0; n < 8; n++) {
		_send_data (0); // write a zero to CG ram
	}

	translateChar (addr, addr); // remove char from translation table
	home (); // make sure cursor isn't fubar
}

void LiquidCrystal::translateChar (uint8_t addr, uint8_t charCode)
{
	_translateTable[(addr % 8)] = charCode;
}

// custom bitmaps in SRAM
void LiquidCrystal::createChar (uint8_t addr, const char *bitmap, uint8_t charCode)
{
	createChar (addr, (const uint8_t *)(bitmap), charCode);
}

// custom bitmaps in SRAM
void LiquidCrystal::createChar (uint8_t addr, const uint8_t *bitmap, uint8_t charCode)
{
	uint8_t n;

	_send_cmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_send_data (bitmap[n]); // 8 bytes to a char (but only 5 bits)
	}

	if (charCode != 255) {
		translateChar (addr, charCode);
	}

	home (); // make sure cursor isn't fubar
}

// custom bitmaps in PROGMEM
void LiquidCrystal::createChar_P (uint8_t addr, const char *bitmap, uint8_t charCode)
{
	createChar_P (addr, (const uint8_t *)(bitmap), charCode);
}

// custom bitmaps in PROGMEM
void LiquidCrystal::createChar_P (uint8_t addr, const uint8_t *bitmap, uint8_t charCode)
{
	uint8_t n;

	_send_cmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_send_data (pgm_read_byte (bitmap + n));
	}

	if (charCode != 255) {
		translateChar (addr, charCode);
	}

	home (); // make sure cursor isn't fubar
}

// custom bitmaps in EEPROM
void LiquidCrystal::createChar_E (uint8_t addr, const char *bitmap, uint8_t charCode)
{
	createChar_E (addr, (const uint8_t *)(bitmap), charCode);
}

// custom bitmaps in EEPROM
void LiquidCrystal::createChar_E (uint8_t addr, const uint8_t *bitmap, uint8_t charCode)
{
	uint8_t n;

	_send_cmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_send_data (eeprom_read_byte ((const uint8_t *) (bitmap + n)));
	}

	if (charCode != 255) {
		translateChar (addr, charCode);
	}

	home (); // make sure cursor isn't fubar
}

///////////////////////////////////////////////
//     note 0x00...0x07 are custom chars     //
///////////////////////////////////////////////
size_t LiquidCrystal::write (uint8_t data)
{
	size_t n;

	n = 8;

	while (n--) {
		if (data == _translateTable[n]) { // if character matches a table entry...
			data = n; // get the address of the new character
			break;
		}
	}

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
			clearScreen ();
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

	if (_cur_x < (_numcols - 1)) { // if next col pos isn't at end
		_cur_x++;

	} else {
		_cur_x = 0;

		if (_cur_y < (_numrows - 1)) { // need new row
			_cur_y++;

		} else {
			_cur_x = 0;
			_cur_y = 0;
		}
	}

	setCursor (_cur_x, _cur_y);

	n = 1;

	return n;
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
	write ((uint8_t)(' '));
	setCursor (_tmp_x, _tmp_y);

	return 0;
}

size_t LiquidCrystal::_lineFeed (void)
{
	if (_cur_y < (_numrows - 1)) {
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
		n += write ((uint8_t)(' '));
	}

	while (_cur_x % _tab_size) {
		n += write ((uint8_t)(' '));
	}

	return n;
}

void LiquidCrystal::_send_cmd (uint8_t cmd)
{
	_send (cmd, _CMD); // rs = low
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
		_serial_cmd &= ~RWBIT; // write mode
		*_STB_PORT &= ~_STB_BIT; // assert strobe
		_serialXfer (_serial_cmd); // send command via serial
		_serialXfer (data); // send data via serial
		*_STB_PORT |= _STB_BIT; // de-assert strobe

	} else { // set or clear RS pin (parallel mode)
		rs ? *_RS_PORT |= _RS_BIT : *_RS_PORT &= ~_RS_BIT;

		if (_rw_pin != NO_RW) {
			*_RW_PORT &= ~_RW_BIT; // set r/w low = write
		}

		if (_bit_mode == MODE_4) {
			_send4bits (data >> 4); // send top half of byte
			_send4bits (data & 0x0F); // send bottom half of byte

		} else {
			_send8bits (data); // send command or data via parallel
		}
	}
}

// parallel 4 bit mode (we send top 4 bits, then bottom 4)
void LiquidCrystal::_send4bits (uint8_t data)
{
	uint8_t n = 4; // bit count

	while (n--) { // 4 bits parallel
		(data & (1UL << n)) ? *_DATA_PORT[n + 4] |= _BIT_MASK[n + 4] : *_DATA_PORT[n + 4] &= ~_BIT_MASK[n + 4];
	}

	_pulseEnable (); // clock in the data
}

// parallel 8 bit mode (we send all 8 bits at once)
void LiquidCrystal::_send8bits (uint8_t data)
{
	uint8_t n = 8; // bit count

	while (n--) { // 8 bits parallel
		(data & (1UL << n)) ? *_DATA_PORT[n] |= _BIT_MASK[n] : *_DATA_PORT[n] &= ~_BIT_MASK[n];
	}

	_pulseEnable (); // clock in the data
}

// pulse the "enable" pin to clock in data
void LiquidCrystal::_pulseEnable (void)
{
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e6))*1);
	*_EN_PORT |= _EN_BIT; // assert "enable" (strobe in the data byte)
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e6))*1);
	*_EN_PORT &= ~_EN_BIT; // de-assert "enable"
	__builtin_avr_delay_cycles (((double)(F_CPU)/(double)(1e6))*100);
}

uint8_t LiquidCrystal::_serialXfer (uint8_t data)
{
	uint8_t bits = 8;
	while (bits--) {
		*_SCK_PORT &= ~_SCK_BIT; // set sck low
		*_SIO_DDR |= _SIO_BIT; // set sio DDR as output
		data & (1UL << bits) ? *_SIO_PORT |= _SIO_BIT : *_SIO_PORT &= ~_SIO_BIT; // write bit
		*_SCK_PORT |= _SCK_BIT; // set sck high
		*_SIO_DDR &= ~_SIO_BIT; // set sio DDR as input
		*_SIO_PIN & _SIO_BIT ? data |= (1UL << bits) : data &= ~(1UL << bits);  // read bit
	}
	return data;
}

// end of LiquidCrystal.cpp
