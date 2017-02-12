# nixieclock
Nixie Clock Project

Code to be run on an AVR ATMege368p (should run on other model easily).
The program is fairly simple. It interact with a DS1307 over i2c (and use the latest as a time base through the externa interupt 0).
It then use a set of shift register to generate a BCD sequences used to drive 6 Nixie Drivers
