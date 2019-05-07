///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD/VFD) driver library
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//  Copyright (c) 2019 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 6 May 2019
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

// serial interface, hardware reset not available
LiquidCrystal::LiquidCrystal (
	uint8_t siso, uint8_t stb, uint8_t sck
) { // 3
	init (MODE_S, siso, stb, sck, NO_RST, 0, 0, 0, 0, 0, 0, 0, NO_RST);
}

// serial interface, hardware reset is available (D0 pin is used for reset)
LiquidCrystal::LiquidCrystal (
	uint8_t siso, uint8_t stb, uint8_t sck, uint8_t reset
) { // 4
	init (MODE_S, siso, stb, sck, reset, 0, 0, 0, 0, 0, 0, 0, NO_RST);
}

// parallel interface 4 bits without active r/w (must tie r/w low manually)
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) { // 6
	init (MODE_4, rs, NO_RW, en, 0, 0, 0, 0, d4, d5, d6, d7, NO_RST);
}

// parallel interface 4 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) { // 7
	init (MODE_4, rs, rw, en, 0, 0, 0, 0, d4, d5, d6, d7, NO_RST);
}

// parallel interface 4 bits with active r/w and active reset
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
	uint8_t v0
) { // 8
	init (MODE_4, rs, rw, en, 0, 0, 0, 0, d4, d5, d6, d7, v0);
}

// parallel interface 8 bits without active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, /* no rw */ uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) { // 10
	init (MODE_8, rs, NO_RW, en, d0, d1, d2, d3, d4, d5, d6, d7, NO_RST);
}

// parallel interface 8 bits with active r/w
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7
) { // 11
	init (MODE_8, rs, rw, en, d0, d1, d2, d3, d4, d5, d6, d7, NO_RST);
}

