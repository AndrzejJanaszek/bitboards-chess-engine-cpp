#pragma once

#include <cstdint>
#include <vector>

#include "board.h"

class Move{
public:
    unsigned int encoded_value = 0;

    inline int get_from_square(){
        return (encoded_value & 0x3f);
    }

    inline int get_to_square(){
        return ((encoded_value & 0xfc0) >> 6);
    }

    inline int get_piece(){
        return ((encoded_value & 0xf000) >> 12);
    }

    inline int get_move_type(){
        return ((encoded_value & 0xf0000) >> 16);
    }

    void encode_move(int from_square, int to_square, int piece, MoveType move_type);

    void print();
};

bool is_square_attacked_by(int square, int side, GameState &game_state);
// debug / testing
U64 get_attacked_squares(int side, GameState &game_state);

std::vector<Move> generate_moves(GameState &game_state);