This is my first work on a Teensy 3.1, a lot of the code is a
modified mash between the Teensy Arduino, Freescale's Processor Expert,
and my own work.
I didn't like all the extra overhead that the Arduino adds, so this is also a baremetal project.

It's just set up to be an aux display for my 2013 VW Jetta TDI,
because I wanted a coolant temp gauge and a better fuel level gauge.
(yes, the Jetta has NO coolant temp gauge, just an idiot light!)

So this was also reversing some of the newer CAN messages that VW uses as well.

I used a pair of 8-digit 7-segment SPI LED displays from DFRobot, very handy, very easy to use.
I also used a CANbus transceiver from WaveShare, cheap-o eBay special, can't complain, it works.
