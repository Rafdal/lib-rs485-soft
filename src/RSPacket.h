#ifndef RSPacket_H
#define RSPacket_H

#include <Arduino.h>
#include "pearsonHash.h"

#define RS485_MAX_DATA_SIZE 48

#define RS485_MAX_TOPIC_SIZE 12

enum PacketError
{
	PACKET_OK, // No error
	NULL_PACKET, // No packet
	ERROR_TIMEOUT,
	ERROR_HEADER_BROKEN,
    ERROR_HASH_MISMATCH,
	ERROR_SIZE_OVERFLOW,
	PACKET_EMPTY,
    BOUNDING_ERROR,
	ERROR_UNKNOWN,
};

enum ReservedIDs
{
    IDNotSet = 253,
    PublicID,
    MasterID,
};

class RSPacket
{
public:
    RSPacket();
    ~RSPacket(){}


    /**
     * @brief Print packet content data (but not ID)
     */
    void print();

    /**
     * @brief Clear packet content data (but not ID)
     */
    void clear();

    /**
     * @brief Add a topic to the packet
     * 
     * @param topic char*
     */
    void addTopic(const char topic[RS485_MAX_TOPIC_SIZE]);

    /**
     * @brief Get the topic of the packet if it's in a valid topic format
     * 
     * @param topic external buffer with RS485_MAX_TOPIC_SIZE
     * @retval true = valid topic format
     * @retval false = invalid topic format or not present
     */
    bool getTopic(char buffer[RS485_MAX_TOPIC_SIZE]);

    /**
     * @brief Load packet with a C-String
     * 
     * @param str NULL terminated string
     */
    void load(const char* str); 

    /**
     * @brief Load packet with another packet data
     * 
     * @param p RSPacket
     */
    void load(RSPacket& p); 

    /**
     * @brief Append new byte and increase packet size by 1
     * 
     * @param byte 
     */
    void push_back(uint8_t byte);

    /**
     * @brief Append n bytes and increase packet size by n
     * 
     * @param n size of array
     * @param array array of bytes to append
     */
    void push_back(uint8_t n, uint8_t array[]);

    /**
     * @brief Append new byte and increase packet size by 1
     * 
     * @param byte 
     */
    void push_front(uint8_t byte);

    /**
     * @brief Append n bytes and increase packet size by n
     * 
     * @param n size of array
     * @param array array of bytes to append
     */
    void push_front(uint8_t n, uint8_t array[]);

    /**
     * @brief Get the last byte and decrease size by 1
     * 
     * @return uint8_t last byte
     */
    uint8_t pop_back();

    /**
     * @brief Get the first byte and decrease size by 1
     * 
     * @return uint8_t first byte
     */
    uint8_t pop_front();

    /**
     * @brief Erase n bytes of the front
     * @param n amount of bytes to erase
     */
    void erase_front(uint8_t n);

    /**
     * @brief Copy n bytes from starting pos to external buffer
     * 
     * @param n amount of bytes to copy
     * @param buffer buffer with enough size to store copied bytes
     * @param pos starting pos to copy from (optional)
     */
    void copyBytes(uint8_t n, uint8_t* buffer, uint8_t pos = 0);

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

    /**
     * @brief Get Pearson hash value of the topic
     * @return uint8_t topic hash value
     */
    uint8_t hashTopic();

    // packet basic data
	uint8_t size;
	uint8_t data[RS485_MAX_DATA_SIZE]; // data chunk
    uint8_t error;

    uint8_t id; // to / from

private:

    void sayError();

};


#endif