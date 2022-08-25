# attiny13-lp-ir-detection
Low power IR signal detection with ATtiny13 (originally experimented by http://milkandlait.blogspot.com/2015/02/)

This code uses a watchdog timer to detect IR signal periodically and lets ATtiny13 sleep most of the time.
And by powering an IR receiver by PB4 of ATtiny13, the receiver also wakes up periodically so that it won't consume power most of the time.

This code can be compiled and flashed with https://github.com/MCUdude/MicroCore. The following settings are recommended to make its power consumption even lower.
* Clock 600kHz (128kHz is fine too, but you'd need to modify SPI_CLOCK of ArduinoISP to reprogram)
* BOD disabled

With GP1UXC4xQS IR receiver, the average power consumption was 11.6uA x 3.3V at 600kHz clock speed (measured with Nordic PPK2).

Pins to be used:
* PB2 (output): Connect to LED or anything. HIGH when it detects IR signal; otherwise LOW
* PB3 (input): Connect to an output of IR receiver. It assumes LOW when IR gets detected.
* PB4 (output): Connect to Vcc of IR receiver.