// parallel interface 8 bits with active r/w and active reset
LiquidCrystal::LiquidCrystal (
	uint8_t rs, uint8_t rw, uint8_t en,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
	uint8_t v0
) { // 12
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
		*_SIO_DDR |= _SIO_BIT;
		*_SIO_PORT |= _SIO_BIT;

		n = digitalPinToPort (rw); // STROBE pin is on RW
		_STB_BIT = digitalPinToBitMask (rw);
		_STB_PORT = portOutputRegister (n);
		_STB_DDR = portModeRegister (n);
		*_STB_DDR |= _STB_BIT;
		*_STB_PORT |= _STB_BIT;

		n = digitalPinToPort (en); // SCLOCK pin is on EN
		_SCK_BIT = digitalPinToBitMask (en);
		_SCK_PORT = portOutputRegister (n);
		_SCK_DDR = portModeRegister (n);
		*_SCK_DDR |= _SCK_BIT;
		*_SCK_PORT |= _SCK_BIT;

		if (_reset_pin != NO_RST) { // if we are using reset...
			n = digitalPinToPort (d0); // RESET pin is on D0
			_RST_BIT = digitalPinToBitMask (d0);
			_RST_PORT = portOutputRegister (n);
			_RST_DDR = portModeRegister (n);
			*_RST_DDR |= _RST_BIT; // set it as output
			*_RST_PORT &= ~_RST_BIT; // lower reset pin
			__builtin_avr_delay_cycles (F_CPU / (_MSEC / 10.0)); // delay 10 msec
			*_RST_PORT |= _RST_BIT; // raise reset pin
		}

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
		*_RS_DDR |= _RS_BIT; // ddr = output
		*_RS_PORT |= _RS_BIT; // initial setting RS = HIGH = data

		n = digitalPinToPort (en); // enable
		_EN_BIT = digitalPinToBitMask (en);
		_EN_PORT = portOutputRegister (n);
		_EN_DDR = portModeRegister (n);
		*_EN_DDR |= _EN_BIT; // ddr = output
		*_EN_PORT &= ~_EN_BIT; // initial = low

		if (_rw_pin != NO_RW) { // read/write
			n = digitalPinToPort (rw);
			_RW_BIT = digitalPinToBitMask (rw);
			_RW_PORT = portOutputRegister (n);
			_RW_DDR = portModeRegister (n);
			*_RW_DDR |= _RW_BIT; // ddr = output
			*_RW_PORT &= ~_RW_BIT; // initial setting RW = LOW = write
		}

		if (v0 != NO_RST) {
			_reset_pin = v0; // alternate use of pin
			n = digitalPinToPort (_reset_pin); // RESET pin is on V0
			_RST_BIT = digitalPinToBitMask (_reset_pin);
			_RST_PORT = portOutputRegister (n);
			_RST_DDR = portModeRegister (n);
			*_RST_DDR |= _RST_BIT; // set it as output
			*_RST_PORT &= ~_RST_BIT; // lower reset pin
			__builtin_avr_delay_cycles (F_CPU / (_MSEC / 10));
			*_RST_PORT |= _RST_BIT; // raise reset pin
		}

		x = 8;

		while (x--) {
			n = digitalPinToPort (data_pin[x]);
			_BIT_MASK[x] = digitalPinToBitMask (data_pin[x]); // get bitmask
			_DATA_DDR[x] = portModeRegister (n); // get DDR register
			_DATA_PORT[x] = portOutputRegister (n); // get output port register
			_DATA_PIN[x] = portInputRegister (n); // get input port register
			// if we are in 4 bit mode then only set d7...d4
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
//  use this one if LCD/VFD lines 2 and 3 don't line up properly
//	setRowOffsets (0x00, 0x40, 0x10, 0x50);


	// we need at least 40ms after power rises above 2.7V before sending commands.
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 100.0));

	x = _bit_mode; // save actual bitmode
	_bit_mode = MODE_8; // force reset to be 8 bit

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
	_send_cmd (_displayFunction);
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 100.0));

	_send_cmd (_displayFunction);
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 50.0));

	_send_cmd (_displayFunction);
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 50.0));

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
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 50.0));

	_bit_mode = x; // now driver uses 4 or 8 bits

	_send_cmd (_displayMode); // entry mode set
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 50.0));

	setDisplay (1); // turn display on
	clearScreen();  // clear display
	vt_reset(); // initialize vt parser
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
	_send_cmd (FUNCTIONSET);
	// send the brightness control bits - 0b00:100%, 0b01:75%, 0b10:50%, 0b11:25%
	_send_data (brite); // set brightness (VFD only)
}

void LiquidCrystal::home (void)
{
	_send_cmd (RETURNHOME);
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 50.0));
	setCursor (0, 0);
}

void LiquidCrystal::clearScreen (void)
{
	clear();
}

void LiquidCrystal::clear (void)
{
	_send_cmd (CLEARDISPLAY);
	__builtin_avr_delay_cycles (F_CPU / (_MSEC / 50.0));
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
	_send_cmd (SETDDRAMADDR | (_cur_x + _row_offsets[_cur_y]));
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

	_send_cmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_send_data (bitmap[n]); // 8 bytes to a char (but only 5 bits)
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

	_send_cmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_send_data (pgm_read_byte (bitmap + n));
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

	_send_cmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_send_data (eeprom_read_byte ((const uint8_t *)(bitmap + n)));
	}

	home();  // make sure cursor isn't fubar
}

// reset VT parser to starting defaults
void LiquidCrystal::vt_reset (void)
{
	vt_state = 0;
	vt_cmd = 0;
	vt_args = 0;
	memset (vt_arg, 0, sizeof (vt_arg));
}

