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

#ifndef LIQUID_CRYSTAL_H
#define LIQUID_CRYSTAL_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

class LiquidCrystal : public Print {
	public:
		// serial, no reset
		LiquidCrystal (
			uint8_t, uint8_t, uint8_t
		); // 3

		// serial, with reset
		LiquidCrystal (
			uint8_t, uint8_t, uint8_t, uint8_t
		); // 4

		// 4 bit parallel, no r/w
		LiquidCrystal (
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t
		); // 6

		// 4 bit parallel, with r/w, w/o reset
		LiquidCrystal (
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
			uint8_t
		); // 7

		// 4 bit parallel, with r/w, with reset
		LiquidCrystal (
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
			uint8_t, uint8_t
		); // 8

		// 8 bit parallel, no r/w
		LiquidCrystal (
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
			uint8_t, uint8_t, uint8_t, uint8_t
		); // 10

		// 8 bit parallel, with r/w, w/o reset
		LiquidCrystal (
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t
		); // 11

		// 8 bit parallel, with r/w, with reset
		LiquidCrystal (
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t
		); // 12

		// initialization
		void initalize (
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
			uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t
		); // 13

		void init (uint8_t, uint8_t, uint8_t = 0); // init is same as begin
		void begin (uint8_t, uint8_t, uint8_t = 0);

		// user commands
		void setBrightness (uint8_t);
		void home (void);
		void clearScreen (void);
		void clear (void);
		void setRowOffsets (uint8_t, uint8_t, uint8_t, uint8_t);
		void setLine (uint8_t, uint8_t);
		void getLine (uint8_t &, uint8_t &);
		void setCursor (uint8_t, uint8_t);
		void getCursor (uint8_t &, uint8_t &);
		void pushCursor (void); // for user only
		void popCursor (void); // for user only
		void noDisplay (void);
		void display (void);
		void noCursor (void);
		void cursor (void);
		void noBlink (void);
		void blink (void);
		void noUnderline (void);
		void underline (void);
		void noAutoscroll (void);
		void autoscroll (void);
		void setDisplay (uint8_t);
		void setUnderline (uint8_t);
		void setBlink (uint8_t);
		void setAutoscroll (uint8_t);
		void scrollDisplayLeft (void);
		void scrollDisplayRight (void);
		void leftToRight (void);
		void rightToLeft (void);
		void createChar (uint8_t, const char *);
		void createChar (uint8_t, const uint8_t *);
		void createChar_P (uint8_t, const char *);
		void createChar_P (uint8_t, const uint8_t *);
		void createChar_E (uint8_t, const char *);
		void createChar_E (uint8_t, const uint8_t *);
		void vt_Reset (void);
		size_t vt_Exec (void);
		size_t write (uint8_t);
		using Print::write; // pull in write

	private:
		// private code begins here
		// hd44780 commands
#define CLEARDISPLAY (1<<0)
#define RETURNHOME   (1<<1)
#define SETCGRAMADDR (1<<6)
#define SETDDRAMADDR (1<<7)

		// bits for entry mode set
#define ENTRYMODESET (1<<2)
#define DISPLAYSHIFT (1<<0) // 1=display shift/0=cursor shift
#define INCREMENT   (1<<1) // CG/DD RAM address 1=increment/0=decrement

		// bits for display control
#define DISPLAYCTRL (1<<3)
#define BLINKON     (1<<0) // blink 1=on/0=off
#define CURSORON    (1<<1) // cursor 1=on/0=off
#define DISPLAYON   (1<<2) // display 1=on/0=off (cathode off)

		// bits for cursor control
#define CURSORSHIFT (1<<4)
#define MOVERIGHT   (1<<2) // move char 1=right/0=left
#define DISPLAYMOVE (1<<3) // display shift

		// bits for function set
#define FUNCTIONSET (1<<5)
#define BITMODE8    (1<<4) // interface mode 1=8 bit/0=4 bit

