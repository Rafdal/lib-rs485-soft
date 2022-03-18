#include "RS485Soft.h"

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



// Send NULL terminated string (null is not included in packet)
void RS485Soft::sendChunk(const char *data)
{
    txMode();
	// Header
	write(ASCII_SOH);
	write(ASCII_STX);
	for (uint8_t i = 0; i < RS485_SOFT_BUFFER_SIZE && data[i] != NULL; i++)
		write((uint8_t)data[i]); // body
	// footer
	write(ASCII_ETX);
	write(ASCII_ETB);
}


void RS485Soft::sendChunk(uint8_t *data, uint8_t size)
{
	txMode();
	// Header
	write(ASCII_SOH);
	write(ASCII_STX);
	for (uint8_t i = 0; i < size && i < RS485_SOFT_BUFFER_SIZE; i++)
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
 * @retval 0 = OK
 * @retval 1 = TIMEOUT
 * @retval 2 = PACKET INCOMPLETE OR BROKEN
 * @retval 3 = BUFFER OVERFLOW
 * @retval 4 = EMPTY PACKET
 * @retval 5 = UNKNOWN ERROR
 */
error_code_t RS485Soft::readChunk()
{
    rxMode();
	read_state_t state = FSM_WAIT_H0;
	error_code_t out = ERROR_UNKNOWN;
	size = 0;
	_timeStamp();
	while (state != FSM_END)
	{
		uint8_t b = ASCII_NUL;

		if (rs485->available())
		{
			b = (uint8_t)rs485->read(); // read one byte at a time
		}

		if (_timedOut())
		{
			out = ERROR_TIMEOUT;
			state = FSM_END;
		}

		switch (state) // FINITE STATE MACHINE
		{
		case FSM_WAIT_H0:
			if (b == ASCII_SOH)
			{
				state = FSM_WAIT_H1; // OK, next state
				_timeStamp();
			}
			break;

		case FSM_WAIT_H1:
			if (b == ASCII_STX)
			{
				state = FSM_PACKET; // OK, next state
				_timeStamp();
			}
			else if (b != ASCII_NUL)
			{
				state = FSM_END;
				out = ERROR_INCOMPLETE_OR_BROKEN;
			}
			break;

		case FSM_PACKET:
			switch (b)
			{
			case ASCII_NUL:
				break;
			case ASCII_ETX:
			{
                if(size == 0){
                    out = ERROR_EMPTY;
                    state = FSM_END;
                }
                else{
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
				out = ERROR_OK;		 // OK Packet Complete!
			}
			else if (b != ASCII_NUL)
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
    errorCode = out;
	return out;
}

uint8_t RS485Soft::error()
{
    return errorCode;
}

void RS485Soft::printChunk()
{
	for(int i=0; i<size && i<RS485_SOFT_BUFFER_SIZE; i++)
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