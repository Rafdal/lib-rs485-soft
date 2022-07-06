#include <Arduino.h>

#include <SoftwareSerial.h>
#include <RS485Soft.h>

/* 
 *	 	This is the code of the Master and Slave at the same time, to switch between them
 * 	 	you have to change the value of #define MODE
 * 
 * 		The master sends the packet "t" to the slave every 2 seconds and the slave responds with the packet "echo".
 * 
 *   RS485 Wiring:
 *		RO -> D10
 *		DI -> D11
 *		DE -> D3
 *		RE -> D3
 */

#define MODE SLAVE			// MASTER or SLAVE
#define SEND_INTERVAL 2000	// milliseconds

// Wiring
#define RXpin 10		// -> RO pin
#define TXpin 11		// -> DI pin
#define controlPin 3	// -> DE and RE pin

#define MASTER 1
#define SLAVE 2

#error THIS DOESNT WORK

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
		if(rs485.readChunk() == NEW_PACKET)
		{
			Serial.print("Received: ");
			rs485.printChunk();
		}
		else if (rs485.error())
		{
			Serial.print("Error reading chunk code: ");
			Serial.println( rs485.error() );
		}
	}
	#elif MODE == SLAVE
	if(rs485.available())
	{
		if(rs485.readChunk() == NEW_PACKET)
		{
			if(rs485.chunkData(0) == 't')
			{
				Serial.println("Received 't', sending echo");
				rs485.sendChunk("echo!");
			}
		}
		else if (rs485.error())
		{
			Serial.print("Error reading chunk code: ");
			Serial.println( rs485.error() );
		}
	}
	#endif
}