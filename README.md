# lib-rs485-soft
SoftwareSerial based RS485 basic library for sending and receiving chunks of data

## Wiring
Is recommended to use PWM digital pins
 * RO -> pinRO
 * DI -> pinDI
 * DE -> pinControl
 * RE -> pinControl


## Declaration

```c++
SoftwareSerial serial(pinRO, pinDI);
RS485Soft rs485(serial, pinRO, pinDI, pinControl);
```

## RS485 datasheet
For more info about wiring and RS485 module usage, you can read [this doc](https://github.com/Rafdal/lib-rs485-soft/blob/main/MAX485%20Module%205V%20logic%20TTL%20to%20RS-485.pdf)
