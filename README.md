LiquidCrystal
=============
Arduino drivers for standard LCD displays and Noritake HD44780 compatible LCD/VFD displays.

Changes include direct port access instead of "digitalWrite" and "pinMode" and the addition of Noritake "CU-UW" (SPI) code to support serial interface devices such as the Noritake series of Hitachi HD44780 compatible VFD (<b>V</b>acuum <b>F</b>luorescent <b>D</b>isplay) modules (CU16025, CU20049, etc...).

Also several new functions have been added such as character code support (for example, a carriage return character homes the cursor, a linefeed drops down to the next line, etc...). It is also possible to read, from the driver, the current cursor position as row and column, making it easier to locate the cursor and update individual areas of the display.

<p>&nbsp;</p>
<p>&nbsp;</p>

___
<sub><b>(Typical LCD Display)</b></sub>
<a href="#"><img width="414" height="190" src="https://camo.githubusercontent.com/4a66dabd383acaf6b2af3c5f4b910933d0171726/687474703a2f2f6563782e696d616765732d616d617a6f6e2e636f6d2f696d616765732f492f3431577a5745357546354c2e6a7067" /></a>

<div>&nbsp;</div>

<sub><b>(Typical VFD Display)</b></sub>
<a href="http://noritake-vfd.com/" title="This link takes you to the Noritake Itron VFD Website" target="_blank"><img width="414" height="190" src="https://camo.githubusercontent.com/83db1615f52cb9f826d4530dfd48cf9f7791731d/68747470733a2f2f7777772e6e6f726974616b652d6974726f6e2e6a702f70726f64756374732f6d6f64756c652f63752d752f696d675f73697a652f70726f5f732f637532303034392d7577326a2e6a7067" /></a>
