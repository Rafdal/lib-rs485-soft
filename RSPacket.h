#ifndef RSPacket_H
#define RSPacket_H

#include <Arduino.h>

#define RS485_MAX_DATA_SIZE 32

enum PacketError
{
	PACKET_OK, // No error
	NULL_PACKET, // No packet
	ERROR_TIMEOUT,
	ERROR_HEADER_BROKEN,
    ERROR_HASH_MISMATCH,
	ERROR_SIZE_OVERFLOW,
	PACKET_EMPTY,
	ERROR_UNKNOWN,
};

class RSPacket
{
public:
    RSPacket();
    ~RSPacket(){}

    void print();
    void clear();

    /**
     * @brief Load packet with a C-String
     * 
     * @param str NULL terminated string
     */
    void load(const char* str); 

    /**
     * @brief Search str inside message. This is a wrapper of strstr func
     * 
     * @param str string to search
     * @return char* pointer to the first occurrence of str inside message or NULL if not found
     */
    char* search(const char* str);

    /**
     * @brief Get Pearson hash value
     * @return uint8_t hash value
     */
    uint8_t hash();

    // packet data
	uint8_t size;
	uint8_t data[RS485_MAX_DATA_SIZE]; // data chunk
    uint8_t error;

private:

};

#endif