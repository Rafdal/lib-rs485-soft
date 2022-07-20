#ifndef RSMASTER_H
#define RSMASTER_H

#include "RSNetDevice.h"
#include <Arduino.h>
#include <list>

#define RSMASTER_MAX_REQUESTS   8
#define RS_DEFAULT_REQUEST_TIMEOUT      500    // [ms]    
#define RS_MAX_FAILED_ATTEMPS   4

struct TopicRequest;
struct FailedAttemp;

class RSMaster : public RSNetDevice
{
public:
    RSMaster(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
    ~RSMaster();

    /**
     * @brief Setup and begin device
     */
    void begin();

    /**
     * @brief Request a topic to some device and set a callback for the expected response
     * 
     * @param deviceID ID of recipient device
     * @param topic char*
     * @param callback Function to execute when the response arrives
     */
    void requestTopicTo(uint8_t deviceID, const char* topic, RSPacketCallback callback);
    
    /**
     * @brief Set a Timeout in milliseconds for topic requests
     * 
     * @param tout (unsigned int) time [ms]
     */
    void setRequestTimeout(unsigned int tout);

    /**
     * @brief Send a packet with a specific topic on it
     * 
     * @param deviceID recipient device ID
     * @param topic char*
     * @param packet packet with payload to send
     * @retval true = OK 
     * @retval false = Format ERROR (packet not sent)
     */
    bool sendTopic(uint8_t deviceID, const char* topic, RSPacket& packet);

    /**
     * @brief Set a function to call back when some device is not responding after some tries defined in RS_MAX_FAILED_ATTEMPS
     * 
     * @param callback void function(uint8_t deviceID)
     */
    void onDeviceNotResponding(void (*callback)(uint8_t deviceID));

    /**
     * @brief Listen for new packets, execute loops and callbacks (non-blocking)
     * 
     */
    virtual void run();


private:
    using RSNetDevice::begin;

    std::list<TopicRequest> requestList;
    void loopTopics();

    unsigned int requestTimeout = RS_DEFAULT_REQUEST_TIMEOUT;
    std::list<FailedAttemp> failAttempts;
    void resetFails(uint8_t id);
    uint8_t incrementFails(uint8_t id);
    FailedAttemp* getAttemptListReference(uint8_t id);

    void (*deviceNotRespondingCallback)(uint8_t deviceID) = NULL;
};

inline void RSMaster::setRequestTimeout(unsigned int tout)
{
    requestTimeout = tout;
}

inline void RSMaster::onDeviceNotResponding(void (*callback)(uint8_t))
{
    deviceNotRespondingCallback = callback;
}

inline void RSMaster::begin()
{
    RSNetDevice::begin(MasterID, "master");
}


#endif