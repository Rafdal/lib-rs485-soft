#include "RSNetDevice.h"


RSNetDevice::RSNetDevice(uint8_t rxPin, uint8_t txPin, uint8_t txControl) : RS485Soft(rxPin, txPin, txControl)
{
    localID = IDNotSet;
}

RSNetDevice::~RSNetDevice()
{
}

void RSNetDevice::send(RSPacket& packet)
{
    if(packet.id == IDNotSet)
    {
        Serial.println(F("RSPacket receiver not set!"));
        _delay_ms(500);
        return;
    }

    // Network packet data structure: (PAYLOAD)[from][to]
    packet.push_back(localID); // from
    packet.push_back(packet.id); // to
    RS485Soft::send(packet);
}

void RSNetDevice::run()
{
    if(localID == IDNotSet)
    {
        Serial.println(F("RSNetDevice ID not initialized!"));
        _delay_ms(500);
        return;
    }

    if( broadcastCallback != NULL)
    {
        if(millis() - broadcastLastMillis >= broadcastInterval)
        {
            this->send( broadcastCallback() );

            broadcastLastMillis = millis();
        }
    }

    if( available() )
    {
        RSPacket packet;
        if( readPacket(packet) )
        {
            // Network packet data structure = [PAYLOAD][from][to]
            uint8_t to = packet.pop_back();

            if(to == localID || to == PublicID) // It's for me?
            {
                packet.id = packet.pop_back(); // sender ID

                if(onPacketCallback != NULL)
                    onPacketCallback(packet);
            }
        }
        else if( error() )
        {
            Serial.print(F("RS485 Error code "));
            Serial.println(packet.error);
        }
    }
}