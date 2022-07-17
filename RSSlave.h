#ifndef RSSLAVE_H
#define RSSLAVE_H

#include "RSNetDevice.h"    // base class
#include <map>              // ArduinoSTL
#include <string.h>         // strlen

/**
 * @brief Slave device is listening to topic requests
 * 
 */

class RSSlave : public RSNetDevice
{
public:
    RSSlave(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
    ~RSSlave();

    /**
     * @brief Set a callback for topic-specific packets
     * 
     * @param topic (c-string) name of the topic to listen for
     * @param callback function which has (RSPacket &) as parameter 
     */
    void onTopic(const char* topic, RSPacketCallback callback);

    virtual void run();

private:

    /**
     * @brief Get the topic of the packet if it's in a valid topic format
     * 
     * @param packet RSPacket
     * @param topic external buffer with RS485_MAX_TOPIC_SIZE
     * @retval true = valid topic
     * @retval false = invalid topic or not present
     */
    bool parseTopic(RSPacket& packet, char topic[RS485_MAX_TOPIC_SIZE]);

    std::map<const char*, RSPacketCallback> listenTopics;
};

inline void RSSlave::onTopic(const char* topic, RSPacketCallback callback)
{
    size_t len = strlen(topic);
    if(RS485_MAX_TOPIC_SIZE >= len && callback != NULL)
    {
        for (unsigned int i = 0; i < len; i++)
        {
            if(!isalnum(topic[i]))
                return; // only alphanumeric topics allowed
        }
        listenTopics[topic] = callback;
    }
}


#endif