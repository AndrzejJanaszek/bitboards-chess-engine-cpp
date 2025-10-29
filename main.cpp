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

void make_move(Move move, GameState &game_state){
    // GAME STATE UPDATE AT THE END OF FUNCTION DEFINITION
    // color_to_move & color_occupancies & full- half move count
    //
    // enemy color occupancy update on CAPTURE MOVE block

    // reset en passant square
    // if double push flag will be set
    game_state.en_passant_square = -1;

    //* UPDATE CASTLE RIGHTS
    // if rook ON A or H rank update castle rights
    if(move.get_piece() == static_cast<int>(PIECE::R)){
        if( A_FILE_MASK & (1ULL << move.get_from_square()) ){
            // white queen
            game_state.castles &= 0b0111;
        }
        else if( H_FILE_MASK & (1ULL << move.get_from_square()) ){
            // white king
            game_state.castles &= 0b1011;
        }
    }
    else if(move.get_piece() == static_cast<int>(PIECE::r)){
        if( A_FILE_MASK & (1ULL << move.get_from_square()) ){
            // black queen
            game_state.castles &= 0b1101;
        }
        else if( H_FILE_MASK & (1ULL << move.get_from_square()) ){
            // black king
            game_state.castles &= 0b1110;
        }
    }

    // if king remove castle right for that color
    else if(move.get_piece() == static_cast<int>(PIECE::K)){
        // white
        game_state.castles &= 0b0011;
    }
    else if(move.get_piece() == static_cast<int>(PIECE::K)){
        // black
        game_state.castles &= 0b1100;
    }


    //* QUIET MOVE
    if(move.get_move_type() == static_cast<int>(MoveType::quiet_move)){
        // UPDATE BITBOARDS [PICES]
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        game_state.bitboards[move.get_piece()] |= 1ULL << move.get_to_square();
    }


    //* CAPTURE
    if(move.get_move_type() == static_cast<int>(MoveType::capture)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        
        // remove target square
        for(int i = static_cast<int>(PIECE::P); i < static_cast<int>(PIECE::p); i++){
            game_state.bitboards[i+((!game_state.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        game_state.color_occupancy_bitboards[!game_state.color_to_move] &= ~( 1ULL << move.get_to_square() );
        
        // set target square
        game_state.bitboards[move.get_piece()] |= 1ULL << move.get_to_square();

    }

    
    //* MoveType::double_pawn_push
    // double push
    // sett en passant flag
    if(move.get_move_type() == static_cast<int>(MoveType::double_pawn_push)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        game_state.bitboards[move.get_piece()] |= 1ULL << move.get_to_square();

        // set en passant square
        game_state.en_passant_square = (move.get_from_square() + move.get_to_square()) / 2;
    }


    //* MoveType::en_passant_capture
    if(move.get_move_type() == static_cast<int>(MoveType::en_passant_capture)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        game_state.bitboards[move.get_piece()] |= 1ULL << move.get_to_square();

        // remove enemy pawn
        // to target square add offset (for white -8 for black +8)
        const int enemy_pawn_square = move.get_to_square() + (game_state.color_to_move*2 - 1)*8;
        game_state.bitboards[static_cast<int>(PIECE::P) + ((!game_state.color_to_move)*6)] &= ~(1ULL << enemy_pawn_square);
        // OCCUPANCIES
        // remove target square
        game_state.color_occupancy_bitboards[!game_state.color_to_move] &= ~(1ULL << enemy_pawn_square);
    }

    //* PAWN PROMOTION
    // rook
    if(move.get_move_type() == static_cast<int>(MoveType::rook_promotion)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        game_state.bitboards[static_cast<int>(PIECE::R) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // bishop
    else if(move.get_move_type() == static_cast<int>(MoveType::bishop_promotion)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        game_state.bitboards[static_cast<int>(PIECE::B) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // knight
    else if(move.get_move_type() == static_cast<int>(MoveType::knight_promotion)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        game_state.bitboards[static_cast<int>(PIECE::N) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // queen
    else if(move.get_move_type() == static_cast<int>(MoveType::queen_promotion)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        game_state.bitboards[static_cast<int>(PIECE::Q) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();
    }

    //* MoveType::rook_promo_capture
    // rook
    if(move.get_move_type() == static_cast<int>(MoveType::rook_promo_capture)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        
        // remove target square
        for(int i = 0; i < 6; i++){
            game_state.bitboards[i+((!game_state.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        game_state.color_occupancy_bitboards[!game_state.color_to_move] &= ~( 1ULL << move.get_to_square() );

        // set target square
        game_state.bitboards[static_cast<int>(PIECE::R) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // bishop
    else if(move.get_move_type() == static_cast<int>(MoveType::bishop_promo_capture)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );

        // remove target square
        for(int i = 0; i < 6; i++){
            game_state.bitboards[i+((!game_state.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        game_state.color_occupancy_bitboards[!game_state.color_to_move] &= ~( 1ULL << move.get_to_square() );


        // set target square
        game_state.bitboards[static_cast<int>(PIECE::B) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // knight
    else if(move.get_move_type() == static_cast<int>(MoveType::knight_promo_capture)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );

        // remove target square
        for(int i = 0; i < 6; i++){
            game_state.bitboards[i+((!game_state.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        game_state.color_occupancy_bitboards[!game_state.color_to_move] &= ~( 1ULL << move.get_to_square() );

        // set target square
        game_state.bitboards[static_cast<int>(PIECE::N) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // queen
    else if(move.get_move_type() == static_cast<int>(MoveType::queen_promo_capture)){
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );

        // remove target square
        for(int i = 0; i < 6; i++){
            game_state.bitboards[i+((!game_state.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        game_state.color_occupancy_bitboards[!game_state.color_to_move] &= ~( 1ULL << move.get_to_square() );

        // set target square
        game_state.bitboards[static_cast<int>(PIECE::Q) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();
    }

    //* MoveType::king_castle
    if(move.get_move_type() == static_cast<int>(MoveType::king_castle)){
        // king
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        game_state.bitboards[static_cast<int>(PIECE::K) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();

        // rook
        const int rook_target_square = (move.get_from_square() + move.get_to_square())/2;
        const int rook_from_square = game_state.color_to_move ? static_cast<int>(SQUARE::h8) : static_cast<int>(SQUARE::h1);
        // remove rook
        game_state.bitboards[rook_from_square] &= ~(1ULL << move.get_from_square());
        //set rook
        game_state.bitboards[static_cast<int>(PIECE::R) + (game_state.color_to_move*6)] |= 1ULL << rook_target_square;

        // castle rights updated at the start of function
    }

    //* MoveType::queen_castle
    if(move.get_move_type() == static_cast<int>(MoveType::queen_castle)){
        // king
        // remove from square
        game_state.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        game_state.bitboards[static_cast<int>(PIECE::K) + (game_state.color_to_move*6)] |= 1ULL << move.get_to_square();

        // rook
        const int rook_target_square = (move.get_from_square() + move.get_to_square())/2;
        const int rook_from_square = game_state.color_to_move ? static_cast<int>(SQUARE::a8) : static_cast<int>(SQUARE::a1);
        // remove rook
        game_state.bitboards[rook_from_square] &= ~(1ULL << move.get_from_square());
        //set rook
        game_state.bitboards[static_cast<int>(PIECE::R) + (game_state.color_to_move*6)] |= 1ULL << rook_target_square;

        // castle rights updated at the start of function
    }


    // update game state
    game_state.halfmove_counter += 1;
    game_state.fullmove_number = game_state.halfmove_counter/2;

    // UPDATE OCCUPANCIES [COLOR]
    // remove from square
    game_state.color_occupancy_bitboards[game_state.color_to_move] &= ~( 1ULL << move.get_from_square() );
    // add target square
    game_state.color_occupancy_bitboards[game_state.color_to_move] |= 1ULL << move.get_to_square();
    game_state.both_occupancy_bitboard = game_state.color_occupancy_bitboards[0] | game_state.color_occupancy_bitboards[1];

    // update color to move game state
    game_state.color_to_move = !game_state.color_to_move;
}

int main(int argc, char const *argv[])
{
    GameState game_state;
    init_all_lookup_tables(game_state);
    // U64 board = 0ULL;
    // both_occupancy_bitboard = 0ULL;

    // load fen
    game_state.load_fen("8/8/8/8/8/3p4/4P3/8 w - - 0 1"); // white king e4

    // for each move
    char a;
    for(Move m : generate_moves(game_state)){
        m.print();
        game_state.print_board_unicode();
        std::cin >> a;

        GameState game_state_copy = game_state;
        
        make_move(m, game_state_copy);
        game_state_copy.print_board_unicode();
        std::cin >> a;
    }



    // load_fen("K7/8/8/8/8/8/8/8 w - - 0 1"); // white king e4
    // print_bitboard_bits(get_attacked_squares( (int)COLOR::white ));


    // for(Move m : generate_moves()){
    //     m.print();
    // }

    return 0;
}

// todo
// - refractor
// - - game class or game state
// - - separate into files
// - - rewirte copy and load (milion arguments => object reference)
// - test generate moves (perf)
// - test make move & take back move (copy and save game state)
// - GUI