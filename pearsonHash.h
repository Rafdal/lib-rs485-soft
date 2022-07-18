#ifndef PEARSONHASH_H
#define PEARSONHASH_H

#include <Arduino.h>

/**
 * @brief Get Pearson hash value
 * @param n size of array
 * @param array array of bytes to hash
 * @return uint8_t hash value
 */
uint8_t pearsonHash(uint8_t n, uint8_t array[]);

#endif