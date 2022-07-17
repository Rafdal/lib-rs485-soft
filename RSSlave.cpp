#include "RSSlave.h"

RSSlave::RSSlave(uint8_t rxPin, uint8_t txPin, uint8_t txControl) : RSNetDevice(rxPin, txPin, txControl)
{
}

RSSlave::~RSSlave()
{
}

void RSSlave::run()
{
    if (localID == IDNotSet)
    {
        Serial.println(F("RSNetDevice ID not initialized!"));
        _delay_ms(500);
        return;
    }

    runBroadcastCallback();

    if (rs485->available())
    {
        RSPacket packet;
        if(readAndParsePacket(packet))
        {
            // (1) Look for topics
            char topic[RS485_MAX_TOPIC_SIZE];
            if(parseTopic(packet, topic))
            {
                if(listenTopics.find(topic) != listenTopics.end())
                {
                    // Execute topic callback
                    RSPacketCallback topicCallback = listenTopics[topic];
                    if(topicCallback != NULL) // this should not be null, but you know
                        topicCallback(packet);
                }
            }

            // (2) Execute packet callback if set
            if (onPacketCallback != NULL)
                onPacketCallback(packet);
        }
    }
}


bool RSSlave::parseTopic(RSPacket& packet, char topic[RS485_MAX_TOPIC_SIZE])
{
    // -> topic structure looks like:  "$TOPIC:"
    // -> RS485_MAX_TOPIC_SIZE is the len of the topic + the NULL terminator which is not in packet
    if(packet.data[0] == '$')
    {
        bool readingTopic = true;

        int tsize = 0; // topic size
        while(readingTopic)
        {
            //                                  +2 for the '$' and ':' characters
            if((tsize < (RS485_MAX_TOPIC_SIZE - 1)) && (tsize + 2) < packet.size)
            {
                char c = packet.data[tsize + 1]; // +1 for the '$'
                
                if(isalnum(c))
                {
                    topic[tsize++] = c;
                    if(tsize == (RS485_MAX_TOPIC_SIZE - 1))
                    {
                        if(packet.data[tsize + 1] == ':')   // TODO: TEST THIS CASE
                        {
                            // Ok 1
                            topic[tsize] = '\0'; // set null terminator
                            readingTopic = false;
                        }
                        else
                        {
                            // error
                            return false;
                        }
                    }
                }
                else if(c == ':')
                {
                    if(tsize > 0) // TODO: TEST THIS CASE
                    {
                        // Ok 2
                        topic[tsize] = '\0'; // set null terminator
                        readingTopic = false;
                    }
                    else
                    {
                        // error topic empty
                        return false;
                    }
                }
                else
                {
                    // error, not an alphanumeric character
                    return false;
                }
            }
            else
            {
                // size error
                return false;
            }
        }        
        
        packet.erase_front(tsize + 2);
        
        return true;
    }
    return false;
}