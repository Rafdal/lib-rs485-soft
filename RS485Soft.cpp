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

// Send null-terminated string. NULL terminator is NOT INCLUDED in the packet !!!!  (!)
bool RS485Soft::sendChunk(const char *data)
{
	return sendChunk((uint8_t*)data, strlen(data));
}

// Send bytes of data
bool RS485Soft::sendChunk(uint8_t *data, uint8_t len)
{
	if(len > RS485_SOFT_BUFFER_SIZE)
		return false;

	txMode();
	// Header
	write(ASCII_NUL);
	write(ASCII_SOH);
	write(ASCII_STX);
	write(len);

	// body
	for (uint8_t i = 0; i < len; i++)
		write(data[i]);

	// footer
	write(ASCII_ETX);
	write(ASCII_ETB);
	write( _getShitty8BitCRC(data, len) );
	return true;
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

// This does exactly what you think
uint8_t RS485Soft::_getShitty8BitCRC(uint8_t* data, uint8_t size)
{
	uint8_t shittyCRC = 0;
	for(uint8_t i=0; i<size; i++)
		shittyCRC ^= data[i];
	return shittyCRC;
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
	uint8_t localSize = 0;

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
			if (b == ASCII_NUL) // Ignore NULL bytes
			{
				out = NULL_PACKET;
				state = FSM_END;
			}
			break;

		case FSM_WAIT_H1:
			if (b == ASCII_STX) // Second byte header
			{
				state = FSM_WAIT_LEN; // OK, next state
				_timeStamp();
			}
			else if (b != ASCII_EMPTY_VALUE)
			{
				state = FSM_END;
				out = ERROR_INCOMPLETE_OR_BROKEN;
			}
			break;
		
		case FSM_WAIT_LEN:
			if(b != ASCII_EMPTY_VALUE)
			{
				if(b > RS485_SOFT_BUFFER_SIZE)
				{
					out = ERROR_OVERFLOW;
					state = FSM_END;
				}
				else
				{
					_timeStamp();
					size = b;
					state = FSM_PACKET;
				}
			}
			break;

		case FSM_PACKET:
			switch (b)
			{
			case ASCII_EMPTY_VALUE:
				break;
			case ASCII_ETX:
				{
					if (localSize != size)
					{
						out = ERROR_INCOMPLETE_OR_BROKEN;
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
				if (localSize < size)
				{
					_timeStamp();
					buffer[localSize] = b;
					localSize++;
				}
				else // packet completed
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
				state = FSM_WAIT_CRC; // OK
				_timeStamp();
			}
			else if (b != ASCII_EMPTY_VALUE)
			{
				state = FSM_END;
				out = ERROR_INCOMPLETE_OR_BROKEN;
			}
			break;

		case FSM_WAIT_CRC:
			if(b != ASCII_EMPTY_VALUE)
			{
				if(b != _getShitty8BitCRC(buffer, size))
				{
					out = ERROR_CRC_MISMATCH;
				}
				else
				{
					out = NEW_PACKET;	 // OK Packet Complete!
				}
				state = FSM_END			
				// flush
				delay(10);
				read();
				read();
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
	size = 0; // no need to erase the buffer
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