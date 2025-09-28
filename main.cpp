#include <iostream>
#include <cstdint>
#include <bit>
#include <vector>

// ************************************
//          DEFINE STATEMENTS
// ************************************

#define U64 uint64_t



// ************************************
//               ENUMS
// ************************************

enum class SQUARE {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8
};

enum class PIECE{
    // pawn, rook, knight, bishop, queen, king
    // white pieces
    P, R, N, B, Q, K, 
    // black pieces
    p, r, n, b, q, k
};

int piece_ascii_to_number[128] = {
    piece_ascii_to_number['P'] = static_cast<int>(PIECE::P),
    piece_ascii_to_number['R'] = static_cast<int>(PIECE::R),
    piece_ascii_to_number['N'] = static_cast<int>(PIECE::N),
    piece_ascii_to_number['B'] = static_cast<int>(PIECE::B),
    piece_ascii_to_number['Q'] = static_cast<int>(PIECE::Q),
    piece_ascii_to_number['K'] = static_cast<int>(PIECE::K),

    piece_ascii_to_number['p'] = static_cast<int>(PIECE::p),
    piece_ascii_to_number['r'] = static_cast<int>(PIECE::r),
    piece_ascii_to_number['n'] = static_cast<int>(PIECE::n),
    piece_ascii_to_number['b'] = static_cast<int>(PIECE::b),
    piece_ascii_to_number['q'] = static_cast<int>(PIECE::q),
    piece_ascii_to_number['k'] = static_cast<int>(PIECE::k),
};

char ascii_pieces[] = {
    'P', 'R', 'N', 'B', 'Q', 'K',
    'p', 'r', 'n', 'b', 'q', 'k'
};


// ************************************
//              BITBOARDS
// ************************************

// bitboards; index: PIECE enum
U64 bitboards[12] = {0};

// ************************************
//              FUNCTIONS
// ************************************

char rank_names[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

inline void set_bit(U64 &bitboard, int square){
    bitboard |= (1ULL << square);
}

// ************************************
//            VISUALISATION
// ************************************

void print_board_of_strings(const std::vector<std::string> &board_strings){
    // print bitboard bits with rank and file descriptions
    printf("\n-------------------------\n");
    printf("    a b c d e f g h\n");
    printf("\n");
    for (int  rank = 7; rank >= 0; rank--){
        printf("%d   ", rank+1);
        for (int  file = 0; file < 8; file++)
        {
            std::cout << board_strings[rank*8 + file] << " ";
        }
        printf("  %d", rank+1);
        printf("\n");
    }
    printf("\n");
    printf("    a b c d e f g h\n");
    printf("\n-------------------------\n");
}

void print_bitboard_bits(const U64 &bitboard){
    // little-endian rank-file mapping
    std::vector<std::string> str_board(64, "0");
    
    // tranform U64 to string (first char is a1 last is h8)
    for (int  i = 0; i < 64; i++)
        str_board[i] = ((bitboard & (1ULL << i)) ? "1" : "0");

    print_board_of_strings(str_board);
}

void print_board_ascii(){
    std::vector<std::string> pieces(64, "0");


    // for each bitboard (piece type) set its characters
    for(int bitboard_index = 0; bitboard_index < 12; bitboard_index++){
        U64 piece_bitboard = bitboards[bitboard_index];
        // until 1 bits available in current piece bitboard
        while(piece_bitboard){
            // get LS1B index (count the number of consecutive 0 bits) 
            int square_number = std::countr_zero(piece_bitboard);

            // set ascii character 
            pieces[square_number] = std::string(1, ascii_pieces[bitboard_index]);

            // delete LS1B
            piece_bitboard = piece_bitboard & (piece_bitboard-1);
        }
    }

    print_board_of_strings(pieces);
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

std::vector<std::string> unicode_pieces = {
    // white pieces
    "♙","♖","♘","♗","♕","♔",
    // black pieces
    "♟","♜","♞","♝","♛","♚"
};

void print_board_unicode(){
    std::vector<std::string> pieces(64, ".");

    // for each bitboard (piece type) set its characters
    for(int bitboard_index = 0; bitboard_index < 12; bitboard_index++){
        U64 piece_bitboard = bitboards[bitboard_index];
        // until 1 bits available in current piece bitboard
        while(piece_bitboard){
            // get LS1B index (count the number of consecutive 0 bits) 
            int square_number = std::countr_zero(piece_bitboard);

            // set unicode character 
            pieces[square_number] = unicode_pieces[bitboard_index];

            // delete LS1B
            piece_bitboard = piece_bitboard & (piece_bitboard-1);
        }
    }

    print_board_of_strings(pieces);
}


// ************************************
//               MAIN
// ************************************

int main(int argc, char const *argv[])
{
    U64 board = 0;
    set_bit(bitboards[static_cast<int>(PIECE::P)], static_cast<int>(SQUARE::a2));
    set_bit(bitboards[static_cast<int>(PIECE::P)], static_cast<int>(SQUARE::b2));
    set_bit(bitboards[static_cast<int>(PIECE::P)], static_cast<int>(SQUARE::c2));
    set_bit(bitboards[static_cast<int>(PIECE::P)], static_cast<int>(SQUARE::d2));

    // print_bitboard_bits(bitboards[0]);
    
    // std::cout << std::countr_zero(bitboards[0]);
    // print_board_ascii();
    print_board_ascii();
    print_board_unicode();
    print_bitboard_bits(bitboards[static_cast<int>(PIECE::P)]);



    return 0;
}
// 