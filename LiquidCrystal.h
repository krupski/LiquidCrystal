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

#ifndef _LIQUID_CRYSTAL_H
#define _LIQUID_CRYSTAL_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

// hd44780 commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// bits for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// bits for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// bits for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// bits for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define READ HIGH
#define WRITE LOW

#define CMD LOW
#define STAT LOW
#define DAT HIGH

// serial command byte (Noritake CU20049-UW2J manual pg. 12)
// bit [7...3] = 1
// bit [2] = read/*write (1=read,0=write)
// bit [1] = register select (1=data,0=command)
// bit [0] = 0
#define RWBIT _BV (2) // read/write bit (currently not used)
#define RSBIT _BV (1) // register select bit

class LiquidCrystal : public Print
{
public:
	LiquidCrystal (uint8_t, uint8_t, uint8_t);
	LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t);
	LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
	LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
	LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
	LiquidCrystal (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
	void init (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
	void begin (uint8_t, uint8_t, uint8_t = LCD_5x8DOTS);
	void setBrightness (uint8_t);
	void clearScreen (void);
	void setLine (uint8_t, uint8_t);
	void setCursor (uint8_t, uint8_t);
	void setDisplay (uint8_t);
	void setCursor (uint8_t);
	void setBlink (uint8_t);
	void createChar (uint8_t, const uint8_t *);
	void createChar_P (uint8_t, const uint8_t *);
	virtual size_t write (uint8_t);
	using Print::write;
//private:
	inline 	void _command (uint8_t);
	void _setRW (uint8_t);
	void _send (uint8_t, uint8_t);
	void _write4bits (uint8_t);
	void _write8bits (uint8_t);
	void _sendSerial (uint8_t);
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
	uint8_t _displayfunction;
	uint8_t _displaycontrol;
	uint8_t _displaymode;
	uint8_t _numcols;
	uint8_t _numlines;
};
#endif

