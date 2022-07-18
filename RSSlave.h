#ifndef RSSLAVE_H
#define RSSLAVE_H

#include "RSNetDevice.h"    // base class
#include <vector>              // ArduinoSTL
#include <string.h>         // strlen

/**
 * @brief Slave device is listening to topic requests
 * 
 */

struct ListenTopic
{
    bool answer;
    const char* topic;
    RSPacketCallback callback;
    ListenTopic(const char* t, RSPacketCallback c, bool a = false)
    {
        topic = t;
        callback = c;
        answer = a;
    }
};

class RSSlave : public RSNetDevice
{
public:
    RSSlave(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
    virtual ~RSSlave();

    /**
     * @brief Set a callback for topic-specific packets
     * 
     * @param topic (c-string) name of the topic to listen for
     * @param callback function which has (RSPacket &) as parameter 
     */
    void onTopic(const char* topic, RSPacketCallback callback);

    /**
     * @brief Set a callback for topic-specific packets to be answered
     * 
     * @param topic (c-string) name of the topic to listen for
     * @param callback function which has (RSPacket &) as parameter 
     */
    void onTopicAnswer(const char* topic, RSPacketCallback callback);

    virtual void run();

private:

    // I've had problems with the map STL, so I use vector as a shitty workaround
    std::vector<ListenTopic> listenTopics; 

    void addListenTopic(ListenTopic& listenTopic);
};


#endif