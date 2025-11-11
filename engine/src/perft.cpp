#include "perft.h"

void printPerftObject(PerftMovesCount obj){
    printf("count: %llu, captures: %llu, ep: %llu, castles: %llu, promotion: %llu, checks: %llu, checkmates: %llu\n",
            obj.count, obj.captures, obj.en_pasants, obj.castles, obj.promotion, obj.checks, obj.checkmates);
}

PerftMovesCount perf(int depth, Board &board){
    PerftMovesCount moves_count;

    if(depth == 0){
        return moves_count;
    }

    if(depth == 1){
        auto moves = generate_legal_moves(board);

        for(const Move &move : moves){
            if (move.get_move_type() & static_cast<int>(MoveType::capture)){
                moves_count.captures += 1;
            }
            
            if (move.get_move_type() == static_cast<int>(MoveType::en_passant_capture)){
                moves_count.en_pasants += 1;
            }

            if (move.get_move_type() == static_cast<int>(MoveType::king_castle) || move.get_move_type() == static_cast<int>(MoveType::queen_castle)){
                moves_count.castles += 1;
            }

            if (move.get_move_type() & static_cast<int>(MoveType::knight_promotion)){
                moves_count.promotion += 1;
            }
            
            Board copy = board;
            make_move(move, copy);
            if( isKingUnderAttack(copy) ){
                moves_count.checks += 1;
            }

            if(isCheckMate(copy)){
                moves_count.checkmates += 1;
            }
        }

        moves_count.count = moves.size();

        return moves_count;
    }
    
    auto moves = generate_legal_moves(board);

    for(const Move &move : moves){
        Board copy = board;
        make_move(move, copy);

        PerftMovesCount new_count = perf(depth-1, copy);
        moves_count.count += new_count.count;
        moves_count.captures += new_count.captures;
        moves_count.en_pasants += new_count.en_pasants;
        moves_count.checks += new_count.checks;
        moves_count.checkmates += new_count.checkmates;
        moves_count.castles += new_count.castles;
        moves_count.promotion += new_count.promotion;
    }

    return moves_count;
}
