#include <Arduino.h>


/*
 * 	RS485 COMMUNICATION WITH MASTER-SLAVE NETWORKING AND TOPICS
 * 
 * 	 	This is the code of the Master and Slave at the same time, to switch between them
 * 	 	you have to change the value of #define MODE
 *
 *   RS485 Wiring:
 *		RO -> D10
 *		DI -> D11
 *		DE -> D3
 *		RE -> D3
 */

// options
#define MODE 	MASTER		// > set MASTER or SLAVE <
#define SEND_INTERVAL 2000 	// milliseconds (master only)

// Wiring
#define RXpin 10	 // -> RO pin
#define TXpin 11	 // -> DI pin
#define controlPin 3 // -> DE and RE pin

#define SLAVE_ID	0
#define SLAVE_NAME	"Slave"



#define MASTER 0
#define SLAVE 1

#if MODE == MASTER

#include <RSMaster.h>
RSMaster device(RXpin, TXpin, controlPin);
#define DEVICE_NAME	"Master"

#elif MODE == SLAVE

// #include <RSNetDevice.h>
#include <RSSlave.h>
RSSlave device(RXpin, TXpin, controlPin);
#define DEVICE_NAME	"Slave"

#endif



void setup()
{
	Serial.begin(115200);
	Serial.println(F("Testing " DEVICE_NAME));

#if MODE == MASTER

	device.begin();

	// Request topic "test" every SEND_INTERVAL milliseconds
	device.setInterval(SEND_INTERVAL, [](){

		device.requestTopicTo(2, "test", [](RSPacket& packet){
			// execute this callback when we receive the answer to the request (async)
			Serial.println(F("Received \"test\" response!"));
			packet.print();
			Serial.println();
		});
	});

	/*
	 * Set a function to manage not-responding devices
	 * 		This function is executed when a single device 
	 * 		exceeds RS_REQUEST_TIMEOUT and RS_MAX_FAILED_ATTEMPS
	 */
	device.onDeviceNotResponding([](uint8_t id){
		Serial.print(F("Device not responding:\tdeviceID = "));
		Serial.println(id);
	});

	// Set a callback for packets with unhandled topics or without topics
	device.onPacket([](RSPacket& packet){
		Serial.println(F("@ Received Packet"));
		packet.print();
	});

#elif MODE == SLAVE

	// Begin slave with device ID and name
	device.begin(SLAVE_ID, SLAVE_NAME);

	// Specify how to handle specific topic and then answer it
	device.onTopicAnswer("test", [](RSPacket& packet){
		Serial.println(F("Received topic \"test\""));
		// print received packet
		packet.print();

		// answer the topic
		packet.clear();
		packet.load("hola!");
		Serial.println();
		// the packet is sent with this topic when this callback ends
	});

	// Set a callback for packets with unhandled topics or without topics
	device.onPacket([](RSPacket& packet){
		Serial.println(F("Received unhandled packet"));
		packet.print();
		Serial.println();
	});

#endif

}

void loop()
{
	device.run(); // listen for packets and run loops or callbacks 
}