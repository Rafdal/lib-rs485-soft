#include "RSPacket.h"

// Used for the hash function
const uint8_t shuffledTable[] PROGMEM = {
    156, 143, 61, 112, 18, 209, 104, 151, 68, 213, 199, 194, 52, 175, 41, 93,
    2, 67, 64, 72, 90, 205, 129, 164, 227, 69, 159, 240, 248, 44, 60, 135,
    84, 217, 114, 200, 49, 198, 86, 97, 254, 160, 122, 110, 145, 98, 65, 33,
    218, 111, 100, 123, 102, 223, 26, 3, 203, 132, 226, 95, 215, 185, 73, 207,
    79, 235, 195, 150, 233, 216, 10, 125, 87, 238, 206, 191, 51, 107, 7, 230,
    196, 172, 126, 221, 224, 162, 255, 173, 239, 222, 96, 25, 204, 131, 174, 193,
    243, 32, 12, 80, 252, 147, 148, 144, 210, 140, 155, 43, 137, 50, 94, 29,
    56, 167, 141, 228, 242, 231, 116, 39, 184, 54, 250, 197, 1, 74, 142, 48,
    180, 247, 214, 118, 237, 21, 17, 241, 89, 31, 75, 78, 63, 42, 47, 15,
    182, 225, 211, 99, 103, 251, 11, 189, 81, 136, 55, 169, 246, 220, 139, 161,
    188, 37, 121, 62, 153, 38, 115, 88, 57, 165, 170, 134, 178, 113, 176, 45,
    168, 22, 5, 201, 27, 117, 19, 82, 186, 181, 70, 249, 146, 158, 163, 187,
    13, 234, 14, 40, 108, 53, 202, 127, 212, 219, 157, 232, 245, 8, 133, 179,
    0, 59, 154, 120, 190, 6, 24, 253, 85, 9, 149, 192, 236, 28, 166, 128,
    83, 36, 208, 119, 130, 66, 152, 124, 183, 91, 16, 20, 244, 77, 105, 35,
    23, 92, 76, 109, 34, 46, 229, 177, 138, 171, 71, 58, 106, 101, 4, 30,
};


RSPacket::RSPacket()
{
    clear(); // fresh start
}


void RSPacket::clear()
{
    size = 0;
	id = 0;
    error = PACKET_EMPTY;
    memset(data, 0, RS485_MAX_DATA_SIZE);
}

uint8_t RSPacket::pop_back()
{
	if(size > 0)
		return data[--size];
	return 0;
}

void RSPacket::push_back(uint8_t byte)
{
	if(size < RS485_MAX_DATA_SIZE)
		data[size++] = byte;
}

void RSPacket::push_back(uint8_t n, uint8_t array[])
{
	if(size + n <= RS485_MAX_DATA_SIZE)
	{
		for(int i=0; i<n ; i++)
			data[size++] = array[i];
	}
}

void RSPacket::copyBytes(uint8_t n, uint8_t* buffer, uint8_t pos)
{
	if(pos + n <= size)
	{
		for(int i=0; i<n; i++)
			buffer[i] = data[pos + i];
	}
}


uint8_t RSPacket::hash()
{
    // see https://en.wikipedia.org/wiki/Pearson_hashing
    uint8_t h = 0;
    for(int i=0; i < size && i < RS485_MAX_DATA_SIZE; i++ )
        h = pgm_read_byte_near(shuffledTable + (h ^ data[i]));

    return h;
}

void RSPacket::load(const char* str)
{
    if( strlen(str) <= RS485_MAX_DATA_SIZE)
    {
		size = strlen(str);
        for(int i=0; i < size; i++)
            data[i] = (uint8_t) str[i];   // dont copy the NULL terminator
    }
}

char* RSPacket::search(const char* str)
{
    return strstr((char*)data, str);
}

void RSPacket::print()
{
	Serial.print('"');
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
	Serial.print(F("\"\nbytes["));
	Serial.print(size);
	Serial.print(F("] = {"));
	for(int i=0; i < size && i < RS485_MAX_DATA_SIZE; i++)
	{
		Serial.print((int)(data[i]));
		Serial.print(",");
	}
	Serial.println("}");
}
