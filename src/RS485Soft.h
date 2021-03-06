#ifndef RS485_SOFT_H
#define RS485_SOFT_H

// #define RS485_DEBUG				// Enable debug: print incoming bytes
// #define RS485_DEBUG_TIMESTAMP	// print reading timestamps

#define RS485_DEFAULT_TIMEOUT 50 // ms

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "RSPacket.h"

// FSM states
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

class RS485Soft : private SoftwareSerial
{
public:
	RS485Soft(uint8_t rxPin, uint8_t txPin, uint8_t txControl);
	virtual ~RS485Soft();

	/**
	 * @brief Begin communication with the RS485 module via Software Serial
	 * 
	 * @param speed baudrate
	 */
	virtual void begin(long speed);

	/**
	 * @brief Check if we have new incoming bytes
	 * @return int >0 if available
	 */
	virtual int available();

	/**
	 * @brief Read packet from serial buffer
	 * 
	 * @param packet 
	 * @retval true = read OK
	 * @retval false = ERROR or NULL packet
	 */
	virtual bool readPacket(RSPacket& packet);

	/**
	 * @brief Check for true error
	 * 
	 * @retval true ERROR
	 * @retval false Not an error (NULL packet) or OK
	 */
	virtual bool error();

	/**
	 * @brief Send a packet via RS485 module
	 * 
	 * @param packet RSPacket with <data> and <size>
	 */
	virtual void send(RSPacket& packet);


private:
	void txMode();
	void rxMode();

	uint8_t rxPin;
	uint8_t txPin;
	uint8_t txControl; // Half-Duplex direction pin

	bool errorFlag;

	// timeout handling
	unsigned long timeout = RS485_DEFAULT_TIMEOUT; 	 // timeout in ms
	unsigned long timestamp;
	void timeStamp(); // update timestamp

	/*
	* @brief Watch for timeout
	* 
	* @retval 0 Not timed out
	* @retval 1 Time Out!
	*/
	uint8_t timedOut();

};

// Inlined methods

inline bool RS485Soft::error()
{
	return errorFlag;
}

inline void RS485Soft::txMode()
{
	digitalWrite(txControl, HIGH);
}

inline void RS485Soft::rxMode()
{
	digitalWrite(txControl, LOW);
}


inline void RS485Soft::timeStamp()
{
	timestamp = millis();
}

inline uint8_t RS485Soft::timedOut()
{
	return (millis() - timestamp) > timeout;
}

#endif