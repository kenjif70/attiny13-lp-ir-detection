# attiny13-lp-ir-detection
Low power IR signal detection with ATtiny13

This code uses a watchdog timer to detect IR signal periodically and lets ATtiny13 sleep most of the time.
And by powering an IR receiver by PB4 of ATtiny13, the receiver also wakes up periodically so that it won't consume power most of the time.

This code can be compiled and flashed with https://github.com/MCUdude/MicroCore. The following settings are recommended to make its power consumption even lower.
* Clock 1MHz (or 128kHz, which requires one line change in Arduino ISP code)
* BOD disabled

Pins to be used:
* PB2 (output): Connect to LED or anything. HIGH when it detects IR signal; otherwise LOW
* PB3 (input): Connect to an output of IR receiver. It assumes LOW when IR gets detected.
* PB4 (output): Connect to Vcc of IR receiver.
