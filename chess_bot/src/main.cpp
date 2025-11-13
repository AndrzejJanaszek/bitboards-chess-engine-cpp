#include <iostream>

#include "pieces_weights.hpp"

#include <board.hpp>
#include <attacks.hpp>
#include <utility.hpp>
#include <constants.hpp>

//todo zmienić nazwy plików .hpp, dodać namespace i ogarnać sprawę includeów

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

int main(int argc, char const *argv[])
{
    std::cout << "Dziala\n";

    Board board;
    init_all_lookup_tables(board);

    board.load_fen("r1bqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1");
    printf("fen 1: %d\n", eval(board));

    // board.load_fen("4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1");
    // printf("fen 2: %d\n", eval(board));
   
    // board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RN1QKBNR w KQkq - 0 1");
    // printf("fen 3: %d\n", eval(board));
    
    // board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1");
    // printf("fen 4: %d\n", eval(board));

    // board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1BNR w HAkq - 0 1");
    // printf("fen 5: %d\n", eval(board));

    std::cin.get();

    return 0;
}
