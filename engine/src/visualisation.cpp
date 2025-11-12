#include <iostream>
#include <vector>

#include "visualisation.hpp"

void print_board_of_strings(const std::vector<std::string> &board_strings){
    // print bitboard bits with rank and file descriptions
    // printf("-------------------------\n");
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
    // printf("-------------------------\n");
}

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