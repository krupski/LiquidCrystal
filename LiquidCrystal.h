///////////////////////////////////////////////////////////////////////////////
//
//  Arduino Liquid Crystal (LCD) driver
//  Copyright (c) 2012 David A. Mellis <dam@mellis.org>
//  Copyright (c) 2015 Roger A. Krupski <rakrupski@verizon.net>
//
//  Last update: 01 August 2015
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
  #define PGM_READ pgm_read_byte_far
#else
  #define PGM_READ pgm_read_byte_near
#endif

// hd44780 commands
#define LCD_CLEARDISPLAY   _BV(0)
#define LCD_RETURNHOME     _BV(1)
#define LCD_ENTRYMODESET   _BV(2)
#define LCD_DISPLAYCONTROL _BV(3)
#define LCD_CURSORSHIFT    _BV(4)
#define LCD_FUNCTIONSET    _BV(5)
#define LCD_SETCGRAMADDR   _BV(6)
#define LCD_SETDDRAMADDR   _BV(7)

// bits for display entry mode
#define LCD_ENTRYRIGHT       0x00
#define LCD_ENTRYLEFT      _BV(1)
#define LCD_ENTRYSHIFTDEC    0x00
#define LCD_ENTRYSHIFTINC  _BV(0)

// bits for display control
#define LCD_BLINKOFF         0x00
#define LCD_BLINKON        _BV(0)
#define LCD_CURSOROFF        0x00
#define LCD_CURSORON       _BV(1)
#define LCD_DISPLAYOFF       0x00
#define LCD_DISPLAYON      _BV(2)

// bits for cursor control
#define LCD_MOVELEFT         0x00
#define LCD_MOVERIGHT      _BV(2)
#define LCD_CURSORMOVE       0x00
#define LCD_DISPLAYMOVE    _BV(3)

// bits for function set
#define LCD_5x8DOTS          0x00
#define LCD_5x10DOTS       _BV(2)
#define LCD_1LINE            0x00
#define LCD_2LINE          _BV(3)
#define LCD_4BITMODE         0x00
#define LCD_8BITMODE       _BV(4)

// defines for _init code_, not LCD
#define MODE_4               0x04 // 4 bit parallel mode
#define MODE_8               0x08 // 8 bit parallel or SPI mode
#define MODE_S               0xFF // flag: serial SPI mode
#define NO_RW                0xFF // flag: read/write pin not used
#define NO_RST               0xFF // flag: reset pin not used or not available

// misc defines
#define _READ                HIGH // read bit is 1
#define _WRITE                LOW // write bit is 0
#define _DATA                HIGH // register select high = LCD data
#define _CMD                  LOW // register select low  = LCD command
#define LCD_BUSYFLAG       _BV(7) // 1=busy, 0=ready (must READ status to get this [which we don't do yet])

// serial command byte (Noritake CU20049-UW2J manual pg. 12)
// bit [7...3] = 1
// bit [2] = read/*write (1=read,0=write)
// bit [1] = register select (1=data,0=command)
// bit [0] = 0
#define RSBIT              _BV(1) // register select bit
#define RWBIT              _BV(2) // read/write bit (1=read, 0=write)

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

    // initialization
    void init (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void begin (uint8_t, uint8_t, uint8_t = LCD_5x8DOTS);

    // user commands
    void setBrightness (uint8_t);
    void home (void);
    void clearScreen (void);
    void clear (void);
    void setRowOffsets (uint8_t, uint8_t, uint8_t, uint8_t);
    void setLine (uint8_t, uint8_t);
    void setCursor (uint8_t, uint8_t);
    void getCursor (uint8_t &, uint8_t &);
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
    void createChar (uint8_t, const uint8_t *);
    void createChar_P (uint8_t, const uint8_t *);
    void createChar_P (uint8_t, const uint32_t);

	// dummies for stream compatibility
    virtual int available (void);
    virtual int peek (void);
    virtual int read (void);
    virtual void flush (void);

	// universal write used by Print
    virtual size_t write (uint8_t);
    using Print::write;

  private:
    // private code begins here
    size_t _backSpace (void);
    size_t _lineFeed (void);
    size_t _carriageReturn (void);
    void _send_cmd (uint8_t);
    void _send_data (uint8_t);
    void _send (uint8_t, uint8_t);
    void _setRW (uint8_t);
    void _transfer4bits (uint8_t);
    void _transfer8bits (uint8_t);
    uint8_t _serial_IO (uint16_t);

    // variables
	uint8_t _x; // generic
	uint8_t _y; // ditto
	uint16_t _z; // 16 bit generic
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
    uint8_t _row_offsets[4];

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
};
#endif
