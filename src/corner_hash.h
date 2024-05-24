#ifndef INVL_CORNER_HASH
#define INVL_CORNER_HASH

#include <cstdint>
#include "ccoord.h"

// perfect hash of raw 64-bit corners -> 65536 buckets
// - only valid for involutions
// - even or odd parity, but not both

uint16_t corner_hash(ccoord cc);

#endif
