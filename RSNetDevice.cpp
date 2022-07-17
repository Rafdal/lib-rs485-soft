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
        Serial.println(F("RSPacket receiver not set!"));
        _delay_ms(500);
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

    runBroadcastCallback();

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

        if (to == localID || to == PublicID) // It's for me?
        {
            packet.id = packet.pop_back(); // sender ID
            return true; // OK!
        }
    }
    else if (rs485->error())
    {
        Serial.print(F("RS485 Error code "));
        Serial.println(packet.error);
    }

    return false;
}

void RSNetDevice::runBroadcastCallback()
{
    if (broadcastCallback != NULL)
    {
        if (millis() - broadcastLastMillis >= broadcastInterval)
        {
            RSPacket packet;
            broadcastCallback(packet);
            send(packet);

            broadcastLastMillis = millis();
        }
    }
}
