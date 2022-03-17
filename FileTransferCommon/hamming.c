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
    return d;
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
    return h >> 1;// | parity32(h);
}

uint32_t hamming_decode(uint32_t h)
{
    return h;
    h = (h<<1) | parity32(h);
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
        h ^= 1 << i;
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

/*uint64_t encode_x_block_to_y(char** dst, char* src, uint64_t src_size, int x, int y, uint32_t(*f_encode)(uint32_t)) {
    uint64_t m = src_size / x;
    uint64_t dst_size = sizeof(char) * y * m;
    *dst = malloc(dst_size);
    if (*dst == NULL) {
        dst_size = 0;
        return;
    }

    for (int i = 0; i < (dst_size); i++) (*dst)[i] = 0;

    uint32_t raw, encoded;
    for (int depth = 0; depth < m; depth++) {
        for (int height = 0; height < 8; height++) {
            raw = 0;
            for (int buf_shift = 0; buf_shift < x; buf_shift++) {
                raw |= ((src[(x * depth) + buf_shift] >> height) & 1) << (x - 1 - buf_shift);
            }
            encoded = (*f_encode)(raw);
            for (int buf_shift = 0; buf_shift < y; buf_shift++) {
                int this_bit = (encoded >> (y - 1 - buf_shift)) & 1;
                (*dst)[(y * depth) + buf_shift] |= this_bit << height;
            }
        }
    }
    return dst_size;
}*/

void encode_x_block_to_y(char dst[], char src[], int x, int y, uint32_t(*f_encode)(uint32_t)) {
    uint64_t dst_size = sizeof(char) * y;

    for (int i = 0; i < (dst_size); i++) dst[i] = 0;

    for (int height = 0; height < sizeof(char); height++) {
        int line = 0, encoded = 0;
        for (int shift = 0; shift < x; shift++) {
            line <<= 1;
            line |= (src[shift] >> height) & 1;
        }
        encoded = (*f_encode)(line);
        for (int shift = 0; shift < y; shift++) {
            int current_bit = (encoded >> shift) & 1;
            dst[y - 1 - shift] |= current_bit << height;
        }
    }
}

void encode_x_block_to_y_offset(char dst[], char src[], int x, int y, uint32_t(*f_encode)(uint32_t), int src_offset) {
    uint64_t dst_size = sizeof(char) * y;

    for (int i = 0; i < (dst_size); i++) dst[i] = 0;

    for (int height = 0; height < sizeof(char); height++) {
        int line = 0, encoded = 0;
        for (int shift = 0; shift < x; shift++) {
            line |= ((src[shift+src_offset] >> height) & 1) << (x-1-shift);
        }
        encoded = (*f_encode)(line);
        for (int shift = 0; shift < y; shift++) {
            int current_bit = (encoded >> shift) & 1;
            dst[y - 1 - shift] |= current_bit << height;
        }
    }
}

/*void encode_26_block_to_31(char dst[], char src[]) {
    uint64_t dst_size = sizeof(char) * 31;

    for (int i = 0; i < (dst_size); i++) dst[i] = 0;

    for (int height = 0; height < sizeof(char); height++) {
        int line = 0, encoded = 0;
        for (int shift = 0; shift < 26; shift++) {
            line <<= 1;
            line |= (src[shift] >> height) & 1;
        }
        encoded = hamming_encode(line);
        for (int shift = 0; shift < 31; shift++) {
            int current_bit = (encoded >> shift) & 1;
            dst[31 - 1 - shift] |= current_bit << height;
        }
    }
}*/

void encode_26_block_to_31(char dst[], char src[]) {
    return encode_x_block_to_y(dst, src, 26, 31, &hamming_encode);
}

void encode_26_block_to_31_offset(char dst[], char src[], int src_offset) {
    for (int i = 0; i < 31; i++) dst[i] = 0;

    for (int height = 0; height < 8; height++) {
        uint32_t line = 0, encoded = 0;
        for (int shift = 0; shift < 26; shift++) {
            line |= ((src[shift + src_offset] >> height) & 1) << (26 - 1 - shift);
        }
        encoded = hamming_encode(line);
        for (int shift = 0; shift < 31; shift++) {
            int current_bit = (encoded >> shift) & 1;
            if (current_bit != 0) {
                printf("lala %d\n", shift);
            }
            dst[31 - 1 - shift] |= current_bit << height;
        };
    }
}

void decode_31_block_to_26(char dst[], char src[]) {
    return encode_x_block_to_y(dst, src, 31, 26, &hamming_decode);
}

void decode_31_block_to_26_offset(char dst[], char src[], int src_offset) {
    for (int i = 0; i < 26; i++) dst[i] = 0;

    for (int height = 0; height < sizeof(char); height++) {
        int line = 0, encoded = 0;
        for (int shift = 0; shift < 31; shift++) {
            line |= ((src[shift + src_offset] >> height) & 1) << (31 - 1 - shift);
        }
        encoded = hamming_decode(line);
        for (int shift = 0; shift < 26; shift++) {
            int current_bit = (encoded >> shift) & 1;
            dst[26 - 1 - shift] |= current_bit << height;
        }
    }
}