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