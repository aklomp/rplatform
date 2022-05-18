# rplatform

This project contains the firmware for a rotating platform that I designed and
built. The platform is part of a homebuilt panoramic camera system.

The firmware runs on a custom designed PCB, so it is not of much use to anyone
else. It uses [`libopencm3`](https://github.com/libopencm3/) as a thin hardware
abstraction layer. The library is included as a submodule.

The firmware drives a worm-geared motor by controlling a `DRV8833` motor driver
chip. It displays information about rotation speed and direction to the user
via a liquid crystal display (LCD) that is driven by a `HT1621` chip. Why an
LCD? They're energy efficient, easy to read out in full sunlight, and I wanted
an excuse to use one in a project.

The firmware listens for inputs from a rotary encoder (to set the speed and
switch between fine and coarse speed adjustments) and an on/off switch that
engages and disengages the motor.

I added a `DS18B20` temperature sensor to the board because there was enough
free space and GPIO lines, and added firmware support to blink the temperature
in degrees C every thirteen seconds when the manual controls are idle. This
serves no real purpose other than to add a cool feature and work through my box
of spare parts.

The display includes various animations to show the direction of rotation and
whether the motor is engaged.

The board has an LED that pulses three times when turned on, to indicate
successful powerup. The LED is also turned on when the `DRV8833` drives the
`FAULT` line low.
