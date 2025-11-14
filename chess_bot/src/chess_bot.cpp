#include "chess_bot.hpp"


int eval(Board& board){
    // todo napisać funckje evaluacji bazową dla testu
// // iterujemy po wszystkich typach figur (12 bitboardów)

    int evaluation_result = 0;

    int color_factor = 1;
    for (int bitboard_index = 0; bitboard_index < 12; bitboard_index++)
    {
        if (bitboard_index > 5)
            color_factor = -1;

        U64 piece_bitboard = board.bitboards[bitboard_index];

        // sum pieces values
        while (piece_bitboard)
        {
            int square_number = get_LS1B(piece_bitboard);

            // WHITE PAWN
            if(bitboard_index == static_cast<int>(PIECE::P)){
                evaluation_result += color_factor * (PIECE_VALUE[static_cast<int>(PIECE::P)] + PAWN_WHITE_PSQT[square_number]);
            }
            // BLACK PAWN
            else if(bitboard_index == static_cast<int>(PIECE::p)){
                evaluation_result += color_factor * (PIECE_VALUE[static_cast<int>(PIECE::P)] + PAWN_BLACK_PSQT[square_number]);
            }
            // ROOK
            else if((bitboard_index % 6) == static_cast<int>(PIECE::R)){
                evaluation_result += color_factor * (PIECE_VALUE[static_cast<int>(PIECE::R)] + ROOK_PSQT[square_number]);
            }
            // KNIGHT
            else if((bitboard_index % 6) == static_cast<int>(PIECE::N)){
                evaluation_result += color_factor * (PIECE_VALUE[static_cast<int>(PIECE::N)] + KNIGHT_PSQT[square_number]);
            }
            // BISHOP
            else if((bitboard_index % 6) == static_cast<int>(PIECE::B)){
                evaluation_result += color_factor * (PIECE_VALUE[static_cast<int>(PIECE::B)] + BISHOP_PSQT[square_number]);
            }
            // KING
            else if((bitboard_index % 6) == static_cast<int>(PIECE::K)){
                evaluation_result += color_factor * (PIECE_VALUE[static_cast<int>(PIECE::K)]); //+ KING_MIDGAME_PSQT[square_number];
            }
            // QUEEN
            else if((bitboard_index % 6) == static_cast<int>(PIECE::Q)){
                evaluation_result += color_factor * (PIECE_VALUE[static_cast<int>(PIECE::Q)] + QUEEN_PSQT[square_number]);
            }

            // usuwamy LS1B
            pop_bit(piece_bitboard);
        }
    }

    return evaluation_result;
}

 


std::pair<int, Move> get_best_move(Board& board, int depth){
    bool success = false;
    std::pair<int, Move> best_move;
    best_move.first = (board.color_to_move - 0.5)*9999999;

    auto moves = generate_moves(board);

    if(depth == 1){

        for(Move& move : moves){
            Board copy_board = board;

            make_move(move, copy_board);

            // isLegal
            if(!isKingUnderAttack(copy_board, true)){
                int e = eval(copy_board);

                // if white better or black better move
                if ((board.color_to_move == static_cast<int>(COLOR::white) && e > best_move.first) ||
                    (board.color_to_move == static_cast<int>(COLOR::black) && e < best_move.first)){
                    best_move = std::pair<int, Move>(e, move);
                    success = true;
                }
            }
        }

        // if no legal moves
        if(success == false){
            //checkmate
            if(!isCheckMate(board)){
                // stale mate
                // board.print_board_ascii(board);
                // throw std::runtime_error("No legal moves!");
                best_move.first = 0;
            }
        }

        return best_move;
    }
    
    for(Move& move : moves){
        Board copy_board = board;

        make_move(move, copy_board);

        // isLegal
        if(!isKingUnderAttack(copy_board, true)){
            int e = get_best_move(copy_board, depth-1).first;

            // if white better or black better move
            if ((board.color_to_move == static_cast<int>(COLOR::white) && e > best_move.first) ||
                (board.color_to_move == static_cast<int>(COLOR::black) && e < best_move.first)){
                best_move = std::pair<int, Move>(e, move);
                success = true;
            }
        }
    }
    // if no legal moves
        if(success == false){
            //checkmate
            if(!isCheckMate(board)){
                // stale mate
                // board.print_board_ascii(board);
                // throw std::runtime_error("No legal moves!");
                best_move.first = 0;
            }
        }

    return best_move;
}
