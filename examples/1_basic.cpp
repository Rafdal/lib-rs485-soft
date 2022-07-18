#include <Arduino.h>

#include <RS485Soft.h>

/*
 * 	BASIC RS485 COMMUNICATION WITHOUT NETWORKING
 *
 * 	 	This is the code of the Master and Slave at the same time, to switch between them
 * 	 	you have to change the value of #define MODE
 *
 * 		The master sends the packet "Hello there!" to the slave every 2 seconds 
 * 		and the slave responds with the packet "echo!".
 *
 *   RS485 Wiring:
 *		RO -> D10
 *		DI -> D11
 *		DE -> D3
 *		RE -> D3
 */

#define MODE MASTER		   // MASTER or SLAVE
#define SEND_INTERVAL 2000 // milliseconds

// Wiring
#define RXpin 10	 // -> RO pin
#define TXpin 11	 // -> DI pin
#define controlPin 3 // -> DE and RE pin

#define MASTER 1
#define SLAVE 2

RS485Soft rs485(RXpin, TXpin, controlPin);

unsigned long lastMs;

void setup()
{
	Serial.begin(115200);
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

	if (millis() - lastMs > SEND_INTERVAL)
	{
		RSPacket packet;
		packet.load("Hello there!");
		rs485.send(packet);

		Serial.println("Sending msg");
		lastMs = millis();
	}

	if (rs485.available())
	{
		RSPacket packet;
		if (rs485.readPacket(packet))
		{
			Serial.print("Received: ");
			packet.print();
		}
		else if (rs485.error())
		{
			Serial.print("Error reading chunk code: ");
			Serial.println(packet.error);
		}
	}

#elif MODE == SLAVE
	if (rs485.available())
	{
		RSPacket packet;
		if (rs485.readPacket(packet))
		{
			if (packet.search("there"))
			{
				Serial.println("Found \"there\", sending echo");
				packet.load("echo!");
				rs485.send(packet);
			}
		}
		else if( rs485.error() )
		{
			Serial.print("Error reading chunk code: ");
			Serial.println(packet.error);
		}
	}
#endif
}