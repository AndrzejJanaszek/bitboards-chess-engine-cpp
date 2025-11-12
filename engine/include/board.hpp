#pragma once

#include <iostream>
#include <vector>
#include <cstdint>


#include "enums.hpp"


using U64 = uint64_t;

class Board{
public:
    // bitboards; index: PIECE enum
    U64 bitboards[12] = {0ULL};

    // 0 - white; 1 - black
    U64 color_occupancy_bitboards[2] = {0ULL};
    U64 both_occupancy_bitboard = 0ULL;

    // castle system - each bit describes one possibility
    // | white queenside | white kingside | black queenside | black kingside |
    // |      bit 0/1    |     bit 0/1    |     bit 0/1     |    bit 0/1     |
    int castles = 0;

    // from enum COLOR -> white = 0, black = 1
    int color_to_move = static_cast<int>(COLOR::none);

    // from enum 0->63 square | -1 none
    int en_passant_square = static_cast<int>(SQUARE::none);

    // halfmoves since last capture or pawn advance for fifty-move rule
    int halfmove_counter = 0;

    // Fullmove number: The number of the full moves. It starts at 1 and is incremented after Black's move.
    int fullmove_number = 1;

    // przeciążenie operatora przypisania
    Board &operator=(const Board &other);

    void clear_bitboards();

    void load_fen(std::string fen);

    void print_game_state();
    void print_board_unicode();

    void print_board_ascii(Board &game_state);
};