// execute a VT command
size_t LiquidCrystal::vt_exec (void)
{
	uint8_t i, n;
	double col, row;

	switch (vt_cmd) {

		// CSI[n]A (cursor up), CSI[n]B (cursor down),
		// CSI[n]C (cursor forward) or CSI[n]D (cursor backward).
		// NOTE: [n] is optional and defaults to 1 if not given
		case 'A' ... 'D': {
			static struct {
				const double col, row;
			} adj[] = {
				{  0.0, -1.0 },
				{  0.0,  1.0 },
				{  1.0,  0.0 },
				{ -1.0,  0.0 },
			};

			i = (vt_cmd - 'A');

			n = (vt_arg[0] > 0) ? vt_arg[0] : 1;

			getLine (col, row); // get current cursor row & column

			while (n--) {
				row += adj[i].row;
				col += adj[i].col;
			}
			setLine (col, row); // set new cursor row & column
			break;
		}

		// CSI[row];[column]f (line position X,Y)
		// CSI[row];[column]H (line position X,Y)
		// NOTE: non-standard, ANSI uses 1;1 as the home position while
		//       we use 0;0 to conform with the Arduino numbering.
		case 'f':
		case 'H': {
			col = vt_arg[0];
			row = vt_arg[1];
			setLine (col, row); // set new cursor row & column
			break;
		}

		// CSI[2]J (erase in display `clear screen')
		case 'J': {
			if (vt_arg[0] == 2) {
				clearScreen();
			}
			break;
		}

		// CSIm (select graphic rendition)
		// we sort of support this as follows:
		// CSI0m:  reset:   set brightness to  50% and invert off
		// CSI1m:  bold:    set brightness to 100%
		// CSI2m:  faint:   set brightness to  20%
		// CSI7m:  inverse: set video invert on
		// CSI27m: inv off: set video invert off
		// CSI30m...37m: foreground "colors" 30...37 are 8 steps of brightness
		case 'm': {
			for (n = 0; n < vt_args; n++) {
				switch (vt_arg[n]) {
					case 0: {
						setBrightness (50); // ANSI reset/normal
						break;
					}
					case 1: {
						setBrightness (100); // ANSI bold/bright
						break;
					}
					case 2: {
						setBrightness (20); // ANSI faint/dim
						break;
					}
					// ansi colors used as brightness control
					case 30 ... 39: {
						// 30==0, 39==99
						setBrightness ((vt_arg[n] % 10) * 11);
						break;
					}
					default: {
						break;
					}
				}
			}
		}

		// CSIs (save cursor position)
		case 's': {
			pushCursor();
			break;
		}

		// CSIu (restore cursor position)
		case 'u': {
			popCursor();
			break;
		}

		default: {
			break;
		}
	}

	vt_reset(); // cmd done, reset parser

	return 0;
}

size_t LiquidCrystal::write (int c)
{
	// support non-uint8_t writes
	write ((uint8_t)(c));
	return 1;
}

