#pragma once

#include <iostream>
#include <cstdint>


#include "board.hpp"

using U64 = uint64_t;

extern U64 pawn_lookup_attacks[2][64];

extern U64 knight_lookup_attacks[64];
extern U64 king_lookup_attacks[64];

extern U64 rook_lookup_attacks[64][4096];
extern U64 bishop_lookup_attacks[64][512];

U64 calculate_bishop_attacks(int square, Board &game_state);

U64 calculate_rook_attacks(int square, Board &game_state);

U64 rook_relevant_occupancy(int square);

U64 bishop_relevant_occupancy(int square);

U64 pawn_attacks(int square, int color);

U64 king_attacks(int square);

U64 knight_attacks(int square);

U64 rook_attacks(int square, Board &game_state);

U64 bishop_attacks(int square, Board &game_state);

U64 queen_attacks(int square, Board &game_state);

void print_relevant_occupancy_count_tables();

void init_rook_bishop_lookup_tables(bool rook, Board &game_state);

void init_pawn_lookup_table();

void init_king_lookup_table();

void init_knight_lookup_table();

void init_all_lookup_tables(Board &game_state);