		// defines for _init code_, not LCD/VFD
#define DOTS5X10    (1<<2)
#define LINES2      (1<<3)
#define MODE_4        0x04 // 4 bit parallel mode
#define MODE_8        0x08 // 8 bit parallel or SPI mode
#define MODE_S        0xFF // flag: serial SPI mode
#define NO_RW         0xFF // flag: read/write pin not used
#define NO_RST        0xFF // flag: reset pin not used or not available

		// misc defines
#define _READ         HIGH // read bit is 1
#define _WRITE         LOW // write bit is 0
#define _DATA         HIGH // register select high = LCD/VFD read or write data
#define _CMD           LOW // register select low  = LCD/VFD write command
#define _STAT          LOW // register select low  = LCD/VFD read status
#define _MSEC       1000.0 // divider for builtin delay milliseconds
#define _USEC    1000000.0 // divider for builtin delay microseconds
#define _NSEC 1000000000.0 // divider for builtin delay nanoseconds
#define _RSBIT      (1<<1) // register select bit
#define _RWBIT      (1<<2) // read/write bit (1=read, 0=write)
#define _SYNC       ((1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)) // serial synchronous bits

		// prototypes
		void _clearChar (uint8_t);
		size_t _backSpace (void);
		size_t _lineFeed (void);
		size_t _carriageReturn (void);
		size_t _doTabs (uint8_t);
		uint8_t _recv_stat (void);
		uint8_t _recv_data (void);
		uint8_t _recv (uint8_t);
		uint8_t _recv4bits (void);
		uint8_t _recv8bits (void);
		void _send_cmd (uint8_t);
		void _send_data (uint8_t);
		void _send (uint8_t, uint8_t);
		void _send4bits (uint8_t);
		void _send8bits (uint8_t);
		void _serialSend (uint8_t);
		uint8_t _serialRecv (void);
		void _setDDR (uint8_t);

		// variables
		uint8_t _cur_x;
		uint8_t _cur_y;
		uint8_t _save_x;
		uint8_t _save_y;
		uint8_t _numCols;
		uint8_t _numRows;
		uint8_t _row_offsets[4];
		uint8_t _serial_cmd;
		uint8_t _serial_mode;
		uint8_t _rw_pin;
		uint8_t _reset_pin;
		uint8_t _bit_mode;
		uint8_t _displayMode;
		uint8_t _displayControl;
		uint8_t _displayCursor;
		uint8_t _displayFunction;
		uint8_t vt_state;
		uint8_t vt_cmd;
		uint8_t vt_args;
		uint8_t vt_arg[8];

		// pin bitmasks
		uint8_t _BIT_MASK[8];
		uint8_t _RS_BIT;
		uint8_t _RW_BIT;
		uint8_t _EN_BIT;
		uint8_t _SCK_BIT;
		uint8_t _STB_BIT;
		uint8_t _RST_BIT;
		uint8_t _SIO_BIT;

		// I/O ports
		volatile uint8_t *_DATA_PORT[8];
		volatile uint8_t *_DATA_PIN[8];
		volatile uint8_t *_RS_PORT;
		volatile uint8_t *_RW_PORT;
		volatile uint8_t *_EN_PORT;
		volatile uint8_t *_SCK_PORT;
		volatile uint8_t *_STB_PORT;
		volatile uint8_t *_RST_PORT;
		volatile uint8_t *_SIO_PORT;

		// sio is needed as an input too
		volatile uint8_t *_SIO_PIN;

		// port data direction registers
		volatile uint8_t *_DATA_DDR[8];
		volatile uint8_t *_RS_DDR;
		volatile uint8_t *_RW_DDR;
		volatile uint8_t *_EN_DDR;
		volatile uint8_t *_SCK_DDR;
		volatile uint8_t *_STB_DDR;
		volatile uint8_t *_RST_DDR;
		volatile uint8_t *_SIO_DDR;
};

#endif // #ifndef LIQUID_CRYSTAL_H
