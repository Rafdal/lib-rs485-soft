#include "RSNetDevice.h"


RSNetDevice::RSNetDevice(uint8_t rxPin, uint8_t txPin, uint8_t txControl)
{
    rs485 = new RS485Soft(rxPin, txPin, txControl); // I dont want to use inheritance here
    localID = IDNotSet;
}

RSNetDevice::~RSNetDevice()
{
    delete rs485;
}

void RSNetDevice::send(RSPacket& packet)
{
    if(packet.id == IDNotSet)
    {
        Serial.println(F("RSPacket recipient ID not set!"));
        return;
    }

    if(packet.error != PACKET_OK)
    {
        Serial.println(F("RSPacket format ERROR before sending!"));
        Serial.print(F("packet error code: "));
        Serial.println(packet.error);
        return;
    }

    // Network packet data structure: (PAYLOAD)[from][to]
    packet.push_back(localID); // from
    packet.push_back(packet.id); // to
    rs485->send(packet);
}

void RSNetDevice::run()
{
    if (localID == IDNotSet)
    {
        Serial.println(F("RSNetDevice ID not initialized!"));
        _delay_ms(500);
        return;
    }

    runIntervals();

    if (rs485->available())
    {
        if (onPacketCallback != NULL)
        {
            RSPacket packet;
            if(readAndParsePacket(packet))
                onPacketCallback(packet);
        }
    }
}

bool RSNetDevice::readAndParsePacket(RSPacket& packet)
{
    if (rs485->readPacket(packet))
    {
        // Network packet data structure = [PAYLOAD][from][to]
        uint8_t to = packet.pop_back();

        if ((to == localID || to == PublicID) && (to != IDNotSet)) // Is it for me?
        {
            packet.id = packet.pop_back(); // sender ID
            return true; // OK!
        }
    }
    else if (rs485->error())
    {
        Serial.print(F("RS485 Read Packet error code "));
        Serial.println(packet.error);
    }

    return false;
}

void RSNetDevice::runIntervals()
{
    unsigned long ms = millis();
    for(auto& i : intervals)
    {
        if(ms - i.lastMs >= i.interval)
        {
            i.callback();
            i.lastMs = ms;
        }
    }
}