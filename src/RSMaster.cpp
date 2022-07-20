#include "RSMaster.h"

RSMaster::RSMaster(uint8_t rxPin, uint8_t txPin, uint8_t txControl) : RSNetDevice(rxPin, txPin, txControl)
{
}

RSMaster::~RSMaster()
{
}

struct TopicRequest
{
    uint8_t deviceID;
    char* topic;
    RSPacketCallback callback;
    unsigned long timestamp;
    TopicRequest() : deviceID(IDNotSet), topic(NULL), callback(NULL), timestamp(0) {}
    TopicRequest(uint8_t id, const char* t, RSPacketCallback c) : deviceID(id), callback(c)
    {
        topic = const_cast<char*>(t);
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
    FailedAttemp() : id(IDNotSet), count(0) {}
    FailedAttemp(uint8_t id) : id(id), count(0) {} 
    uint8_t id;
    uint8_t count;
};

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

void RSMaster::requestTopicTo(uint8_t deviceID, const char *topic, RSPacketCallback callback)
{
    if (callback != NULL && requestList.size() < RSMASTER_MAX_REQUESTS)
    {
        RSPacket packet;
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
            Serial.print(F("\" to id="));
            Serial.print((*it).deviceID);
            Serial.println(F(" timed out!"));

            uint8_t fails = incrementFails( (*it).deviceID );
            if( fails > RS_MAX_FAILED_ATTEMPS && deviceNotRespondingCallback != NULL)
                deviceNotRespondingCallback( (*it).deviceID );

            requestList.erase(it);
            break;
        }
    }
}

FailedAttemp* RSMaster::getAttemptListReference(uint8_t id)
{
    for(auto& att : failAttempts)
    {
        if(att.id == id)
            return &att;
    }
    // if it doesn't exists in the vector
    failAttempts.push_back(FailedAttemp(id)); // insert it
    return &(failAttempts.back());
}

void RSMaster::resetFails(uint8_t id)
{
    FailedAttemp* patt = getAttemptListReference(id);
    patt->count = 0;
}

uint8_t RSMaster::incrementFails(uint8_t id)
{
    FailedAttemp* patt = getAttemptListReference(id);
    return patt->count++;
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
        runIntervals();

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
                        resetFails( (*it).deviceID );
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