LiquidCrystal
=============
Arduino drivers for Hitachi HD44780 compatible LCD/VFD displays.

<p>Changes include direct port access vs digitalWrite and pinMode, the addition of SPI code to support serial interface VFD (vacuum fluorescent) devices such as the Noritake series of Hitachi HD44780 compatible VFD displays (CU20049, etc...).</p>

<p>Also several new functions have been added such as character code support (for example, a carriage return character homes the cursor, a linefeed drops down to the next line, etc...). It is also possible to read, from the driver, the current cursor position as row and column, making it easier to locate the cursor and update individual areas of the display.</p>

<p>&nbsp;</p>
<p>&nbsp;</p>

___
<sub><b>(Typical LCD Display)</b></sub>

<a href="#"><img width="414" height="190" src="https://camo.githubusercontent.com/032e614b218d67c3ff1f6b1fafa3f86b99062ea2/687474703a2f2f656c656374726f6e6963666f7270617373696f6e2e636f6d2f77702d636f6e74656e742f75706c6f6164732f323031352f30342f646973686974616368692e6a7067" /></a>

<p>&nbsp;</p>

<sub><b>(Typical VFD Display)</b></sub>

<a href="#"><img src="https://camo.githubusercontent.com/83db1615f52cb9f826d4530dfd48cf9f7791731d/68747470733a2f2f7777772e6e6f726974616b652d6974726f6e2e6a702f70726f64756374732f6d6f64756c652f63752d752f696d675f73697a652f70726f5f732f637532303034392d7577326a2e6a7067" /></a>
