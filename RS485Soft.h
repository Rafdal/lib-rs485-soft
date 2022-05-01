#ifndef RS485_SOFT_H
#define RS485_SOFT_H

// #define RS485_DEBUG				// Enable debug: print incoming bytes
// #define RS485_DEBUG_TIMESTAMP	// print reading timestamps

#define RS485_SOFT_BUFFER_SIZE 32
#define RS485_DEFAULT_TIMEOUT 50 // ms

#include "ASCIIDefs.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

// readChunk() FSM states
typedef enum
{
	FSM_WAIT_H0, // Wait for first byte header
	FSM_WAIT_H1, // Wait for second byte header
	FSM_WAIT_LEN,// Wait for len packet
	FSM_PACKET,  // Read packet
	FSM_WAIT_F1, // Check the end of packet
	FSM_WAIT_CRC,// Wait for CRC byte
	FSM_END,     // Stop FSM
} read_fsm_state_t;


// readChunk() status codes
typedef enum
{
	NEW_PACKET, // No error
	NULL_PACKET, // No packet
	ERROR_TIMEOUT,
	ERROR_EXCEEDED_NULL_COUNT,
	ERROR_INCOMPLETE_OR_BROKEN,
	ERROR_OVERFLOW,
	ERROR_EMPTY,
	ERROR_CRC_MISMATCH,
	ERROR_UNKNOWN,
} read_code_t;

class RS485Soft
{
private:
	SoftwareSerial *rs485;
	uint8_t rxPin;
	uint8_t txPin;
	uint8_t txControl; // Half-Duplex direction pin

	// timeout handling
	unsigned long timeout; 	 // timeout in ms
	unsigned long timestamp;
	void _timeStamp();
	uint8_t _timedOut();
	uint8_t errorCode; // last error code
	uint8_t _getShitty8BitCRC(uint8_t* data, uint8_t size);

	// packet data
	uint8_t size;
	uint8_t buffer[RS485_SOFT_BUFFER_SIZE]; // data chunk

public:
	RS485Soft(SoftwareSerial &softSerial, uint8_t rxPin, uint8_t txPin, uint8_t txControl);
	~RS485Soft();

	void begin(long baudrate, unsigned long timeout);
	void begin(long baudrate);

	int available();
	void write(uint8_t b);
	uint8_t read();

	void txMode();
	void rxMode();

	// chunk handling
	bool sendChunk(uint8_t *data, uint8_t size);
	bool sendChunk(const char *data); // sends NULL terminated string (null is not included in packet)
	read_code_t readChunk(); // read and buffer chunk of data
	uint8_t chunkSize();
	uint8_t chunkData(uint8_t i);
	uint8_t error(); // get error code
	void clearChunk();

	// debug
	void printChunk();
};

#endif