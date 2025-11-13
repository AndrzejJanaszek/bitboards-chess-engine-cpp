#include <iostream>

#include "pieces_weights.hpp"

#include "board.hpp"
#include "attacks.hpp"

//todo zmienić nazwy plików .hpp, dodać namespace i ogarnać sprawę includeów

int eval(Board& board){
    // todo napisać funckje evaluacji bazową dla testu
// // iterujemy po wszystkich typach figur (12 bitboardów)
//     for (int bitboard_index = 0; bitboard_index < 12; bitboard_index++)
//     {
//         U64 piece_bitboard = bitboards[bitboard_index];

//         while (piece_bitboard)
//         {
//             int square_number = get_LS1B(piece_bitboard);

//             // przypisujemy odpowiedni znak ASCII do pola
//             arr[square_number] = ascii_pieces[bitboard_index];

//             // usuwamy LS1B
//             piece_bitboard &= (piece_bitboard - 1);
//         }
//     }
}

int main(int argc, char const *argv[])
{
    std::cout << "Dziala\n";

    Board board;
    init_all_lookup_tables(board);

    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/8/8 w HAkq - 0 1");



    return 0;
}
