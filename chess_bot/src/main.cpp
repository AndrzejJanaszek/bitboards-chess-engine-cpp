#include <iostream>

#include "pieces_weights.hpp"

#include <board.hpp>
#include <attacks.hpp>
#include <utility.hpp>
#include <constants.hpp>
#include <moves.hpp>

#include "chess_bot.hpp"

//todo zmienić nazwy plików .hpp, dodać namespace i ogarnać sprawę includeów

int main(int argc, char const *argv[])
{
    std::cout << "Dziala\n";

    Board board;
    init_all_lookup_tables(board);

    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // board.load_fen("k7/8/8/8/3r1n2/4P3/8/K7 w - - 0 1");    // bicie wierzy e3xd4
    // board.load_fen("k7/7R/8/8/8/8/6R1/K7 w - - 0 1");    // mat w 1

    // printf("fen 1: %d\n", eval(board));

    for(int i = 0; i < 10; i++){
        printf("%d: ", i);
        get_best_move(board, i).print();
    }
    // get_best_move(board, 2).second.print();


    // auto moves = generate_legal_moves(board);

    // for(Move& m : moves){
    //     m.print();
    // }

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
