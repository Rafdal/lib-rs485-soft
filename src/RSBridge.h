#ifndef RSBridge_H
#define RSBridge_H

#include "RS485Soft.h"

// TODO: Add network children information

class RSBridge
{
public:
    RSBridge(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
    ~RSBridge();

    virtual void begin();
    virtual void run();

protected:

	virtual int bridgeAvailable() = 0;
	virtual bool readBridge(RSPacket& packet) = 0;
	virtual void sendBridge(RSPacket& packet) = 0;

private:
    RS485Soft* rs485 = NULL;
};


#endif