///////////////////////////////////////////////////////////////
//  note 0x00...0x07 are custom chars, so we parse VT first  //
///////////////////////////////////////////////////////////////
size_t LiquidCrystal::write (uint8_t c)
{
	switch (vt_state) {

		// state 0 is "ordinary character" or "ground state"
		case 0: {
			if (c == 0x1B) { // VT code starts with ESC
				vt_state++; // flag "got ESC, look for more VT
				return 0; // got part of a vt sequence, don't print it

			} else {
				vt_reset(); // reset parser
				break; // fall through to main write
			}
		}

		// state 1 is "got VT escape (0x1B) look for left bracket (0x5B)"
		case 1: {
			if (c == '[') { // VT esc code followed by "["
				vt_state++; // flag "got a sequence, look for a param"
				return 0; // got part of a vt sequence, don't print it

			} else {
				vt_reset(); // reset parser
				break; // fall through to main write
			}
		}

		// state 2...9 is "get parameter"
		case 2 ... 9: {
			if (isdigit (c)) { // if 0...9 then it's a parameter
				vt_arg[vt_args] *= 10; // parse out...
				vt_arg[vt_args] += (c - '0'); // ...first param
				return 0; // got part of a vt sequence, don't print it

			} else if (c == ';') { // semicolon flags a parameter delimiter
				vt_args++; // count parsed arg
				vt_state++; // flag "got param delimiter, look for next param"
				return 0; // got part of a vt sequence, don't print it

			} else if (! ((c < '@') || (c > '~'))) { // 0x40...0x7E marks end of VT command
				vt_cmd = c; // copy VT command
				vt_args++; // normalize count
				return vt_exec(); // got a valid sequence, exec it
			}
		}

		default: {
			vt_reset(); // unknown piece of vt, reject it and print
			break; // fall through to main write
		}
	}

	switch (c) {

		default: {
			_send_data (c);

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
			break;
		}

		case '\b': {
			return _backSpace();
		}

		case '\t': {
			return _doTabs (4);
			break;
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

	return (size_t)(1);
}

void LiquidCrystal::_clearChar (uint8_t addr)
{
	uint8_t n;

	_send_cmd (SETCGRAMADDR | ((addr % 8) * 8));

	for (n = 0; n < 8; n++) {
		_send_data (0); // erase old
	}
}

size_t LiquidCrystal::_backSpace (void)
{
	uint8_t _tmp_x = _cur_x;
	uint8_t _tmp_y = _cur_y;

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

uint8_t LiquidCrystal::_recv_stat (void)
{
	return _recv (_STAT); // rs = low
}

uint8_t LiquidCrystal::_recv_data (void)
{
	return _recv (_DATA); // rs = high
}

void LiquidCrystal::_send_cmd (uint8_t cmd)
{
	_send (cmd, _CMD); // rs = low
}

void LiquidCrystal::_send_data (uint8_t dat)
{
	_send (dat, _DATA); // rs = high
}

// read either status or data determined by rs (register select)
// if rs = 1 then we are reading DD RAM or CG RAM
// if rs = 0 then we are reading BF (Busy Flag) and LCD/VFD address
// RW is set high (read) for SPI, it's set high (read) for
// parallel ONLY IF the RW pin is selected, defined and used.
uint8_t LiquidCrystal::_recv (uint8_t rs)
{
	uint8_t c;

	if (_serial_mode) { // set or clear RS bit in serial command byte
		rs ? _serial_cmd |= _RSBIT : _serial_cmd &= ~_RSBIT;
		_serial_cmd |= _RWBIT; // read mode
		*_STB_PORT &= ~_STB_BIT; // assert strobe
		_serialSend (_serial_cmd); // send command via serial
		__builtin_avr_delay_cycles (F_CPU / (_USEC / 1.0)); // pg. 13
		c = _serialRecv (); // recv data via serial
		*_STB_PORT |= _STB_BIT; // de-assert strobe

	} else { // set or clear RS pin (parallel mode)
		rs ? *_RS_PORT |= _RS_BIT : *_RS_PORT &= ~_RS_BIT;

		if (_rw_pin != NO_RW) {
			*_RW_PORT |= _RW_BIT; // set r/w high = read
			_setDDR (_READ); // set port to read
		} else {
			return 0; // can't set r/w so just return
		}

		if (_bit_mode == MODE_4) {
			c = (_recv4bits () << 4); // recv top half of byte
			c |= _recv4bits (); // recv bottom half of byte

		} else {
			c = _recv8bits (); // recv status or data via parallel
		}
	}
	return c;
}

// write either command or data determined by rs (register select)
// if rs = 1 then we are transferring text or command param data
// if rs = 0 then we are sending an HD44780 command
// RW is set low (write) for SPI, it's set low (write) for parallel
// ONLY IF the RW pin is selected, defined and used.
void LiquidCrystal::_send (uint8_t c, uint8_t rs)
{
	if (_serial_mode) { // set or clear RS bit in serial command byte
		rs ? _serial_cmd |= _RSBIT : _serial_cmd &= ~_RSBIT;
		_serial_cmd &= ~_RWBIT; // write mode
		*_STB_PORT &= ~_STB_BIT; // assert strobe
		_serialSend (_serial_cmd); // send command via serial
		_serialSend (c); // send data via serial
		*_STB_PORT |= _STB_BIT; // de-assert strobe

	} else { // set or clear RS pin (parallel mode)
		rs ? *_RS_PORT |= _RS_BIT : *_RS_PORT &= ~_RS_BIT;

		if (_rw_pin != NO_RW) {
			*_RW_PORT &= ~_RW_BIT; // set r/w low = write
			_setDDR (_WRITE); // set port to write
		}

		if (_bit_mode == MODE_4) {
			_send4bits (c >> 4); // send top half of byte
			_send4bits (c & 0x0F); // send bottom half of byte

		} else {
			_send8bits (c); // send command or data via parallel
		}
	}
}

// parallel 4 bit mode (we receive top 4 bits, then bottom 4)
uint8_t LiquidCrystal::_recv4bits (void)
{
	uint8_t c = 0;
	uint8_t n = 4;

	while (n--) { // 4 bits parallel
		(*_DATA_PIN[n + 4] & _BIT_MASK[n + 4]) ? c |= (1 << n) : c &= ~(1 << n); // receive bit
	}

	*_EN_PORT |= _EN_BIT;
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 10.0));
	*_EN_PORT &= ~_EN_BIT;

	return c;
}

// parallel 8 bit mode (we receive all 8 bits at once)
uint8_t LiquidCrystal::_recv8bits (void)
{
	uint8_t c = 0;
	uint8_t n = 8;

	while (n--) { // 8 bits parallel
		(*_DATA_PIN[n] & _BIT_MASK[n]) ? c |= (1 << n) : c &= ~(1 << n); // receive bit
	}

	*_EN_PORT |= _EN_BIT;
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 10.0));
	*_EN_PORT &= ~_EN_BIT;

	return c;
}

