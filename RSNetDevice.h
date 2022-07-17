#ifndef RSNetDevice_H
#define RSNetDevice_H

#include "RS485Soft.h"
#include "RSPacket.h"

#define RS485_DEFAULT_BAUDRATE  9600

#define RS485_MAX_TOPIC_SIZE 12

#if RS485_MAX_TOPIC_SIZE >= RS485_MAX_DATA_SIZE
#error "RS485_MAX_TOPIC_SIZE > RS485_MAX_DATA_SIZE"
#endif

typedef void (*RSPacketCallback)(RSPacket&);


class RSNetDevice
{
public:
    RSNetDevice(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
    virtual ~RSNetDevice();

    void onPacket(RSPacketCallback callback);
    virtual void setup(uint8_t localID, const char* deviceName);
    virtual void run();

    void broadcastEvery(unsigned int intervalMs, RSPacketCallback callback);
    void send(RSPacket& packet);

protected:
    uint8_t localID;
    const char* deviceName;

    RSPacketCallback onPacketCallback = NULL;
    RSPacketCallback broadcastCallback = NULL;
    unsigned int broadcastInterval = 1000;
    unsigned long broadcastLastMillis = 0;

    RS485Soft* rs485 = NULL;

    void runBroadcastCallback();
    bool readAndParsePacket(RSPacket& packet);
};


inline void RSNetDevice::setup(uint8_t localID, const char* deviceName)
{
    this->localID = localID;
    this->deviceName = deviceName;

    rs485->begin(RS485_DEFAULT_BAUDRATE);
}

inline void RSNetDevice::onPacket(RSPacketCallback callback)
{
    onPacketCallback = callback;
}

inline void RSNetDevice::broadcastEvery(unsigned int intervalMs, RSPacketCallback callback)
{
    broadcastInterval = intervalMs;
    broadcastCallback = callback;
}


#endif