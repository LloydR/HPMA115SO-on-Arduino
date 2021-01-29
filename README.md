# HPMA115SO-on-Arduino
Honeywell Particulate Counter running on an Arduino

The Honeywell HPMA115SO Particulate Counter can run with a single board computer like the Raspberry Pi 
or in this case a micro controller such as the Arduino Mega 2560
It started out with an Arduino Uno but had too many false readings, attributed to the software RS232
The Mega 2560 appears to be much better but still sometimes comes up with bad readings that don't
get flagged.
There is a YouTube video on the original at https://www.youtube.com/watch?v=l0V3Mif44uw&list=PLAxMci4f07pV-LUSe8sMRkI6Ij4QasGE3&index=18
The Raspberry Pi version does not seem to have this problem. 
I don't know what the difference is other than there is a 3 V bus I can use versus using voltage shifters on the Arduino.