// parallel 4 bit mode (we send top 4 bits, then bottom 4)
void LiquidCrystal::_send4bits (uint8_t c)
{
	uint8_t n = 4; // bit count

	while (n--) { // 4 bits parallel
		(c & (1 << n)) ? *_DATA_PORT[n + 4] |= _BIT_MASK[n + 4] : *_DATA_PORT[n + 4] &= ~_BIT_MASK[n + 4];
	}

	*_EN_PORT |= _EN_BIT;
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 10.0));
	*_EN_PORT &= ~_EN_BIT; // latch data
}

// parallel 8 bit mode (we send all 8 bits at once)
void LiquidCrystal::_send8bits (uint8_t c)
{
	uint8_t n = 8; // bit count

	while (n--) { // 8 bits parallel
		(c & (1 << n)) ? *_DATA_PORT[n] |= _BIT_MASK[n] : *_DATA_PORT[n] &= ~_BIT_MASK[n];
	}

	*_EN_PORT |= _EN_BIT;
	__builtin_avr_delay_cycles (F_CPU / (_USEC / 10.0));
	*_EN_PORT &= ~_EN_BIT; // latch data
}

void LiquidCrystal::_serialSend (uint8_t c)
{
	uint8_t n = 8;

	*_SIO_DDR |= _SIO_BIT;

	while (n--) {
		__builtin_avr_delay_cycles (F_CPU / (_USEC / 5.0));
		*_SCK_PORT &= ~_SCK_BIT; // set sck low
		(c & (1 << n)) ? *_SIO_PORT |= _SIO_BIT : *_SIO_PORT &= ~_SIO_BIT; // write bit
		*_SCK_PORT |= _SCK_BIT; // set sck high
	}
}

uint8_t LiquidCrystal::_serialRecv (void)
{
	uint8_t c = 0;
	uint8_t n = 8;

	*_SIO_DDR &= ~_SIO_BIT;

	while (n--) {
		*_SCK_PORT &= ~_SCK_BIT; // set sck low
		__builtin_avr_delay_cycles (F_CPU / (_USEC / 5.0));
		*_SCK_PORT |= _SCK_BIT; // set sck high
		(*_SIO_PIN & _SIO_BIT) ? c |= (1 << n) : c &= ~(1 << n); // read bit
	}

	return c;
}

void LiquidCrystal::_setDDR (uint8_t dir)
{
	uint8_t x = 8;

	while (x--) {
		dir ? *_DATA_DDR[x] &= ~_BIT_MASK[x] : *_DATA_DDR[x] |= _BIT_MASK[x];
		// if we are in 4 bit mode then only set d7...d4
		if (x == _bit_mode) {
			break;
		}
	}
}
// end of LiquidCrystal.cpp
