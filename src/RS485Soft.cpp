#include "RS485Soft.h"

RS485Soft::RS485Soft(uint8_t rxPin, uint8_t txPin, uint8_t txControl) : SoftwareSerial(rxPin, txPin)
{
	this->rxPin = rxPin;
	this->txPin = txPin;
	this->txControl = txControl;

	errorFlag = false;
}

RS485Soft::~RS485Soft()
{
	SoftwareSerial::stopListening();
	SoftwareSerial::end();
}


void RS485Soft::begin(long baudrate)
{
	SoftwareSerial::begin(baudrate);
	pinMode(txControl, OUTPUT);
}

int RS485Soft::available()
{
	rxMode();
	return SoftwareSerial::available();
}


void RS485Soft::send(RSPacket& packet)
{
	txMode();

	// Sync Header (Literally "HI")
	write((uint8_t)0); // start with a null packet
	write('H');
	write('I');
	write(packet.size);

	for (int i = 0; i < packet.size && i < RS485_MAX_DATA_SIZE; i++)
		write(packet.data[i]);	

	// Footer
	write(packet.hash());
	write((uint8_t)0); // end with a null packet
}


bool RS485Soft::readPacket(RSPacket& packet)
{
	#ifdef RS485_DEBUG
	#ifdef RS485_DEBUG_TIMESTAMP
	Serial.print("\n@ packet (t = ");
	Serial.print(millis() - timestamp);
	Serial.print(")\t\t");
	#endif

	uint8_t debug_output[256];
	uint8_t debug_output_count = 0;

	#endif

	rxMode();
	timeStamp();
	packet.clear();

	errorFlag = true;

	uint8_t sizeCounter = 0;

	uint8_t state = FSM_WAIT_H0;
	bool packetComplete = false;
	while (state != FSM_END)
	{
		if (timedOut())
		{
			packet.error = ERROR_TIMEOUT;
			state = FSM_END;
		}

		if (SoftwareSerial::available())
		{
			uint8_t b = read(); // read one byte at a time

			#ifdef RS485_DEBUG
			debug_output[debug_output_count++] = b;
			#endif

			switch (state) // FINITE STATE MACHINE
			{
			case FSM_WAIT_H0: 
				switch (b)
				{
				case 'H': // First byte header
					timeStamp();
					state = FSM_WAIT_H1; // OK, next state
					break;
				case 0: // Ignore first NULL bytes
					packet.error = NULL_PACKET;
					errorFlag = false; // NULL packets are not errors!
					state = FSM_END;			
					break;
				}
				break;

			case FSM_WAIT_H1:
				if(b == 'I') // Second byte header
				{
					timeStamp();
					state = FSM_SIZE; // OK, next state
				}
				else
				{
					packet.error = ERROR_HEADER_BROKEN;
					state = FSM_END;
				}
				break;

			case FSM_SIZE:
				packet.size = b;
				if(packet.size <= RS485_MAX_DATA_SIZE)
					state = FSM_PACKET; // Ok next
				else
				{
					packet.error = ERROR_SIZE_OVERFLOW;
					state = FSM_END;
				}
				break;

			case FSM_PACKET:
				packet.data[sizeCounter++] = b;
				if( sizeCounter >= packet.size || sizeCounter >= RS485_MAX_DATA_SIZE)
					state = FSM_HASH;

				break;

			case FSM_HASH:
				if(packet.hash() == b)
				{
					state = FSM_END;
					packet.error = PACKET_OK;
					errorFlag = false;
					delay(10);
					SoftwareSerial::read();
					SoftwareSerial::read();
					packetComplete = true;
				}
				else
				{
					packet.error = ERROR_HASH_MISMATCH;
					state = FSM_END;
				}
				break;

			case FSM_END:
				break;

			default:
				packet.error = ERROR_UNKNOWN;
				state = FSM_END;
				break;
			}
		}
	}

	#ifdef RS485_DEBUG
	switch (packet.error)
	{
	case PACKET_OK:
		break;
	case ERROR_TIMEOUT:
		Serial.print(F("ERROR_TIMEOUT: timeout of (ms) "));
		Serial.println(millis() - timestamp);
		break;

	case NULL_PACKET:
		Serial.print(F("NULL_PACKET: timestamp = "));
		Serial.println(millis() - timestamp);
		break;

	case ERROR_HEADER_BROKEN:
		Serial.print(F("ERROR_HEADER_BROKEN"));
		break;

	case ERROR_HASH_MISMATCH:
		Serial.print(F("ERROR_HASH_MISMATCH"));
		break;

	default:
		Serial.print(F("ERROR_CODE: "));
		Serial.println(packet.error);
		break;
	}
	Serial.print(F("RS485 received bytes: "));
	for (int i = 0; i < debug_output_count; i++)
	{
		char str[5];
		sprintf(str, "%02X ", debug_output[i]);
		Serial.print(str);
	}
	Serial.println();
	packet.print();

	Serial.println("\n");
	#endif

	return packetComplete;
}