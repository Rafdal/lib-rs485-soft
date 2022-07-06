#include "RS485Packet.h"


RS485Packet::RS485Packet()
{
    clear(); // fresh start
}


uint8_t RS485Packet::hash()
{
    // https://en.wikipedia.org/wiki/Pearson_hashing
    uint8_t h = 0;
    for(int i=0; i < size && i < RS485_MAX_DATA_SIZE; i++ )
        h = pgm_read_byte_near(shuffledTable + (h ^ data[i]));

    return h;
}

void RS485Packet::clear()
{
    size = 0;
    error = PACKET_EMPTY;
    memset(data, 0, RS485_MAX_DATA_SIZE);
}

void RS485Packet::load(const char* str)
{
    if( (size = strlen(str)) <= RS485_MAX_DATA_SIZE)
    {
        for(int i=0; i < size; i++)
            data[i] = (uint8_t) str[i];   // dont copy the NULL terminator
    }
}

char* RS485Packet::search(const char* str)
{
    return strstr((char*)data, str);
}

void RS485Packet::print()
{
	for(int i=0; i < size && i < RS485_MAX_DATA_SIZE; i++)
	{
		switch (data[i])
		{
		case '\a':
			Serial.print("\\a");
			break;
		case '\b':
			Serial.print("\\b");
			break;
		case '\e':
			Serial.print("\\e");
			break;
		case '\f':
			Serial.print("\\f");
			break;
		case '\v':
			Serial.print("\\v");
			break;
		case '\0':
			Serial.print("\\0");
			break;
		case '\n':
			Serial.print("\\n");
			break;
		case '\r':
			Serial.print("\\r");
			break;
		case '\t':
			Serial.print("\\t");
			break;		
		default:
			Serial.print((char)data[i]);
			break;
		}
	}
	Serial.print("\"\nbytes: {");
	for(int i=0; i < size && i < RS485_MAX_DATA_SIZE; i++)
	{
		Serial.print((int)(data[i]));
		Serial.print(",");
	}
	Serial.println("}");
}
