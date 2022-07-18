#ifndef RSMASTER_H
#define RSMASTER_H

#include "RSNetDevice.h"
#include <Arduino.h>
#include <list>
#include <vector>

#define RSMASTER_MAX_REQUESTS   8
#define RS_REQUEST_TIMEOUT      500    // [ms]    
#define RS_MAX_FAILED_ATTEMPS   4

struct TopicRequest
{
    uint8_t deviceID;
    char* topic;
    RSPacketCallback callback;
    unsigned long timestamp;
    TopicRequest()
    {
        deviceID = IDNotSet;
        topic = NULL;
        callback = NULL;
        timestamp = 0;
    }
    TopicRequest(uint8_t id, const char* t, RSPacketCallback c)
    {
        deviceID = id;
        topic = const_cast<char*>(t);
        callback = c;
        timestamp = millis();
    }
    bool match(RSPacket& p, char* t)
    {
        if(deviceID == p.id && strcmp(topic, t) == 0)
            return true;
        return false;
    }
};

struct FailedAttemp
{
    FailedAttemp()
    {
        id = 0;
        fails = 0;
    }
    uint8_t id;
    uint8_t fails;
};

class RSMaster : public RSNetDevice
{
public:
    RSMaster(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
    ~RSMaster();

    void requestTopic(uint8_t deviceID, const char* topic, RSPacketCallback callback);
    
    bool sendTopic(uint8_t deviceID, const char* topic, RSPacket& packet);

    virtual void run();

    void begin();

private:
    using RSNetDevice::setup;

    std::list<TopicRequest> requestList;
    std::vector<

    void loopTopics();
};

inline void RSMaster::begin()
{
    RSNetDevice::setup(MasterID, "master");
}


#endif