// NOTE: parity32, hamming_encode, hamming_decode, print_bin taken from:
// https://gist.github.com/qsxcv/b2f9976763d52bf1e7fc255f52f05f5b

#include "hamming.h"

// https://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
static inline bool parity32(uint32_t v)
{
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
}

// Hamming(31, 26) plus total parity at bit 0 for double error detection
// https://en.wikipedia.org/wiki/Hamming_code#General_algorithm
uint32_t hamming_encode(uint32_t d)
{
    // move data bits into position
    uint32_t h =
        (d & 1) << 3 |
        (d & ((1 << 4) - (1 << 1))) << 4 |
        (d & ((1 << 11) - (1 << 4))) << 5 |
        (d & ((1 << 26) - (1 << 11))) << 6;
    // compute parity bits
    h |=
        parity32(h & 0b10101010101010101010101010101010) << 1 |
        parity32(h & 0b11001100110011001100110011001100) << 2 |
        parity32(h & 0b11110000111100001111000011110000) << 4 |
        parity32(h & 0b11111111000000001111111100000000) << 8 |
        parity32(h & 0b11111111111111110000000000000000) << 16;
    // overall parity
    return h | parity32(h);
}

uint32_t hamming_decode(uint32_t h)
{
    // overall parity error
    bool p = parity32(h);
    // error syndrome
    uint32_t i =
        parity32(h & 0b10101010101010101010101010101010) << 0 |
        parity32(h & 0b11001100110011001100110011001100) << 1 |
        parity32(h & 0b11110000111100001111000011110000) << 2 |
        parity32(h & 0b11111111000000001111111100000000) << 3 |
        parity32(h & 0b11111111111111110000000000000000) << 4;
    // correct single error or detect double error
    if (i != 0) {
        if (p == 1) { // single error
            h ^= 1 << i;
        }
        else { // double error
            return ~0;
        }
    }
    // remove parity bits
    return
        ((h >> 3) & 1) |
        ((h >> 4) & ((1 << 4) - (1 << 1))) |
        ((h >> 5) & ((1 << 11) - (1 << 4))) |
        ((h >> 6) & ((1 << 26) - (1 << 11)));
}

void print_bin(uint32_t v)
{
    for (int i = 31; i >= 0; i--)
        printd("%d", (v & (1 << i)) >> i);
}

// takes src char buff of size 26m bytes == 8*26m bits
// outputs dst encoded char buff of size 31m bytes == 8*31m bits
// matrix height = 8, width = 26, depth m
// turns into
// matrix height = 8, width = 31, depth m
// encoding is performed on the width
uint64_t encode_26_block_to_31(char** dst, char* src, uint64_t src_size) {
    uint64_t m = src_size / 26;
    uint64_t dst_size = sizeof(char) * 31 * m;
    *dst = malloc(dst_size);
    if (*dst == NULL) {
        dst_size = 0;
        return;
    }

    for (int i = 0; i < (dst_size); i++) (*dst)[i] = 0;

    uint32_t raw, encoded;
    for (int depth = 0; depth < m; depth++) {
        for (int height = 0; height < 8; height++) {
            int mask_for_height = 1 << height;
            raw = 0;
            for (int buf_shift = 0; buf_shift < 26; buf_shift++) {
                raw |= ((src[(26 * depth) + buf_shift] >> height) & 1) << (26 - 1 - buf_shift);
            }
            encoded = hamming_encode(raw);
            for (int buf_shift = 0; buf_shift < 31; buf_shift++) {
                int this_bit = (encoded >> (31 - 1 - buf_shift)) & 1;
                (*dst)[(31 * depth) + buf_shift] |= this_bit << height;
            }
        }
    }
    return dst_size;
}

void decode_31_block_to_26(char** dst, uint64_t* dst_size, char* src, uint64_t src_size) {
    uint64_t m = src_size / 31;
    *dst_size = sizeof(char) * 26 * m;
    **dst = malloc(*dst_size);
    if (*dst == NULL) {
        *dst_size = 0;
        return;
    }

    for (int i = 0; i < (*dst_size); i++) (*dst)[i] = 0;

    uint32_t raw, encoded;
    for (int depth = 0; depth < m; depth++) {
        for (int height = 0; height < 8; height++) {
            int mask_for_height = 1 << height;
            raw = 0;
            for (int buf_shift = 0; buf_shift < 31; buf_shift++) {
                raw |= ((src[(31 * m) + buf_shift] >> height) & 1) << (31 - 1 - buf_shift);
            }
            encoded = hamming_decode(raw);
            for (int buf_shift = 0; buf_shift < 26; buf_shift++) {
                int this_bit = (encoded >> (26 - 1 - buf_shift)) & 1;
                (*dst)[(26 * m) + buf_shift] |= this_bit << height;
            }
        }
    }
}