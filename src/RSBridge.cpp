#include "RSBridge.h"

RSBridge::RSBridge(uint8_t rxPin, uint8_t txPin, uint8_t txControl) 
{
    rs485 = new RS485Soft(rxPin, txPin, txControl); // I dont want to use inheritance here
}

RSBridge::~RSBridge()
{
    delete rs485;
}


void RSBridge::begin()
{
    rs485->begin(RS485_DEFAULT_BAUDRATE);
}