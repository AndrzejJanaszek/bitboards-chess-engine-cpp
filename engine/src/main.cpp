#include <iostream>
#include <cstdint>
#include <string>
#include <bit>
#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>
#include <bitset>

#include "attacks.hpp"
#include "board.hpp"
#include "constants.hpp"
#include "enums.hpp"
#include "moves.hpp"
#include "utility.hpp"
#include "visualisation.hpp"
#include "perft.hpp"


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

int main(int argc, char const *argv[])
{
    // ------------------------------------------------------
    // INIT
    Board board;
    init_all_lookup_tables(board);
    
    // load fen
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // starting
    // board.load_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"); // starting
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