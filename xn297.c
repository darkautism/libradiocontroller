#include "XN297.h"
#include <avr/pgmspace.h>

#define GARBAGE_COUNT 5

const uint8_t __attribute__((__progmem__)) xn297_scramble[] = {
    0xE3, 0xB1, 0x4B, 0xEA, 0x85, 0xBC, 0xE5, 0x66,
    0x0D, 0xAE, 0x8C, 0x88, 0x12, 0x69, 0xEE, 0x1F,
    0xC7, 0x62, 0x97, 0xD5, 0x0B, 0x79, 0xCA, 0xCC,
    0x1B, 0x5D, 0x19, 0x10, 0x24, 0xD3, 0xDC, 0x3F,
    0x8E, 0xC5, 0x2F, 0xAA, 0x16, 0xF3, 0x95};

const uint16_t __attribute__((__progmem__)) xn297_crc_xorout_scrambled[] = {
    0x0000, 0x3448, 0x9BA7, 0x8BBB, 0x85E1, 0x3E8C,
    0x451E, 0x18E6, 0x6B24, 0xE7AB, 0x3828, 0x814B,
    0xD461, 0xF494, 0x2503, 0x691D, 0xFE8B, 0x9BA7,
    0x8B17, 0x2920, 0x8B5F, 0x61B1, 0xD391, 0x7401,
    0x2138, 0x129F, 0xB3A0, 0x2988, 0x23CA, 0xC0CB,
    0x0C6C, 0xB329, 0xA0A1, 0x0A16, 0xA9D0 };


static uint8_t bit_reverse(uint8_t b_in)
{
    uint8_t b_out = 0;
    for (uint8_t i = 0; i < 8; ++i)
    {
        b_out = (b_out << 1) | (b_in & 1);
        b_in >>= 1;
    }
    return b_out;
}

static uint16_t crc16_update(uint16_t crc, uint8_t a, uint8_t bits)
{
    crc ^= a << 8;
    while (bits--)
        if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc = crc << 1;
    return crc;
}

static uint16_t calcCRC(uint8_t *msg, uint8_t len)
{
    uint16_t crc = 0xb5d2;
    for (uint8_t i = 0; i < len; ++i)
        crc = crc16_update(crc, msg[i], 8);
    return crc ^ pgm_read_word(&xn297_crc_xorout_scrambled[len - 3]);
}

// Decode XN297 code and check crc
bool XN297Decode(uint8_t *dest, uint8_t *src, uint8_t len)
{
    uint16_t crc = calcCRC(src, len - 2);
    if ((crc >> 8) != src[len - 2] || (crc & 0xff) != src[len - 1])
        return false;

    for (uint8_t i = GARBAGE_COUNT; i < len; i++)
        dest[i - GARBAGE_COUNT] = bit_reverse(src[i] ^ pgm_read_word(&xn297_scramble[i]));
    return true;
}