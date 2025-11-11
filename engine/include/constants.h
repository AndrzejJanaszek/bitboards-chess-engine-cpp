#pragma once

#include <iostream>
#include <cstdint>
#include <vector>

#include "enums.h"

using U64 = uint64_t;

extern const std::string move_type_str[16];

extern int piece_ascii_to_number[128];

extern const char ascii_pieces[];

extern const std::string square_str[];

inline int str_square_to_index(std::string square){
    if(square.length() != 2){
        printf("Error: str_square_to_index(std::string square) square length != 2");
        exit(-1);
    }
    return (square[0] - 'a') + (square[1] - '1') * 8;
}

// todo zmienić na array
// std::vector<std::string> unicode_pieces = {
extern const std::string unicode_pieces[];

extern const char rank_names[];

// masks for checking empty square between king and rooks
constexpr U64 white_queenside_empty_squares_castling_mask = 0xe;
constexpr U64 white_kingside_empty_squares_castling_mask = 0x60;
constexpr U64 black_queenside_empty_squares_castling_mask = 0xe00000000000000;
constexpr U64 black_kingside_empty_squares_castling_mask = 0x6000000000000000;

constexpr U64 A_FILE_MASK = 0x101010101010101ULL;
constexpr U64 B_FILE_MASK = A_FILE_MASK << 1;
constexpr U64 C_FILE_MASK = A_FILE_MASK << 2;
constexpr U64 D_FILE_MASK = A_FILE_MASK << 3;
constexpr U64 E_FILE_MASK = A_FILE_MASK << 4;
constexpr U64 F_FILE_MASK = A_FILE_MASK << 5;
constexpr U64 G_FILE_MASK = A_FILE_MASK << 6;
constexpr U64 H_FILE_MASK = A_FILE_MASK << 7;

constexpr U64 RANK_1_MASK = 0x000000000000ffULL;
constexpr U64 RANK_2_MASK = RANK_1_MASK << (8*1);
constexpr U64 RANK_3_MASK = RANK_1_MASK << (8*2);
constexpr U64 RANK_4_MASK = RANK_1_MASK << (8*3);
constexpr U64 RANK_5_MASK = RANK_1_MASK << (8*4);
constexpr U64 RANK_6_MASK = RANK_1_MASK << (8*5);
constexpr U64 RANK_7_MASK = RANK_1_MASK << (8*6);
constexpr U64 RANK_8_MASK = RANK_1_MASK << (8*7);

constexpr U64 FILE_MASK_ARR[] = {
    A_FILE_MASK,
    B_FILE_MASK,
    C_FILE_MASK,
    D_FILE_MASK,
    E_FILE_MASK,
    F_FILE_MASK,
    G_FILE_MASK,
    H_FILE_MASK
};

constexpr U64 RANK_MASK_ARR[] = {
    RANK_1_MASK,
    RANK_2_MASK,
    RANK_3_MASK,
    RANK_4_MASK,
    RANK_5_MASK,
    RANK_6_MASK,
    RANK_7_MASK,
    RANK_8_MASK
};

constexpr U64 NOT_A_FILE = (~0Ull) ^ A_FILE_MASK;
constexpr U64 NOT_AB_FILE = (~0Ull) ^ (A_FILE_MASK | B_FILE_MASK);
constexpr U64 NOT_H_FILE = (~0Ull) ^ H_FILE_MASK;
constexpr U64 NOT_GH_FILE = (~0Ull) ^ (G_FILE_MASK | H_FILE_MASK);

extern const U64 rook_magic_numbers[64];

extern const U64 bishop_magic_numbers[64];

extern const int bishop_relevant_occupancy_count[64];

extern const int rook_relevant_occupancy_count[64];
