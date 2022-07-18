#include "RSSlave.h"

RSSlave::RSSlave(uint8_t rxPin, uint8_t txPin, uint8_t txControl) : RSNetDevice(rxPin, txPin, txControl)
{
}

RSSlave::~RSSlave()
{
}

void RSSlave::addListenTopic(ListenTopic& listen)
{
    size_t len = strlen(listen.topic);
    if(RS485_MAX_TOPIC_SIZE >= len && listen.callback != NULL)
    {
        for (unsigned int i = 0; i < len; i++)
        {
            if(!isalnum(listen.topic[i]))
            {
                Serial.println(F("bad topic"));
                return; // only alphanumeric topics allowed
            }
        }

        listenTopics.push_back(listen);
    }
}

void RSSlave::onTopicAnswer(const char* topic, RSPacketCallback callback)
{
    ListenTopic listen(topic, callback, true);
    addListenTopic(listen);
}

void RSSlave::onTopic(const char* topic, RSPacketCallback callback)
{
    ListenTopic listen(topic, callback, false);
    addListenTopic(listen);
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
            memset(topic, 0, RS485_MAX_TOPIC_SIZE);
            if(packet.getTopic(topic))
            {
                for(auto& t : listenTopics)
                {
                    if(strcmp(t.topic, topic) == 0)
                    {
                        // Execute topic callback
                        if(t.callback != NULL) // this should not be null, but you know...
                        {
                            packet.erase_front(strlen(topic) + 2); // remove topic
            
                            t.callback(packet);

                            if(t.answer)
                            {
                                packet.addTopic(topic);
                                send(packet);
                            }
                            return; // ok
                        }
                        break;
                    }
                }
            }

            // (2) Execute packet callback if set
            if (onPacketCallback != NULL)
                onPacketCallback(packet);
        }
    }
}


