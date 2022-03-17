#pragma once
#ifndef H_COMMON_HAMMING
#define H_COMMON_HAMMING

#include "common_includes.h"

// NOTE: parity32, hamming_encode, hamming_decode, print_bin taken from:
// https://gist.github.com/qsxcv/b2f9976763d52bf1e7fc255f52f05f5b

// https://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
static inline bool parity32(uint32_t v);

// Hamming(31, 26) plus total parity at bit 0 for double error detection
// https://en.wikipedia.org/wiki/Hamming_code#General_algorithm
uint32_t hamming_encode(uint32_t d);

uint32_t hamming_decode(uint32_t h);

void print_bin(uint32_t v);
#endif