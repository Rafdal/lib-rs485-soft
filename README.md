# lib-rs485-soft
SoftwareSerial based [RS485](#what-is-rs485) tools for sending and receiving data packets, networking and more!

With 3 [abstraction layers](https://en.wikipedia.org/wiki/Abstraction_layer):

### 1: Basic TX/RX Layer (RS485Soft.h)
- Sending and receiving packets (half-duplex)
- Data integrity hash check ([Pearson Hashing](https://en.wikipedia.org/wiki/Pearson_hashing))

### 2: Network Layer (RSNetDevice.h)
- *All the basic features*
- Networking features (addresses for sender and receiver) up to 255 devices in a single network
- Callbacks for incoming packets
- Set intervals for periodic actions or packet broadcasting

### 3: Master-Slave Layer (RSMaster.h & RSSlave.h)
- *All the networking and basic features*
- Master and Slave logic roles
- Advanced callbacks for topic data requests and responses
- Packet topic-handling
- Slave connectivity functions

## What is RS485?
[RS485](https://en.wikipedia.org/wiki/RS-485) is a TIA/EIA hardware standard for wired communications over long distances and noisy industrial enviroments

## Wiring
Is recommended to use PWM digital pins
 * RO -> pinRO
 * DI -> pinDI
 * DE -> pinControl
 * RE -> pinControl

## Object Declaration and Constructors
```c++
// Layer 1 (basic)
RS485Soft rs485(RXpin, TXpin, controlPin);

// Layer 2 (network)
RSNetDevice device(RXpin, TXpin, controlPin);

// Layer 3 (master-slave)
RSMaster master(RXpin, TXpin, controlPin);
RSSlave slave(RXpin, TXpin, controlPin);
```

# RS485 datasheet
For more info about wiring and RS485 module usage, you can read [this doc](https://github.com/Rafdal/lib-rs485-soft/blob/main/MAX485%20Module%205V%20logic%20TTL%20to%20RS-485.pdf)


# See the [examples folder](https://github.com/Rafdal/lib-rs485-soft/tree/main/examples) for a detalied explaination and use cases
