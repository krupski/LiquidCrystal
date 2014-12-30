///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD) driver
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//
//  Changes and additions: Added an SPI-like serial driver
//  for the Noritake CUU class of Vacuum Florescent Displays.
//  Copyright (c) 2012, 2014 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 30 December 2014
//
//  Free software. Use, modify, distribute freely. No warranty is
//  given or implied. Absolutely NOT for medical, aerospace or any
//  other life-critical applications.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _LIQUID_CRYSTAL_H
#define _LIQUID_CRYSTAL_H

#if ARDUINO < 100
  #include <WProgram.h>
#else
  #include <Arduino.h>
#endif

// hd44780 stand-alone commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80
#define LCD_BUSYFLAG 0x80 // 1=busy, 0=ready

// command and bits for display entry mode set
#define LCD_ENTRYNOSHIFT 0x00 // "S" = no shift
#define LCD_ENTRYSHIFT 0x01 // "S" = shift display
#define LCD_ENTRYDEC 0x00 // "I/D" = decrement char pos
#define LCD_ENTRYINC 0x02 // "I/D" = increment char pos
#define LCD_ENTRYMODESET 0x04

// command and bits for display control
#define LCD_BLINKOFF 0x00
#define LCD_BLINKON 0x01 // "B"
#define LCD_CURSOROFF 0x00
#define LCD_CURSORON 0x02 // "C"
#define LCD_DISPLAYOFF 0x00
#define LCD_DISPLAYON 0x04 // "D"
#define LCD_DISPLAYCONTROL 0x08

// command and bits for cursor shift
#define LCD_MOVELEFT 0x00
#define LCD_MOVERIGHT 0x04 // "R/L"
#define LCD_CURSORMOVE 0x00
#define LCD_DISPLAYMOVE 0x08 // "S/C"
#define LCD_CURSORSHIFT 0x10

// command and bits for function set
#define LCD_5x8DOTS 0x00
#define LCD_5x10DOTS 0x04 // "F"
#define LCD_1LINE 0x00
#define LCD_2LINE 0x08 // "N"
#define LCD_4BITMODE 0x00
#define LCD_8BITMODE 0x10 // "DL"
#define LCD_FUNCTIONSET 0x20

// misc stuff
#define READ HIGH
#define WRITE LOW
#define CMD LOW
#define DAT HIGH

// defines for init code, not LCD
#define MODE_4 0x01 // 4 bit parallel mode
#define MODE_8 0x00 // 8 bit parallel mode
#define MODE_S 0xFF // serial SPI mode
#define NO_RW 0xFF // no read/write support
#define NO_RST 0xFF // no reset support

// serial command byte (Noritake CU20049-UW2J manual pg. 12)
// bit [7...3] = 1
// bit [2] = read/*write (1=read,0=write)
// bit [1] = register select (1=data,0=command)
// bit [0] = 0
#define HEAD (_BV(7)|_BV(6)|_BV(5)|_BV(4)|_BV(3))
#define RWBIT _BV(2) // read/write bit
#define RSBIT _BV(1) // register select bit

class LiquidCrystal : public Print
{
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
		virtual inline size_t write (uint8_t);
		using Print::write;
	private:
		void _send_cmd (uint8_t);
		void _send_dat (uint8_t);
		size_t _backSpace (void);
		size_t _lineFeed (void);
		size_t _carriageReturn (void);
		void _setRW (uint8_t);
		void _send (uint8_t, uint8_t);
		void _write4bits (uint8_t);
		void _write8bits (uint8_t);
		uint8_t _serial_IO (uint8_t = 0);
		uint8_t _cur_x;
		uint8_t _cur_y;
		uint8_t _rs_pin;
		uint8_t _rw_pin;
		uint8_t _enable_pin;
		uint8_t _data_pins[8];
		uint8_t _sck;
		uint8_t _stb;
		uint8_t _siso;
		uint8_t _reset;
		uint8_t _serial_cmd;
		uint8_t _serial_mode;
		uint8_t _bit_mode;
		uint8_t _displayFunction;
		uint8_t _displayControl;
		uint8_t _displayEntryMode;
		uint8_t _numcols;
		uint8_t _numrows;
};
#endif
