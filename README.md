# lib-rs485-soft
SoftwareSerial based RS485 basic library for sending and receiving chunks of data

# Usage & Wiring
SoftwareSerial serial(pinRO, pinDI);
RS485Soft rs485(serial, pinRO, pinDI, pinControl);
