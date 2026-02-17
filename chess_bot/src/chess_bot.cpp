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

int minmax(Board& board, int depth){
    if(depth == 0){
        // board.print_board_ascii(board);
        if(isCheckMate(board)){
            return board.color_to_move == static_cast<int>(COLOR::white) ? INT_MIN : INT_MAX;
        }

        return eval(board);
    }
    
    int best_eval = board.color_to_move == static_cast<int>(COLOR::white) ? INT_MIN : INT_MAX;
    bool success = false;

    auto moves = generate_moves(board);

    for(Move& move : moves){
        Board copy_board = board;
        make_move(move, copy_board);

        // isLegal
        if(!isKingUnderAttack(copy_board, true)){
            int e = minmax(copy_board, depth-1);

            // check for better evaluation (better than best_eval)
            if ((board.color_to_move == static_cast<int>(COLOR::white) && e > best_eval) ||
                (board.color_to_move == static_cast<int>(COLOR::black) && e < best_eval)){
                best_eval = e;
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
            return 0;
        }

        // white or black win (max value for white, min value for black)
        // return board.color_to_move == static_cast<int>(COLOR::white) ? INT_MAX : INT_MIN;
        return best_eval;
    }

    return best_eval;
}

// alpha -> maxi
// beta -> mini
int minmax_alpha_beta(Board& board, int depth, int alpha, int beta){
    if(depth == 0){
        if(isCheckMate(board)){
            return board.color_to_move == static_cast<int>(COLOR::white) ? INT_MIN : INT_MAX;
        }

        return eval(board);
    }
    
    int best_eval = board.color_to_move == static_cast<int>(COLOR::white) ? INT_MIN : INT_MAX;
    bool success = false;

    auto moves = generate_moves(board);

    for(Move& move : moves){
        Board copy_board = board;
        make_move(move, copy_board);

        // isLegal
        if(!isKingUnderAttack(copy_board, true)){
            int e = minmax_alpha_beta(copy_board, depth-1, alpha, beta);

            // check for better evaluation (better than best_eval)
            if(board.color_to_move == static_cast<int>(COLOR::white)){
                best_eval = std::max(best_eval, e);
                alpha = std::max(alpha, e);
                success = true;
            }
            else{
                
                best_eval = std::min(best_eval, e);
                beta = std::min(beta, e);
                success = true;
            }

            if(alpha >= beta)
                break;
        }
    }

    // if no legal moves
    if(success == false){
        //checkmate
        if(!isCheckMate(board)){
            // stale mate
            // board.print_board_ascii(board);
            // throw std::runtime_error("No legal moves!");
            return 0;
        }

        // white or black win (max value for white, min value for black)
        // return board.color_to_move == static_cast<int>(COLOR::white) ? INT_MAX : INT_MIN;
        return best_eval;
    }

    return best_eval;
}


Move get_best_move(Board& board, int depth){
    auto moves = generate_moves(board);
    int best_eval = board.color_to_move == static_cast<int>(COLOR::white) ? INT_MIN : INT_MAX;
    Move best_move;

    int alpha = INT_MIN;
    int beta = INT_MAX;

    // find best move
    for(Move& move : moves){
        Board copy_board = board;
        make_move(move, copy_board);

        int e = minmax_alpha_beta(copy_board, depth, alpha, beta);
        // int e = minmax(copy_board, depth);
        
        if(board.color_to_move == static_cast<int>(COLOR::white) && e > alpha){
            // if( e > alpha)
            alpha = e;

            // best_eval = e;
            best_move = move;
        }
        else if(board.color_to_move == static_cast<int>(COLOR::black) && e < beta){
            // if( e < beta)
            beta = e;

            // best_eval = e;
            best_move = move;
        }
    }

    return best_move;
}