#include "RS485Soft.h"

RS485Soft::RS485Soft(uint8_t rxPin, uint8_t txPin, uint8_t txControl) : SoftwareSerial(rxPin, txPin)
{
	this->rxPin = rxPin;
	this->txPin = txPin;
	this->txControl = txControl;
}

RS485Soft::~RS485Soft()
{
	SoftwareSerial::stopListening();
	SoftwareSerial::end();
}

void RS485Soft::send(RSPacket& packet)
{
	txMode();

	// Sync Header
	write(ASCII_NUL);
	write(ASCII_SOH);
	write(ASCII_STX);
	write(packet.size);

	for (int i = 0; i < packet.size && i < RS485_MAX_DATA_SIZE; i++)
		write(packet.data[i]);	

	// Footer
	write(packet.hash());
	write(ASCII_NUL);
}

void RS485Soft::_timeStamp()
{
	timestamp = millis();
}


/*
 * @brief Watch for timeout
 * 
 * @retval 0 Not timed out
 * @retval 1 Time Out!
 */
uint8_t RS485Soft::_timedOut()
{
	return (millis() - timestamp) > timeout;
}

bool RS485Soft::readPacket(RSPacket& packet)
{
	#ifdef RS485_DEBUG
	#ifdef RS485_DEBUG_TIMESTAMP
	Serial.print("t: ");
	Serial.print(millis() - timestamp);
	Serial.print("\t\t");
	#endif

	uint8_t last_state = 0;
	uint8_t debug_output[256];
	uint8_t debug_output_count = 0;

	#endif

	rxMode();
	_timeStamp();
	packet.clear();

	uint8_t i = 0; // size counter

	uint8_t state = FSM_WAIT_H0;
	while (state != FSM_END)
	{
		if (_timedOut())
		{
			packet.error = ERROR_TIMEOUT;
			return false;
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
				case ASCII_SOH: // First byte header
					_timeStamp();
					state = FSM_WAIT_H1; // OK, next state
					break;
				case ASCII_NUL: // Ignore first NULL bytes
					packet.error = NULL_PACKET;
					return false;				
					break;
				}
				break;

			case FSM_WAIT_H1:
				if(b == ASCII_STX) // Second byte header
				{
					_timeStamp();
					state = FSM_SIZE; // OK, next state
				}
				else
				{
					packet.error = ERROR_HEADER_BROKEN;
					return false;
				}
				break;

			case FSM_SIZE:
				packet.size = b;
				if(packet.size <= RS485_MAX_DATA_SIZE)
					state = FSM_PACKET; // Ok next
				else
				{
					packet.error = ERROR_SIZE_OVERFLOW;
					return false;
				}
				break;

			case FSM_PACKET:
				packet.data[i++] = b;
				if( i >= packet.size || i >= RS485_MAX_DATA_SIZE)
					state = FSM_HASH;

				break;

			case FSM_HASH:
				if(packet.hash() == b)
				{
					state = FSM_END;
					packet.error = PACKET_OK;
					delay(10);
					SoftwareSerial::read();
					SoftwareSerial::read();
					return true;
				}
				else
				{
					packet.error = ERROR_HASH_MISMATCH;
					return false;
				}
				break;

			case FSM_END:
				break;

			default:
				packet.error = ERROR_UNKNOWN;
				state = FSM_END;
				return false;
				break;
			}
		}
	}

#ifdef RS485_DEBUG
	switch (out)
	{
	case ERROR_TIMEOUT:
		Serial.print("ERROR_TIMEOUT: timeout of (ms) ");
		Serial.println(millis() - timestamp);
		break;

	case ERROR_INCOMPLETE_OR_BROKEN:
		Serial.print("ERROR_INCOMPLETE_OR_BROKEN - Last state:");
		Serial.println(last_state);
		break;

	default:
		break;
	}
	Serial.print(F("RS485 received bytes: "));
	for (int i = 0; i < debug_output_count; i++)
	{
		char str[5];
		sprintf(str, "%02X ", debug_output[i]);
		Serial.print(str);
	}

	Serial.println("\n");
#endif

	return false;
}

void RS485Soft::begin(long baudrate)
{
	SoftwareSerial::begin(baudrate);
	pinMode(txControl, OUTPUT);
}

void RS485Soft::txMode()
{
	digitalWrite(txControl, HIGH);
}

void RS485Soft::rxMode()
{
	digitalWrite(txControl, LOW);
}

int RS485Soft::available()
{
	rxMode();
	return SoftwareSerial::available();
}