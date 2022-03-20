#include <Arduino.h>

#include <SoftwareSerial.h>
#include <RS485Soft.h>

/* 
 *   RS485 Wiring:
 *		RO -> D10
 *		DI -> D11
 *		DE -> D3
 *		RE -> D3
 */

#define RXpin 10		// -> RO pin
#define TXpin 11		// -> DI pin
#define controlPin 3	// -> DE and RE pin

#define MASTER 1
#define SLAVE 2

#define MODE MASTER			// MASTER or SLAVE
#define SEND_INTERVAL 2000	// ms

SoftwareSerial serial(RXpin, TXpin);
RS485Soft rs485(serial, RXpin, TXpin, controlPin);

unsigned long lastMs;

void setup()
{
	Serial.begin(9600);
	rs485.begin(9600);

	#if MODE == MASTER
	Serial.println("Testing MASTER!");
	#elif MODE == SLAVE
	Serial.println("Testing SLAVE!");
	#endif
}

void loop()
{
	#if MODE == MASTER
	if(millis() - lastMs > SEND_INTERVAL)
	{
		Serial.println("Sending msg");
		rs485.sendChunk("t");
		lastMs = millis();
	}
	

	if(rs485.available())
	{
		if(rs485.readChunk() == ERROR_OK)
		{
			rs485.printChunk();
			rs485.clearChunk();
		}
		else
		{
			Serial.print("Error reading chunk code: ");
			Serial.println( rs485.error() );
		}
	}
	#elif MODE == SLAVE
	if(rs485.available())
	{
		if(rs485.readChunk() == ERROR_OK)
		{
			if(rs485.chunkData(0) == 't')
			{
				char msg[] = "echo!";
				rs485.sendChunk((uint8_t*)msg, 5);
			}
		}
		else
		{
			Serial.print("Error reading chunk code: ");
			Serial.println( rs485.error() );
		}
	}
	#endif
}