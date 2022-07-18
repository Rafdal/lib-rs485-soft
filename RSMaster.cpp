#include "RSMaster.h"

RSMaster::RSMaster(uint8_t rxPin, uint8_t txPin, uint8_t txControl) : RSNetDevice(rxPin, txPin, txControl)
{
}

RSMaster::~RSMaster()
{
}

bool RSMaster::sendTopic(uint8_t deviceID, const char* topic, RSPacket& packet)
{
    if(deviceID != PublicID && deviceID != MasterID && deviceID != IDNotSet && strlen(topic) <= RS485_MAX_TOPIC_SIZE)
    {
        packet.id = deviceID;
        packet.addTopic(topic);
        if(packet.error == PACKET_OK)
        {
            send(packet);
            return true;
        }
    }
    return false;
}

void RSMaster::requestTopic(uint8_t deviceID, const char *topic, RSPacketCallback callback)
{
    if (callback != NULL && requestList.size() < RSMASTER_MAX_REQUESTS)
    {
        RSPacket packet;
        packet.push_back('?'); // request byte is '?'
        if(sendTopic(deviceID, topic, packet))
        {
            TopicRequest req(deviceID, topic, callback); // this also sets a timestamp
            requestList.push_back(req);
            return;
        }
    }
    Serial.println(F("bad request"));
}

void RSMaster::loopTopics()
{
    unsigned long currentMs = millis();
    for(auto it = requestList.begin(); it != requestList.end(); it++)
    {
        if(currentMs - (*it).timestamp >= RS_REQUEST_TIMEOUT)
        {
            Serial.print(F("Request \""));
            Serial.print((*it).topic);
            Serial.println(F("\" timed out!"));
            requestList.erase(it);
            break;
        }
    }
}

void RSMaster::run()
{
    if (localID == IDNotSet)
    {
        Serial.println(F("RSNetDevice ID not initialized!"));
        _delay_ms(500);
        return;
    }

    if ( !(rs485->available()) )
    {
        runBroadcastCallback();

        loopTopics();
    }
    else
    {
        RSPacket packet;
        if (readAndParsePacket(packet))
        {
            // (1) Look for topics
            char topic[RS485_MAX_TOPIC_SIZE];
            if (packet.getTopic(topic))
            {
                for(auto it = requestList.begin(); it != requestList.end(); it++)
                {
                    if((*it).match(packet, topic) && (*it).callback != NULL)
                    {
                        packet.erase_front(strlen(topic) + 2); // remove topic

                        (*it).callback(packet);
                        requestList.erase(it);
                        return;
                    }
                }
            }

            // (2) Execute packet callback if set
            if (onPacketCallback != NULL)
                onPacketCallback(packet);
        }
    }
}