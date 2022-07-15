#ifndef RSNetDevice_H
#define RSNetDevice_H

#include "RS485Soft.h"
#include "RSPacket.h"

#define RS485_DEFAULT_BAUDRATE  9600

typedef void (*RSPacketCallback)(RSPacket&);
typedef RSPacket (*RSBroadcastCallback)(void);

enum ReservedIDs
{
    IDNotSet = 253,
    PublicID,
    MasterID,
};

class RSNetDevice : private RS485Soft
{
public:
    RSNetDevice(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
    ~RSNetDevice();

    virtual void setup(uint8_t localID, const char* deviceName);
    virtual void onPacket(RSPacketCallback callback);
    virtual void run();

    void broadcastEvery(RSBroadcastCallback callback, unsigned int intervalMs);

    virtual void send(RSPacket& packet);

private:
    uint8_t localID;
    const char* deviceName;

    RSPacketCallback onPacketCallback = NULL;
    RSBroadcastCallback broadcastCallback = NULL;
    unsigned int broadcastInterval = 1000;
    unsigned long broadcastLastMillis = 0;
};


inline void RSNetDevice::setup(uint8_t localID, const char* deviceName)
{
    this->localID = localID;
    this->deviceName = deviceName;

    begin(RS485_DEFAULT_BAUDRATE);
}

inline void RSNetDevice::onPacket(RSPacketCallback callback)
{
    onPacketCallback = callback;
}

inline void RSNetDevice::broadcastEvery(RSBroadcastCallback callback, unsigned int intervalMs)
{
    broadcastInterval = intervalMs;
    broadcastCallback = callback;
}


#endif