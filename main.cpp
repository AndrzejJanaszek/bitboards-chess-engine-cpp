#include <iostream>
#include <cstdint>
#include <string>
#include <bit>
#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>
#include <bitset>

#include "attacks.h"
#include "board.h"
#include "constants.h"
#include "enums.h"
#include "moves.h"
#include "utility.h"
#include "visualisation.h"


using U64 = uint64_t;

// ----------------------------------------------------------------------------|
// | MOVE ENCODING                                                             |
// ----------------------------------------------------------------------------|
// | 20 bits                                                                   |
// | 0000 0000 0000 0011 1111 from square === 0x3f                             |
// | 0000 0000 1111 1100 0000 to square   === 0xfc0                            |
// | 0000 1111 0000 0000 0000 piece       === 0xf000                           |
// | 1111 0000 0000 0000 0000 flags       === 0xf0000                          |
// |---------------------------------------------------------------------------|
// |                             FLAG ENCODING                                 |
// |---------------------------------------------------------------------------|
// | code | promotion | capture | special 1 | special 0 |     kind of move     |        
// |---------------------------------------------------------------------------|
// |  0	  |     0     |    0    |     0     |     0     | quiet moves          |
// |  1	  |     0     |    0    |     0     |     1     | double pawn push     |
// |  2	  |     0     |    0    |     1     |     0     | king castle          |
// |  3	  |     0     |    0    |     1     |     1     | queen castle         |
// |  4	  |     0     |    1    |     0     |     0     | captures             |
// |  5	  |     0     |    1    |     0     |     1     | ep-capture           |
// |  8	  |     1     |    0    |     0     |     0     | knight-promotion     |
// |  9	  |     1     |    0    |     0     |     1     | bishop-promotion     |
// |  10  |     1     |    0    |     1     |     0     | rook-promotion       |
// |  11  |     1     |    0    |     1     |     1     | queen-promotion      |
// |  12  |     1     |    1    |     0     |     0     | knight-promo capture |
// |  13  |     1     |    1    |     0     |     1     | bishop-promo capture |
// |  14  |     1     |    1    |     1     |     0     | rook-promo capture   |
// |  15  |     1     |    1    |     1     |     1     | queen-promo capture  |
// -----------------------------------------------------------------------------


void print_bitboard_bits(const U64 &bitboard){
    // little-endian rank-file mapping
    std::vector<std::string> str_board(64, "0");
    
    // tranform U64 to string (first char is a1 last is h8)
    for (int  i = 0; i < 64; i++)
        str_board[i] = ((bitboard & (1ULL << i)) ? "1" : "0");

    print_board_of_strings(str_board);

    std::cout << "bitboard as number: \n";
    std::cout << "hex: 0x" << std::hex << bitboard << "\n";
    std::cout << "dec: " << std::dec << bitboard << "ULL\n";
}


//   a b c d e f g h
// 8 ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜ 
// 7 ♟ ♟ ♟ ♟ ♟ ♟ ♟ ♟ 
// 6 · · · · · · · · 
// 5 · · · · · · · · 
// 4 · · · · · · · · 
// 3 · · · · · · · · 
// 2 ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙ 
// 1 ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖ 
//   a b c d e f g h

// ************************************
// *             MAIN
// ************************************

struct PerftMovesCount
{
    unsigned long long count = 0;
    unsigned long long captures = 0;
    unsigned long long en_pasants = 0;
    unsigned long long checks = 0;
    unsigned long long checkmates = 0;
    unsigned long long castles = 0;
    unsigned long long promotion = 0;
};

void printPerftObject(PerftMovesCount obj){
    printf("count: %llu, captures: %llu, ep: %llu, castles: %llu, promotion: %llu, checks: %llu, checkmates: %llu\n",
            obj.count, obj.captures, obj.en_pasants, obj.castles, obj.promotion, obj.checks, obj.checkmates);
}

bool isKingUnderAttack(Board &board){
    int king_square = get_LS1B(board.bitboards[static_cast<int>(PIECE::K) + (board.color_to_move*6)]);

    if(king_square > 63){
        throw std::runtime_error("Squre > 64 propably no king");
    }

    return is_square_attacked_by(king_square, !board.color_to_move, board);
}

bool isCheckMate(Board &board){
    if(isKingUnderAttack(board) && generate_legal_moves(board).size() == 0){
        return true;
    }

    return false;
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

int main(int argc, char const *argv[])
{
    // ------------------------------------------------------
    // INIT
    Board board;
    init_all_lookup_tables(board);
    
    // load fen
    // board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // starting
    board.load_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"); // starting
    // board.load_fen("k7/8/8/8/8/8/8/K3Q3 w - - 0 1");

    // ------------------------------------------------------

    auto moves = generate_moves(board);
    // auto moves = generate_legal_moves(board);

    char c;
    for(const Move & move : moves){
        move.print();
        // std::cin >> c;
    }

    /* U64 test = 0ULL;

    for(int i = 0; i < 64; i++){
        if(is_square_attacked_by(i, 1, board)){
            test |= (1ULL << i);
        }
    }

    print_bitboard_bits(test); */


    // std::cout << isKingUnderAttack(board);

    for(int i = 1; i < 8; i++){
        printf("%d: ", i);
        PerftMovesCount pmc = perf(i, board);
        printPerftObject(pmc);
    }

    // printf("1: count: %llu, captures: %llu, en_pasants: %llu, checks: %llu, checkmates: %llu  \n", pmc1.count, pmc1.en_pasants, pmc1.captures, pmc1.checks,  pmc1.checkmates);
    // printf("2: count: %llu, captures: %llu, en_pasants: %llu, checks: %llu, checkmates: %llu  \n", pmc2.count, pmc2.en_pasants, pmc2.captures, pmc2.checks,  pmc2.checkmates);
    // printf("3: count: %llu, captures: %llu, en_pasants: %llu, checks: %llu, checkmates: %llu  \n", pmc3.count, pmc3.en_pasants, pmc3.captures, pmc3.checks,  pmc3.checkmates);
    // printf("4: count: %llu, captures: %llu, en_pasants: %llu, checks: %llu, checkmates: %llu  \n", pmc4.count, pmc4.en_pasants, pmc4.captures, pmc4.checks,  pmc4.checkmates);
    // printf("5: count: %llu, captures: %llu, en_pasants: %llu, checks: %llu, checkmates: %llu  \n", pmc5.count, pmc5.en_pasants, pmc5.captures, pmc5.checks,  pmc5.checkmates);
    
    
    return 0;
}

// todo
// - test generate moves (perf)
// - test make move & take back move (copy and save game state)
// - GUI