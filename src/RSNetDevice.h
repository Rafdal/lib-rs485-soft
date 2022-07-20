#ifndef RSNetDevice_H
#define RSNetDevice_H

#include "RS485Soft.h"
#include "RSPacket.h"

#include <vector>

#define RS485_DEFAULT_BAUDRATE  9600

#if (RS485_MAX_TOPIC_SIZE + 2) >= RS485_MAX_DATA_SIZE
#error "RS485_MAX_TOPIC_SIZE >> RS485_MAX_DATA_SIZE"
#endif

#define RS485_MIN_INTERVAL_TIME 800

typedef void (*RSPacketCallback)(RSPacket&);

struct IntervalCallback
{
    IntervalCallback() : callback(NULL), interval(1000), lastMs(0) {}
    IntervalCallback(void (*c)(void), unsigned int i) : callback(c), interval(i), lastMs(0) {}
    void (*callback)(void);
    unsigned int interval;
    unsigned long lastMs;
};

class RSNetDevice
{
public:
    RSNetDevice(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
    ~RSNetDevice();

    /**
     * @brief Setup and begin device
     * 
     * @param localID device ID
     * @param deviceName c-string
     */
    void begin(uint8_t localID, const char* deviceName);

    /**
     * @brief Set callback for incoming packets for this device
     * 
     * @param callback callback with (RSPacket&) as a parameter
     */
    void onPacket(RSPacketCallback callback);

    /**
     * @brief Set interval callback
     * 
     * @param interval time period in milliseconds to periodically call back the function
     * @param callback void function(void) to be called
     */
    void setInterval(unsigned int interval, void (*callback)(void));

    /**
     * @brief Listen for new packets, execute loops and callbacks (non-blocking)
     */
    virtual void run();

    /**
     * @brief Send a packet to specific device
     * 
     * @param packet RSPacket (Recipient and Local ID must be set)
     */
    void send(RSPacket& packet);


protected:
    uint8_t localID;
    const char* deviceName;

    RSPacketCallback onPacketCallback = NULL;
    RSPacketCallback broadcastCallback = NULL;
    unsigned int broadcastInterval = 1500;
    unsigned long broadcastLastMillis = 0;

    RS485Soft* rs485 = NULL;

    /**
     * @brief Get packet and extract recipient and sender information
     * 
     * @param packet 
     * @retval true = Its for me
     * @retval false = It's not for me or BAD Format
     */
    bool readAndParsePacket(RSPacket& packet);
    
    std::vector<IntervalCallback> intervals;
    void runIntervals();
};

inline void RSNetDevice::setInterval(unsigned int interval, void (*callback)(void))
{
    if(callback != NULL && interval > RS485_MIN_INTERVAL_TIME)
        intervals.push_back( IntervalCallback(callback, interval) );
}

inline void RSNetDevice::begin(uint8_t localID, const char* deviceName)
{
    this->localID = localID;
    this->deviceName = deviceName;

    rs485->begin(RS485_DEFAULT_BAUDRATE);
}

inline void RSNetDevice::onPacket(RSPacketCallback callback)
{
    onPacketCallback = callback;
}


#endif