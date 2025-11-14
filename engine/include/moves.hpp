#pragma once

#include <cstdint>
#include <vector>

#include "board.hpp"

class Move{
public:
    unsigned int encoded_value = 0;

    inline int get_from_square() const{
        return (encoded_value & 0x3f);
    }

    inline int get_to_square() const{
        return ((encoded_value & 0xfc0) >> 6);
    }

    inline int get_piece() const{
        return ((encoded_value & 0xf000) >> 12);
    }

    inline int get_move_type() const{
        return ((encoded_value & 0xf0000) >> 16);
    }

    void encode_move(int from_square, int to_square, int piece, MoveType move_type);

    void print() const;
};

bool is_square_attacked_by(int square, int side, Board &game_state);
// debug / testing
U64 get_attacked_squares(int side, Board &game_state);

std::vector<Move> generate_moves(Board &game_state);

// makes move on given board
// returns 1 if legal; 0 if not legal
void make_move(Move move, Board &board);

std::vector<Move> generate_legal_moves(Board &game_state);

// bool isKingUnderAttack(Board &board);
bool isKingUnderAttack(Board &board, bool other_side = false);

bool isCheckMate(Board &board);