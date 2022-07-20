#include "RSPacket.h"
#include "RSNetDevice.h"


RSPacket::RSPacket()
{
    clear(); // fresh start
	id = IDNotSet;
}

void RSPacket::addTopic(const char topic[RS485_MAX_TOPIC_SIZE])
{
    if(strlen(topic) + size <= RS485_MAX_TOPIC_SIZE)
    {
        push_front(':');
        push_front((uint8_t)strlen(topic), (uint8_t*)topic);
        push_front('$');
    }
}

uint8_t RSPacket::hashTopic()
{
	char topic[RS485_MAX_TOPIC_SIZE];
	uint8_t h = 0;
	if(getTopic(topic))
	{
		uint8_t tsize = strlen(topic);
		h = pearsonHash(tsize, (uint8_t*)topic);
	}
	return h;
}

void RSPacket::clear()
{
    size = 0;
    error = PACKET_OK;
    memset(data, 0, RS485_MAX_DATA_SIZE);
}

uint8_t RSPacket::pop_back()
{
	if(size > 0)
		return data[--size];

	error = BOUNDING_ERROR;
	return 0;
}

void RSPacket::push_front(uint8_t byte)
{
	if(size < RS485_MAX_DATA_SIZE)
	{
		// copy all
		for (int i = size-1; i >= 0; i--)
			data[i+1] = data[i];
		
		data[0] = byte; // push front
		size++;
		return;
	}
	error = BOUNDING_ERROR;
}

void RSPacket::push_front(uint8_t n, uint8_t array[])
{
	if(size + n <= RS485_MAX_DATA_SIZE)
	{
		// copy all
		for (int i = size-1; i >= 0; i--)
			data[i+n] = data[i];

		// then push front
		for (int i = 0; i < n; i++)
			data[i] = array[i];		
		size = size + n;
		return;
	}
	error = BOUNDING_ERROR;
}

uint8_t RSPacket::pop_front()
{
	if(size > 0)
	{
		size--;
		uint8_t out = data[0];
		for(int i=0; i < size; i++)
			data[i] = data[i + 1];

		return out;
	}
	error = BOUNDING_ERROR;
	return 0;
}

void RSPacket::erase_front(uint8_t n)
{
	if(size >= n)
	{
		size = size - n;
		for(int i=0; i < size; i++)
			data[i] = data[i + n];
		return;
	}
	error = BOUNDING_ERROR;
}

void RSPacket::push_back(uint8_t byte)
{
	if(size < RS485_MAX_DATA_SIZE)
	{
		data[size++] = byte;
		return;
	}
	error = BOUNDING_ERROR;
}

void RSPacket::push_back(uint8_t n, uint8_t array[])
{
	if(size + n <= RS485_MAX_DATA_SIZE)
	{
		for(int i=0; i<n ; i++)
			data[size++] = array[i];
		return;
	}
	error = BOUNDING_ERROR;
}

void RSPacket::copyBytes(uint8_t n, uint8_t* buffer, uint8_t pos)
{
	if(pos + n <= size)
	{
		for(int i=0; i<n; i++)
			buffer[i] = data[pos + i];
		return;
	}
	error = BOUNDING_ERROR;
}

uint8_t RSPacket::hash()
{
	return pearsonHash(size, data);
}

void RSPacket::load(const char* str)
{
	size_t len = strlen(str);
    if( size + len <= RS485_MAX_DATA_SIZE)
    {
        for(size_t i=0; i < len; i++)
            data[size+i] = (uint8_t) str[i];   // dont copy the NULL terminator
		size += len;
		return;
	}
	error = BOUNDING_ERROR;
}

void RSPacket::load(RSPacket& p)
{
	if( size + p.size <= RS485_MAX_DATA_SIZE)
    {
        for(int i=0; i < p.size; i++)
            data[size+i] = p.data[i];   // dont copy the NULL terminator
		size += p.size;
		id = p.id;
		return;
	}
	error = BOUNDING_ERROR;
}


char* RSPacket::search(const char* str)
{
    return strstr((char*)data, str);
}

void RSPacket::print()
{
	Serial.println(F("@ Packet {"));
	Serial.print(F("\tcontent = \""));
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
	Serial.print(F("\"\n\tbytes["));
	Serial.print(size);
	Serial.print(F("] = {"));
	for(int i=0; i < size && i < RS485_MAX_DATA_SIZE; i++)
	{
		Serial.print((int)data[i]);
		Serial.print(',');
	}
	Serial.println(F("}\n}"));
}

bool RSPacket::getTopic(char buffer[RS485_MAX_TOPIC_SIZE])
{
	// -> topic structure looks like:  "$TOPIC:"
    // -> RS485_MAX_TOPIC_SIZE is the len of the topic + the NULL terminator which is not in packet
    if(data[0] == '$')
    {
        bool readingTopic = true;

        int tsize = 0; // topic size
        while(readingTopic)
        {
            //                                  +2 for the '$' and ':' characters
            if((tsize < (RS485_MAX_TOPIC_SIZE - 1)) && (tsize + 2) < size)
            {
                char c = data[tsize + 1]; // +1 for the '$'
                
                if(isalnum(c))
                {
                    buffer[tsize++] = c;
                    if(tsize == (RS485_MAX_TOPIC_SIZE - 1))
                    {
                        if(data[tsize + 1] == ':')   // TODO: TEST THIS CASE
                        {
                            // Ok 1
                            buffer[tsize] = '\0'; // set null terminator
                            readingTopic = false;
                        }
                        else
                        {
                            // error
                            return false;
                        }
                    }
                }
                else if(c == ':')
                {
                    if(tsize > 0) // TODO: TEST THIS CASE
                    {
                        // Ok 2
                        buffer[tsize] = '\0'; // set null terminator
                        readingTopic = false;
                    }
                    else
                    {
                        // error topic empty
                        return false;
                    }
                }
                else
                {
                    // error, not an alphanumeric character
                    return false;
                }
            }
            else
            {
                // size error
                return false;
            }
        }                
        return true;
    }
    return false;
}