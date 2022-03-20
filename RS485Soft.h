#ifndef RS485_SOFT_H
#define RS485_SOFT_H

#define RS485_DEBUG				// Enable debug: print incoming bytes
#define RS485_DEBUG_TIMESTAMP	// print reading timestamps
#define RS485_SOFT_BUFFER_SIZE 32
#define RS485_DEFAULT_TIMEOUT 100 // ms

#include "ASCIIDefs.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

// readChunk() FSM states
typedef enum
{
	FSM_WAIT_H0, // Wait for first byte header
	FSM_WAIT_H1, // Wait for second byte header
	FSM_PACKET,  // Read packet data until first footer
	FSM_WAIT_F1, // Wait for end of packet
	FSM_END,     // Stop FSM
} read_state_t;


// readChunk() status codes
typedef enum
{
	ERROR_OK, // No error
	ERROR_TIMEOUT,
	ERROR_EXCEEDED_NULL_COUNT,
	ERROR_INCOMPLETE_OR_BROKEN,
	ERROR_OVERFLOW,
	ERROR_EMPTY,
	ERROR_UNKNOWN,
} error_code_t;

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

	// packet data
	uint8_t size;
	uint8_t buffer[RS485_SOFT_BUFFER_SIZE]; // data chunk

	void _flush();

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
	void sendChunk(uint8_t *data, uint8_t size);
	void sendChunk(const char *data); // sends NULL terminated string (null is not included in packet)
	error_code_t readChunk(); // read and buffer chunk of data
	uint8_t chunkSize();
	uint8_t chunkData(uint8_t i);
	uint8_t error(); // get last error code
	void clearChunk();

	// debug
	void printChunk();
};

#endif