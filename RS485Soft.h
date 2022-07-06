#ifndef RS485_SOFT_H
#define RS485_SOFT_H

// #define RS485_DEBUG				// Enable debug: print incoming bytes
// #define RS485_DEBUG_TIMESTAMP	// print reading timestamps

#define RS485_DEFAULT_TIMEOUT 50 // ms

#include "ASCIIDefs.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "RSPacket.h"

// readChunk() FSM states
enum RS485State
{
	FSM_WAIT_H0, // Wait for first byte header
	FSM_WAIT_H1, // Wait for second byte header
	FSM_SIZE,    // Wait for size byte
	FSM_PACKET,  // Read packet data until first footer
	FSM_HASH,	 // Wait for hash byte
	FSM_WAIT_F1, // Wait for end of packet
	FSM_END,     // Stop FSM
};


class RS485Soft : public SoftwareSerial
{
public:
	RS485Soft(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
	~RS485Soft();

	virtual void begin(long speed);
	virtual int available();

	// packet handling
	bool readPacket(RSPacket& packet); // read and save data
	void send(RSPacket& packet);

private:
	void txMode();
	void rxMode();

	uint8_t rxPin;
	uint8_t txPin;
	uint8_t txControl; // Half-Duplex direction pin

	// timeout handling
	unsigned long timeout = RS485_DEFAULT_TIMEOUT; 	 // timeout in ms
	unsigned long timestamp;
	void _timeStamp();
	uint8_t _timedOut();
	uint8_t errorCode; // last error code
	uint8_t _getShitty8BitCRC(uint8_t* data, uint8_t size);

};

#endif