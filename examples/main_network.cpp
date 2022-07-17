#include <Arduino.h>

#include <RSNetDevice.h>

/*
 * 	RS485 COMMUNICATION WITH NETWORKING
 *
 * 
 * 	 	This is the code of the Master and Slave at the same time, to switch between them
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

// options
#define MODE MASTER		   // > set MASTER or SLAVE <
#define SEND_INTERVAL 2000 // milliseconds

// Wiring
#define RXpin 10	 // -> RO pin
#define TXpin 11	 // -> DI pin
#define controlPin 3 // -> DE and RE pin



#define MASTER 0
#define SLAVE 1

#if MODE == MASTER
#define DEVICE_ID	MasterID
#define DEVICE_NAME	"Master"
#elif MODE == SLAVE
#define DEVICE_ID	0
#define DEVICE_NAME	"Slave"
#endif



RSNetDevice device(RXpin, TXpin, controlPin);

void setup()
{
	Serial.begin(115200);
	Serial.println(F("Testing " DEVICE_NAME));

	device.setup(DEVICE_ID, DEVICE_NAME);


#if MODE == MASTER

	device.onPacket([](RSPacket& packet){
		Serial.println(F("@ Received Packet"));
		packet.print();
	});
	
	// set an automated packet broadcasting every <SEND_INTERVAL> ms
	device.broadcastEvery(SEND_INTERVAL, [](RSPacket& packet){
		packet.id = PublicID; // send to everyone
		packet.push_back('t');
		Serial.println(F("\nBroadcasting..."));
	});

#elif MODE == SLAVE

	device.onPacket([](RSPacket& packet){
		Serial.println(F("@ Received Packet"));
		packet.print();

		if (packet.search("t"))
		{
			Serial.println(F("Found 't', sending echo"));
			packet.load("echo!");
			device.send(packet); // no need to change address since id = sender
		}
	});

#endif

}

void loop()
{
	device.run();
}