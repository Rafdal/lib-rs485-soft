#ifndef RS485_SOFT_H
#define RS485_SOFT_H

#define RS485_SOFT_BUFFER_SIZE 32
#define RS485_DEFAULT_TIMEOUT 50 // ms

#include "ASCIIDefs.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

typedef enum
{
	FSM_WAIT_H0,
	FSM_WAIT_H1,
	FSM_PACKET,
	FSM_WAIT_F1,
	FSM_END,
} read_state_t;

typedef enum
{
	ERROR_OK,
	ERROR_TIMEOUT,
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
	uint8_t txControl;

	// timeout handling
	unsigned long timeout; // timeout in ms
	unsigned long timestamp;
	void _timeStamp();
	uint8_t _timedOut();
	uint8_t errorCode; // last error code

	// packet data
	uint8_t size;
	uint8_t buffer[RS485_SOFT_BUFFER_SIZE];

	// device data
	const char *name;

public:
	RS485Soft(SoftwareSerial &softSerial, uint8_t rxPin, uint8_t txPin, uint8_t txControl);
	~RS485Soft();

	void begin(long baudrate, unsigned long timeout);
	void begin(long baudrate);

	int available();
	void write(uint8_t b);
	uint8_t read();

	void sendChunk(uint8_t *data, uint8_t size);
	void sendChunk(const char *data); // sends NULL terminated string (null is not included in packet)
	error_code_t readChunk(); // read and buffer chunk of data
	uint8_t error(); // get last error code
	void clearChunk();
	uint8_t chunkSize();
	uint8_t chunkData(uint8_t i);

	// debug
	void printChunk();

	void txMode();
	void rxMode();
};

#endif