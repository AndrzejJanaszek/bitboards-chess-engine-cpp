#pragma once

#include <iostream>
#include <bit>

using U64 = uint64_t;

inline void set_bit(U64 &bitboard, int square){
    bitboard |= (1ULL << square);
}

inline void pop_bit(U64 &bitboard){
    bitboard &= (bitboard-1);
}

// get least significant bit set(1)
// return index of that bit (0x1 => 0, 0x1000 => 3)
// more precise: number of zeros before FS1B
inline int get_LS1B(U64 &bitboard){
    return std::countr_zero(bitboard);
}
