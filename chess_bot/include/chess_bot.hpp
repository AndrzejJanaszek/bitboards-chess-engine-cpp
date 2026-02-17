#pragma once

#include <board.hpp>
#include <moves.hpp>
#include <utility.hpp>
#include <pieces_weights.hpp>

int eval(Board& board);
int minmax(Board& board, int depth);
Move get_best_move(Board& board, int depth);