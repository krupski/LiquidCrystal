///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD) driver
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//
//  Changes and additions: Added an SPI-like serial driver
//  for the Noritake CUU class of Vacuum Florescent Displays.
//  Copyright (c) 2012, 2015 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 19 July 2015
//
//  Free software. Use, modify, distribute freely. No warranty is
//  given or implied. Absolutely NOT for medical, aerospace or any
//  other life-critical applications.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef LIQUID_CRYSTAL_H
#define LIQUID_CRYSTAL_H

#if ARDUINO < 100
	#include <WProgram.h>
#else
	#include <Arduino.h>
#endif

#include <util/delay.h> // for _delay_us() and _delay_ms()

#ifndef _delay_ns
#define _delay_ns(X) _delay_us(X/1e3f) // _delay_ns() if we don't have it
#endif

#if defined(pgm_read_byte_far)
	#ifndef PGM_READ
		#define PGM_READ pgm_read_byte_far
	#else
		#define PGM_READ pgm_read_byte_near
	#endif
#endif

// hd44780 standard commands
#define LCD_CLEARDISPLAY _BV(0) // display command
#define LCD_RETURNHOME   _BV(1) // display command
#define LCD_SETCGRAMADDR _BV(6) // display command
#define LCD_SETDDRAMADDR _BV(7) // display command

// command and bits for display entry mode set
#define LCD_ENTRYMODESET _BV(2) // command
#define LCD_ENTRYNOSHIFT     0  // "S" = no shift
#define LCD_ENTRYSHIFT   _BV(0) // "S" = shift display
#define LCD_ENTRYDEC         0  // "I/D" = decrement char pos
#define LCD_ENTRYINC     _BV(1) // "I/D" = increment char pos

// command and bits for display control
#define LCD_DISPLAYCTRL  _BV(3) // command
#define LCD_BLINKOFF         0  // "B" blink off
#define LCD_BLINKON      _BV(0) // "B" blink on
#define LCD_CURSOROFF        0  // "C" cursor off
#define LCD_CURSORON     _BV(1) // "C" cursor on
#define LCD_DISPLAYOFF       0  // "D" display off
#define LCD_DISPLAYON    _BV(2) // "D" display on

// command and bits for cursor shift
#define LCD_CURSORSHIFT  _BV(4) // command
#define LCD_MOVELEFT         0  // "R/L" lo=left
#define LCD_MOVERIGHT    _BV(2) // "R/L" hi=right
#define LCD_CURSORMOVE       0  // "S/C" lo=move cursor
#define LCD_DISPLAYMOVE  _BV(3) // "S/C" hi=move display

// command and bits for function setting
#define LCD_FUNCTIONSET  _BV(5) // command
#define LCD_5x8DOTS          0  // "F" lo=5x8 (normal)
#define LCD_5x10DOTS     _BV(2) // "F" hi=5x10 (not all support this)
#define LCD_1LINE            0  // "N" lo=one line display
#define LCD_2LINE        _BV(3) // "N" hi=two line display
#define LCD_4BITMODE         0  // "DL" lo=display i/o mode 4 bits
#define LCD_8BITMODE     _BV(4) // "DL" hi=display i/o mode 8 bits

// defines for _init code_, not LCD
#define MODE_4               4 // 4 bit parallel mode
#define MODE_8               8 // 8 bit parallel or SPI mode
#define MODE_S            0xFF // serial SPI mode
#define NO_RW             0xFF // read/write pin not used
#define NO_RST            0xFF // reset pin not used or not available

// misc defines
#define LCD_BUSYFLAG     _BV(7) // 1=busy, 0=ready (must read status to get this)
#define _READ             HIGH  // read bit is 1
#define _WRITE             LOW  // write bit is 0

// serial command byte (Noritake CU20049-UW2J manual pg. 12)
// bit [7...3] = 1
// bit [2] = read/*write (1=read,0=write)
// bit [1] = register select (1=data,0=command)
// bit [0] = 0
#define RWBIT            _BV(2) // read/write bit (1=read, 0=write)
#define RSBIT            _BV(1) // register select bit

class LiquidCrystal : public Stream {
	public:
		// serial, no reset
		LiquidCrystal (uint8_t, uint8_t, uint8_t);
		// serial, with reset
		LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t);
		// 4 bit parallel, no r/w
		LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
		// 4 bit parallel, with r/w
		LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
		// 8 bit parallel, no r/w
		LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
		// 4 bit parallel, with r/w
		LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
		void init (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
		void begin (uint8_t, uint8_t, uint8_t = LCD_5x8DOTS);
		void setBrightness (uint8_t);
		void home (void);
		void clear (void);
		void clearScreen (void);
		void setLine (uint8_t, uint8_t);
		void setCursor (uint8_t, uint8_t);
		void getCursor (uint8_t &, uint8_t &);
		void noDisplay (void);
		void display (void);
		void noCursor (void);
		void cursor (void);
		void noBlink (void);
		void blink (void);
		void setDisplay (uint8_t);
		void setUnderline (uint8_t);
		void setBlink (uint8_t);
		void createChar (uint8_t, const uint8_t *);
		void createChar_P (uint8_t, const uint8_t *);
		void createChar_P (uint8_t, const uint32_t);
		virtual int available (void);
		virtual int peek (void);
		virtual int read (void);
		virtual void flush (void);
		virtual size_t write (uint8_t);
		using Print::write;
	private:
		// pin bitmasks
		uint8_t _BIT_MASK[8];
		uint8_t _RS_BIT;
		uint8_t _RW_BIT;
		uint8_t _EN_BIT;
		uint8_t _SCK_BIT;
		uint8_t _STB_BIT;
		uint8_t _RST_BIT;
		uint8_t _SIO_BIT;
		// output ports
		volatile uint8_t *_DATA_PORT[8];
		volatile uint8_t *_RS_PORT;
		volatile uint8_t *_RW_PORT;
		volatile uint8_t *_EN_PORT;
		volatile uint8_t *_SCK_PORT;
		volatile uint8_t *_STB_PORT;
		volatile uint8_t *_RST_PORT;
		volatile uint8_t *_SIO_PORT;
		volatile uint8_t *_SIO_PIN; // sio is needed as an input too
		// port data direction registers
		volatile uint8_t *_DATA_DDR[8];
		volatile uint8_t *_RS_DDR;
		volatile uint8_t *_RW_DDR;
		volatile uint8_t *_EN_DDR;
		volatile uint8_t *_SCK_DDR;
		volatile uint8_t *_STB_DDR;
		volatile uint8_t *_RST_DDR;
		volatile uint8_t *_SIO_DDR;
		// private code begins here
		void _send_cmd (uint8_t);
		void _send_data (uint8_t);
		size_t _backSpace (void);
		size_t _lineFeed (void);
		size_t _carriageReturn (void);
		void _setRW (uint8_t);
		void _send (uint8_t, uint8_t);
		void _transfer4bits (uint8_t);
		void _transfer8bits (uint8_t);
		uint8_t _serial_IO (uint16_t);
		uint8_t _cur_x;
		uint8_t _cur_y;
		uint8_t _numcols;
		uint8_t _numrows;
		uint8_t _serial_cmd;
		uint8_t _serial_mode;
		uint8_t _rw_pin;
		uint8_t _reset_pin;
		uint8_t _bit_mode;
		uint8_t _displayFunction;
		uint8_t _displayControl;
		uint8_t _displayCursor;
		uint8_t _displayEntryMode;
};

#endif
