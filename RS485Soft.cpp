#include "RS485Soft.h"

/*
 *
 */
RS485Soft::RS485Soft(SoftwareSerial &softSerial, uint8_t rxPin, uint8_t txPin, uint8_t txControl)
{
	rs485 = &softSerial;
	this->rxPin = rxPin;
	this->txPin = txPin;
	this->txControl = txControl;
	size = 0;
}

RS485Soft::~RS485Soft()
{
	rs485->stopListening();
	rs485->end();
}

// Send null-terminated string  NULL terminator is NOT INCLUDED in the packet !!!!  (!)
void RS485Soft::sendChunk(const char *data)
{
	txMode();
	// Header
	write(ASCII_NUL);
	write(ASCII_SOH);
	write(ASCII_STX);

	for (uint8_t i = 0; i < RS485_SOFT_BUFFER_SIZE && (data[i] != (char)0); i++)
		write((uint8_t)data[i]); // body

	// footer
	write(ASCII_ETX);
	write(ASCII_ETB); // 23 = 16 + 4 + 2 + 1 = 0001 0111 = 1 7
}

// Send n bytes of data
void RS485Soft::sendChunk(uint8_t *data, uint8_t n)
{
	txMode();
	// Header
	write(ASCII_NUL);
	write(ASCII_SOH);
	write(ASCII_STX);
	for (uint8_t i = 0; i < n && i < RS485_SOFT_BUFFER_SIZE; i++)
		write(data[i]); // body
	// footer
	write(ASCII_ETX);
	write(ASCII_ETB);
}

void RS485Soft::_timeStamp()
{
	timestamp = millis();
}

/* Watch for timeout
 * @retval 0 Not timed out
 * @retval 1 Time Out!
 */
uint8_t RS485Soft::_timedOut()
{
	return (millis() - timestamp) > timeout;
}



/* Read chuck of data
 * @retval 0 = NEW_PACKET
 * @retval 1 = NULL_PACKET
 * @retval 2 = ERROR_TIMEOUT
 * @retval 3 = ERROR_EXCEEDED_NULL_COUNT
 * @retval 4 = ERROR_INCOMPLETE_OR_BROKEN
 * @retval 5 = ERROR_OVERFLOW
 * @retval 6 = ERROR_EMPTY
 * @retval 7 = ERROR_UNKNOWN
 */
read_code_t RS485Soft::readChunk()
{
	rxMode();
	read_fsm_state_t state = FSM_WAIT_H0;
	read_code_t out = ERROR_UNKNOWN;
	size = 0;

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

	_timeStamp();

	while (state != FSM_END)
	{
		uint16_t b = ASCII_EMPTY_VALUE; // this value will be ignored

		if (rs485->available())
		{
			b = read(); // read one byte at a time

			#ifdef RS485_DEBUG
			debug_output[debug_output_count++] = (uint8_t)b;
			#endif
		}

		if (_timedOut())
		{
			out = ERROR_TIMEOUT;
			state = FSM_END;
		}

		switch (state) // FINITE STATE MACHINE
		{
		case FSM_WAIT_H0: 
			if (b == ASCII_SOH) // First byte header
			{
				state = FSM_WAIT_H1; // OK, next state
				_timeStamp();
			}
			if (b == ASCII_NUL) // Ignore first NULL bytes
			{
				out = NULL_PACKET;
				state = FSM_END;
			}
			break;

		case FSM_WAIT_H1:
			if (b == ASCII_STX) // Second byte header
			{
				state = FSM_PACKET; // OK, next state
				_timeStamp();
			}
			else if (b != ASCII_EMPTY_VALUE)
			{
				state = FSM_END;
				out = ERROR_INCOMPLETE_OR_BROKEN;
			}
			break;

		case FSM_PACKET:
			switch (b)
			{
			case ASCII_EMPTY_VALUE:
			break;
			case ASCII_ETX:
			{
				if (size == 0)
				{
					out = ERROR_EMPTY;
					state = FSM_END;
				}
				else
				{
					_timeStamp();
					state = FSM_WAIT_F1;
				}
			}
			break;
			case ASCII_ETB:
			{
				state = FSM_END;
				out = ERROR_INCOMPLETE_OR_BROKEN;
			}
			break;
			default:
				if (size < RS485_SOFT_BUFFER_SIZE)
				{
					_timeStamp();
					buffer[size] = b;
					size++;
				}
				else
				{
					state = FSM_END;
					out = ERROR_OVERFLOW;
				}
				break;
			}
			break;

		case FSM_WAIT_F1:
			if (b == ASCII_ETB)
			{
				state = FSM_END; // OK
				out = NEW_PACKET;	 // OK Packet Complete!
				// flush
				delay(10);
				read();
				read();
			}
			else if (b != ASCII_EMPTY_VALUE)
			{
				state = FSM_END;
				out = ERROR_INCOMPLETE_OR_BROKEN;
			}
			break;

		case FSM_END:
			break;

		default:
			out = ERROR_UNKNOWN;
			state = FSM_END;
			break;
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

	errorCode = out;
	return out;
}

/* 
 * Returns error codes
 * @retval 0  = No errors (OK)
 * @retval 2 = ERROR_TIMEOUT
 * @retval 3 = ERROR_EXCEEDED_NULL_COUNT
 * @retval 4 = ERROR_INCOMPLETE_OR_BROKEN
 * @retval 5 = ERROR_OVERFLOW
 * @retval 6 = ERROR_EMPTY
 * @retval 7 = ERROR_UNKNOWN
 */
uint8_t RS485Soft::error()
{
	switch (errorCode)
	{
	case NULL_PACKET:
	case NEW_PACKET:
		errorCode = 0; // no errors
		break;
	default:
		break;
	}
	return errorCode;
}

void RS485Soft::printChunk()
{
	for (int i = 0; i < size && i < RS485_SOFT_BUFFER_SIZE; i++)
		Serial.print((char)buffer[i]);
	Serial.println();
}

void RS485Soft::clearChunk()
{
	size = 0; // we dont need to erase the buffer
}

uint8_t RS485Soft::chunkSize()
{
	return size;
}

uint8_t RS485Soft::chunkData(uint8_t i)
{
	if (i < size && i < RS485_SOFT_BUFFER_SIZE)
		return buffer[i];
	return 0;
}

void RS485Soft::begin(long baudrate)
{
	begin(baudrate, RS485_DEFAULT_TIMEOUT);
}

void RS485Soft::begin(long baudrate, unsigned long timeout)
{
	rs485->begin(baudrate);
	this->timeout = timeout;
	pinMode(txControl, OUTPUT);
}

void RS485Soft::write(uint8_t b)
{
	rs485->write(b);
}

uint8_t RS485Soft::read()
{
	return (uint8_t)rs485->read();
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
	return rs485->available();
}