#include <iostream>
#include <cstdint>
#include <string>
#include <bit>
#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>
#include <bitset>

#include "constants.h"
#include "enums.h"

using U64 = uint64_t;

U64 pawn_lookup_attacks[2][64];

U64 knight_lookup_attacks[64];
U64 king_lookup_attacks[64];

U64 rook_lookup_attacks[64][4096];
U64 bishop_lookup_attacks[64][512];

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

inline int str_square_to_index(std::string square){
    if(square.length() < 2){
        printf("Error: str_square_to_index(std::string square) square length < 1");
        exit(-1);
    }
    return (square[0] - 'a') + (square[1] - '1') * 8;
}

// ************************************
// *            FUNCTIONS
// ************************************

inline void set_bit(U64 &bitboard, int square){
    bitboard |= (1ULL << square);
}

inline void pop_bit(U64 &bitboard){
    bitboard &= (bitboard-1);
}

// get least significant bit set(1)
// return index of that bit (0x1 => 0, 0x1000 => 3)
// more precise: number of zeros before FS1B
inline int get_LS1B(U64 &bitboard){
    return std::countr_zero(bitboard);
}

// ************************************
// *            CLASSES
// ************************************

class Move{
public:
    // todo
    //? todo -> ?unsigned?
    unsigned int encoded_value = 0;

    inline int get_from_square(){
        return (encoded_value & 0x3f);
    }

    int get_to_square(){
        return ((encoded_value & 0xfc0) >> 6);
    }

    int get_piece(){
        return ((encoded_value & 0xf000) >> 12);
    }

    int get_move_type(){
        return ((encoded_value & 0xf0000) >> 16);
    }

    void encode_move(int from_square, int to_square, int piece, MoveType move_type){
        encoded_value = 0;
        encoded_value |= from_square;
        encoded_value |= (to_square << 6);
        encoded_value |= (piece << 12);
        encoded_value |= (static_cast<int>(move_type) << 16);
    }

    void print(){
        std::cout << "Move: " << square_str[this->get_from_square()] << square_str[this->get_to_square()] << " " 
        << ascii_pieces[this->get_piece()] << " " << move_type_str[this->get_move_type()] << "\n";
    }
};

// ************************************
// *      BITBOARDS & GAME STATE
// ************************************

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

class GameState{
public:
    // bitboards; index: PIECE enum
    U64 bitboards[12] = {0ULL};

    // 0 - white; 1 - black
    U64 color_occupancy_bitboards[2] = {0ULL};
    U64 both_occupancy_bitboard = 0ULL;

    // castle system - each bit describes one possibility
    // | white queenside | white kingside | black queenside | black kingside |
    // |      bit 0/1    |     bit 0/1    |     bit 0/1     |    bit 0/1     |
    int castles = 0;

    // from enum COLOR -> white = 0, black = 1
    int color_to_move = static_cast<int>(COLOR::none);

    // from enum 0->63 square | -1 none
    int en_passant_square = static_cast<int>(SQUARE::none);

    // halfmoves since last capture or pawn advance for fifty-move rule
    int halfmove_counter = 0;

    // Fullmove number: The number of the full moves. It starts at 1 and is incremented after Black's move.
    int fullmove_number = 1;

    // przeciążenie operatora przypisania
    GameState &operator=(const GameState &other)
    {
        if (this != &other)
        {
            for (int i = 0; i < 12; ++i)
                this->bitboards[i] = other.bitboards[i];

            for (int i = 0; i < 2; ++i)
                this->color_occupancy_bitboards[i] = other.color_occupancy_bitboards[i];

            this->both_occupancy_bitboard = other.both_occupancy_bitboard;

            this->castles = other.castles;
            this->color_to_move = other.color_to_move;
            this->en_passant_square = other.en_passant_square;
            this->halfmove_counter = other.halfmove_counter;
            this->fullmove_number = other.fullmove_number;
        }

        return *this;
    }

    void clear_bitboards()
    {
        for (U64 &b : bitboards)
            b = 0ULL;

        color_occupancy_bitboards[0] = 0ULL;
        color_occupancy_bitboards[1] = 0ULL;
        both_occupancy_bitboard = 0ULL;
    }

    void load_fen(std::string fen)
    {
        // fen fragments
        // 1. piece data
        // 2. active color
        // 3. castling
        // 4. en passant
        // 5. Halfmove clock
        // 6. Fullmove number

        // split fen into 6 fragments (fields)
        std::vector<std::string> fen_fragments(6, "");
        int fragment_index = 0;
        for (char &c : fen)
        {
            if (c == ' ')
            {
                fragment_index++;
            }
            else
            {
                fen_fragments[fragment_index] += c;
            }
        }

        // *** 1.st fragment ***
        // split 1. fragment into ranks
        // !!! ORDER REVERSED !!!
        // fen construction: rank8/rank7/../rank2/rank1 from black to white
        // !position_fragments are from rank1 to rank8

        // clear bitboards
        this->clear_bitboards();

        int square = static_cast<int>(SQUARE::a8);
        std::string fen_position = fen_fragments[0];
        for (int i = 0; i < fen_position.length(); i++)
        {
            char c = fen_position[i];
            if (fen_position[i] == '/')
            {
                // go down the rank
                square = ((square / 8) - 2) * 8;
                continue;
            }

            if ('1' <= fen_position[i] && fen_position[i] <= '8')
            {
                square += (fen_position[i] - '0');
                continue;
            }

            // add piece to bb
            bitboards[piece_ascii_to_number[fen_position[i]]] |= (1ULL << square);
            square++;
        }

        color_occupancy_bitboards[static_cast<int>(COLOR::white)] =
            bitboards[static_cast<int>(PIECE::P)] |
            bitboards[static_cast<int>(PIECE::R)] |
            bitboards[static_cast<int>(PIECE::B)] |
            bitboards[static_cast<int>(PIECE::N)] |
            bitboards[static_cast<int>(PIECE::Q)] |
            bitboards[static_cast<int>(PIECE::K)];

        color_occupancy_bitboards[static_cast<int>(COLOR::black)] =
            bitboards[static_cast<int>(PIECE::p)] |
            bitboards[static_cast<int>(PIECE::r)] |
            bitboards[static_cast<int>(PIECE::b)] |
            bitboards[static_cast<int>(PIECE::n)] |
            bitboards[static_cast<int>(PIECE::q)] |
            bitboards[static_cast<int>(PIECE::k)];

        both_occupancy_bitboard = color_occupancy_bitboards[0] | color_occupancy_bitboards[1];

        // *** 2.st fragment ***
        // set other game state variables
        if (fen_fragments[1] == "w")
            color_to_move = static_cast<int>(COLOR::white);
        else if (fen_fragments[1] == "b")
            color_to_move = static_cast<int>(COLOR::black);
        else
        {
            // todo rise exception
            printf("Error: fen color to move not w or b");
            exit(-1);
        }

        // *** 3.st fragment ***
        // set castling availability (mask)
        castles = 0;
        for (const char &c : fen_fragments[2])
        {
            switch (c)
            {
                // set white queenside
            case 'Q':
                castles |= 0b1000;
                break;

                // set white kingside
            case 'K':
                castles |= 0b0100;
                break;

                // set black queenside
            case 'q':
                castles |= 0b0010;
                break;

                // set black kingside
            case 'k':
                castles |= 0b0001;
                break;

            default:
                break;
            }
        }

        // *** 4.st fragment ***
        // set en passant square
        if (fen_fragments[3] == "-")
            en_passant_square = -1;
        else
        {
            if (fen_fragments[3].length() > 2)
            {
                // todo rise exception
                printf("Error: fen en passant square not '-' and length > 2");
                exit(-1);
            }

            en_passant_square = str_square_to_index(fen_fragments[3]);
        }

        // *** 5.st fragment ***
        // set halfmove counter
        halfmove_counter = std::stoi(fen_fragments[4]);
        // *** 6.st fragment ***
        // set fullmove number
        fullmove_number = std::stoi(fen_fragments[5]);
    }

    void print_game_state()
    {

        printf("#############################\n");
        printf("#      GAME STATUS INFO     #\n");
        printf("#############################\n");

        printf("\n");
        printf("----------------------------- \n");
        printf("----------- BOARD ----------- \n");
        printf("----------------------------- \n");
        printf("\n");

        print_board_unicode();

        printf("\n");
        printf("----------------------------- \n");
        printf("--------- VARIABLES ---------\n");
        printf("----------------------------- \n");
        printf("\n");

        printf("- Color to move:\n");
        printf("\tint: %d\n", color_to_move);
        printf("\tstr: %s\n", color_to_move ? "black" : "white");

        printf("- Castling availability:\n");
        printf("\tbin: %c%c%c%c\n",
               (castles & 1) ? '1' : '0',
               (castles & 2) ? '1' : '0',
               (castles & 4) ? '1' : '0',
               (castles & 8) ? '1' : '0');
        printf("\tstr: %c%c%c%c\n",
               (castles & 1) ? 'Q' : '-',
               (castles & 2) ? 'K' : '-',
               (castles & 4) ? 'q' : '-',
               (castles & 8) ? 'k' : '-');
        printf("- En passant square: \n");
        printf("\tint: %d\n", en_passant_square);
        std::cout << "\tstr: " << ((en_passant_square < 0) ? "-" : square_str[en_passant_square]) << std::endl;
        printf("- Halfmove counter: %d\n", halfmove_counter);
        printf("- Fullmove number: %d\n", fullmove_number);

        printf("---------------------------\n");
    }

    void print_board_unicode()
    {
        std::vector<std::string> pieces(64, ".");

        // for each bitboard (piece type) set its characters
        for (int bitboard_index = 0; bitboard_index < 12; bitboard_index++)
        {
            U64 piece_bitboard = this->bitboards[bitboard_index];
            // until 1 bits available in current piece bitboard
            while (piece_bitboard)
            {
                // get LS1B index (count the number of consecutive 0 bits)
                int square_number = get_LS1B(piece_bitboard);

                // set unicode character
                pieces[square_number] = unicode_pieces[bitboard_index];

                // delete LS1B
                piece_bitboard = piece_bitboard & (piece_bitboard - 1);
            }
        }

        print_board_of_strings(pieces);
    }
};

// ------------------------------------------

//todo: functions (for example init lookup tables) must clear state (both_occupancies)

// ************************************
// *         ATTACK GENERATION
// ************************************

U64 calculate_bishop_attacks(int square, GameState &game_state){
    U64 attacks = 0ULL;

    // *up-right direction
    U64 cursor = 1ULL << square;
    // 46 is last interesting square
    for(int sq = square; (sq / 8 < 7) & (sq % 8 < 7); sq+=9){
        // move cursor
        cursor <<= 9;
        // save relevant square
        attacks |= cursor;

        // blocker
        if(cursor & game_state.both_occupancy_bitboard)
            break;
    }

    // *up-left direction
    // reset cursor to square
    cursor = 1ULL << square;
    // 47 is last interesting square
    for(int sq = square; (sq / 8 < 7) & (sq % 8 > 0); sq+=7){
        // move cursor
        cursor <<= 7;

        // save relevant square
        attacks |= cursor;

        // blocker
        if(cursor & game_state.both_occupancy_bitboard)
            break;
    }

    // *down-left direction
    // reset cursor to square
    cursor = 1ULL << square;
    // 18 is first interesting square
    for(int sq = square; (sq / 8 > 0) & (sq % 8 > 0); sq-=9){
        // move cursor
        cursor >>= 9;

        // save relevant square
        attacks |= cursor;

        // blocker
        if(cursor & game_state.both_occupancy_bitboard)
            break;
    }

    // *down-right direction
    // reset cursor to square
    cursor = 1ULL << square;
    // 21 is first interesting square
    for(int sq = square; (sq / 8 > 0) & (sq % 8 < 7); sq-=7){
        // move cursor
        cursor >>= 7;

        // save relevant square
        attacks |= cursor;

        // blocker
        if(cursor & game_state.both_occupancy_bitboard)
            break;
    }
    
    return attacks;
}

U64 calculate_rook_attacks(int square, GameState &game_state){
    U64 attacks = 0ULL;
    // * up
    U64 cursor = 1ULL << square;
    for(int sq = square; (sq / 8 < 7); sq+=8){
        // move cursor
        cursor <<= 8;

        // save relevant square
        attacks |= cursor;

        // blocker
        if(cursor & game_state.both_occupancy_bitboard)
            break;
    }

    // * down
    // reset cursor
    cursor = 1ULL << square;
    for(int sq = square; (sq / 8 > 0); sq-=8){
        // move cursor
        cursor >>= 8;

        // save relevant square
        attacks |= cursor;

        // blocker
        if(cursor & game_state.both_occupancy_bitboard)
            break;
    }

    // * left
    // reset cursor
    cursor = 1ULL << square;
    for(int sq = square; (sq % 8 > 0); sq-=1){
        // move cursor
        cursor >>= 1;

        // save relevant square
        attacks |= cursor;

        // blocker
        if(cursor & game_state.both_occupancy_bitboard)
            break;
    }

    // * right
    // reset cursor
    cursor = 1ULL << square;
    for(int sq = square; (sq % 8 < 7); sq+=1){
        // move cursor
        cursor <<= 1;

        // save relevant square
        attacks |= cursor;

        // blocker
        if(cursor & game_state.both_occupancy_bitboard)
            break;
    }

    return attacks;
}

U64 rook_relevant_occupancy(int square){
    int rank = square / 8;
    int file = square % 8;
    U64 border = RANK_1_MASK ^ RANK_8_MASK ^ A_FILE_MASK ^ H_FILE_MASK;
    // border:
    // 8   0 1 1 1 1 1 1 0   8
    // 7   1 0 0 0 0 0 0 1   7
    // 6   1 0 0 0 0 0 0 1   6
    // 5   1 0 0 0 0 0 0 1   5
    // 4   1 0 0 0 0 0 0 1   4
    // 3   1 0 0 0 0 0 0 1   3
    // 2   1 0 0 0 0 0 0 1   2
    // 1   0 1 1 1 1 1 1 0   1

    // if square on border:
    if (rank == 0 || rank == 7){
        border = border ^ RANK_MASK_ARR[rank];
    }
    if (file == 0 || file == 7){
        border = border ^ FILE_MASK_ARR[file];
    }
    // XOR these edges (on square position); here square = a1
    // 8   1 1 1 1 1 1 1 0   8
    // 7   0 0 0 0 0 0 0 1   7
    // 6   0 0 0 0 0 0 0 1   6
    // 5   0 0 0 0 0 0 0 1   5
    // 4   0 0 0 0 0 0 0 1   4
    // 3   0 0 0 0 0 0 0 1   3
    // 2   0 0 0 0 0 0 0 1   2
    // 1   0 0 0 0 0 0 0 1   1

    U64 relevant_occupancy = 
    // make cross on square position (vertical and horizontal, actually '+' sign)
    (RANK_MASK_ARR[rank] | FILE_MASK_ARR[file]) 
    // remove border from relevant ocupancies
    & ~(border) 
    // remove square
    & ~(1ULL << square);

    return relevant_occupancy;
}

U64 bishop_relevant_occupancy(int square){
    U64 relevant_occupancy = 0ULL;

    // *up-right direction
    U64 cursor = 1ULL << square;
    // 46 is last interesting square
    for(int sq = square; (sq / 8 < 6) & (sq % 8 < 6); sq+=9){
        // move cursor
        cursor <<= 9;

        // save relevant square
        relevant_occupancy |= cursor;
    }

    // *up-left direction
    // reset cursor to square
    cursor = 1ULL << square;
    // 47 is last interesting square
    for(int sq = square; (sq / 8 < 6) & (sq % 8 > 1); sq+=7){
        // move cursor
        cursor <<= 7;

        // save relevant square
        relevant_occupancy |= cursor;
    }

    // *down-left direction
    // reset cursor to square
    cursor = 1ULL << square;
    // 18 is first interesting square
    for(int sq = square; (sq / 8 > 1) & (sq % 8 > 1); sq-=9){
        // move cursor
        cursor >>= 9;

        // save relevant square
        relevant_occupancy |= cursor;
    }

    // *down-right direction
    // reset cursor to square
    cursor = 1ULL << square;
    // 21 is first interesting square
    for(int sq = square; (sq / 8 > 1) & (sq % 8 < 6); sq-=7){
        // move cursor
        cursor >>= 7;

        // save relevant square
        relevant_occupancy |= cursor;
    }
    
    return relevant_occupancy;
}

U64 pawn_attacks(int square, int color){
    U64 piece_position = 1Ull << square;
    if(color == static_cast<int>(COLOR::white)){
        // bit shift and mask overflowing bits
        return ((piece_position << 9) & NOT_A_FILE) | ((piece_position << 7) & NOT_H_FILE);
    }
    
    // bit shift and mask overflowing bits
    return ((piece_position >> 7) & NOT_A_FILE) | ((piece_position >> 9) & NOT_H_FILE);
}

U64 king_attacks(int square){
    // left    right
    // <<7 <<8 <<9
    // >>1  K  <<1
    // >>9 >>8 >>7
    // left side moves mask with NOT_H_RANK
    // right side moves mask with NOT_A_RANK

    U64 piece_position = 1Ull << square;

    U64 attacks =
    // up
    (piece_position << 8) |
    //down
    (piece_position >> 8) |
    //right
    (((piece_position << 9) | (piece_position << 1) | (piece_position >> 7)) & NOT_A_FILE) |
    //left
    (((piece_position << 7) | (piece_position >> 1) | (piece_position >> 9)) & NOT_H_FILE);

    return attacks;
}

U64 knight_attacks(int square){
    U64 piece_position = 1Ull << square;

    //| GH |  H |   |  A | AB |
    //|    |<<15| - |<<17|    |
    //|<<6 |    | - |    |<<10|
    //|    |    | N |    |    |
    //|>>10|    | - |    |>>6 |
    //|    |>>17| - |>>15|    |
    // moves must be masked to prevent overflowing moves

    U64 attacks =
    // A mask
    (((piece_position << 17) | (piece_position >> 15)) & NOT_A_FILE) |
    // AB mask
    (((piece_position << 10) | (piece_position >> 6)) & NOT_AB_FILE) |
    // H mask
    (((piece_position << 15) | (piece_position >> 17)) & NOT_H_FILE) |
    // GH mask
    (((piece_position << 6) | (piece_position >> 10)) & NOT_GH_FILE);

    return attacks;
    
}

U64 rook_attacks(int square, GameState &game_state){
    U64 magic_number = rook_magic_numbers[square];
    U64 relevant_occupancy = rook_relevant_occupancy(square) & game_state.both_occupancy_bitboard;
    int magic_index = relevant_occupancy * magic_number >> (64-rook_relevant_occupancy_count[square]);

    return rook_lookup_attacks[square][magic_index];
}

U64 bishop_attacks(int square, GameState &game_state){
    U64 magic_number = bishop_magic_numbers[square];
    U64 relevant_occupancy = bishop_relevant_occupancy(square) & game_state.both_occupancy_bitboard;
    int magic_index = relevant_occupancy * magic_number >> (64-bishop_relevant_occupancy_count[square]);

    return bishop_lookup_attacks[square][magic_index];
}

U64 queen_attacks(int square, GameState &game_state){
    return (bishop_attacks(square, game_state) | rook_attacks(square, game_state));
}

// not in use
// todo test
void print_relevant_occupancy_count_tables(){
    // set relevant occupancy count tables
    std::cout << "BISHOP RELEVANT OCCUPANCY COUNT: \n";
    for(int i = 0; i < 64; i++){
        // count of set bits (1 bits)
        std::cout << std::popcount(bishop_relevant_occupancy(i)) << ", ";
        if((i+1)%8 == 0) 
            std::cout << "\n";
    }

    std::cout << "\n\nROOK RELEVANT OCCUPANCY COUNT: \n";
    for(int i = 0; i < 64; i++){
        // count of set bits (1 bits)
        std::cout << std::popcount(rook_relevant_occupancy(i)) << ", ";
        if((i+1)%8 == 0) 
            std::cout << "\n";
    }
}

void generate_magic_numbers(bool rook, GameState &game_state){
    // random number generator
    constexpr U64 SEED = 123456789ULL;
    std::mt19937_64 gen(SEED);
    std::uniform_int_distribution<U64> dist(0, ~0ULL);

    int correct_numbers = 0;
    
    for (int square = 0; square < 64; square++){
        // !tmp
        U64 att = 0ULL;
        U64 time = 0;
        
        // auto start = std::chrono::high_resolution_clock::now();

        // tmp
        U64 magic_number = 0;
        int try_index = 0;
        for(try_index = 0; try_index < 1'000'000; try_index++){
            magic_number = dist(gen) & dist(gen) & dist(gen);
            // std::cout << magic_number << "\n";
            bool fail = false;

            U64 attack_table[4096] = {0};
            int relevant_bits = rook ? rook_relevant_occupancy_count[square]
                                     : bishop_relevant_occupancy_count[square];
            int variations = 1 << relevant_bits;
            for (int variation = 0; variation < variations && !fail; variation++) {
                // todo
                att++;

                U64 relevant_occupancy = 0ULL;
                int index = 0;

                U64 occupation_mask = rook ? 
                rook_relevant_occupancy(square) : 
                bishop_relevant_occupancy(square);

                while(occupation_mask){
                    int mask_bit = get_LS1B(occupation_mask);
                    
                    // get bit from variation and put under set (1) bit in relevant_occupancy
                    // relevant_occupancy |= ( !!(variation & (1ULL << index)) << mask_bit );
                    U64 bit = (variation & (1ULL << index)) ? 1ULL : 0ULL;
                    relevant_occupancy |= (bit << mask_bit);
                    
                    pop_bit(occupation_mask);
                    index++;
                }
                
                // relevant occupancy contains pieces configuration on rook/bishop sight ray
                int magic_index = 0;
                if(rook)
                    magic_index = relevant_occupancy * magic_number >> (64-rook_relevant_occupancy_count[square]);
                else
                    magic_index = relevant_occupancy * magic_number >> (64-bishop_relevant_occupancy_count[square]);

                // set relevant occupancy to board
                game_state.both_occupancy_bitboard = relevant_occupancy;
                // oparates on both_occupancies
                U64 attacks = rook ? calculate_rook_attacks(square, game_state) : calculate_bishop_attacks(square, game_state);
                
                // reset both_occupancy_bitboard
                game_state.both_occupancy_bitboard = 0ULL;

                if(attack_table[magic_index] && attack_table[magic_index] != attacks){
                    // failed! other magic number
                    fail = true;
                    break;
                }
                else{
                    attack_table[magic_index] = attacks;
                }
            }
            
            if(!fail){
                // magic number is correct
                correct_numbers++;
                
                break;
            }
        }
        // auto stop = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        // time += duration.count();
        
        std::cout << magic_number << "ULL,\n";
        // std::cout << "iter: " << try_index << "\n";
        // std::cout << "attempts: " << att << "\n";
        // std::cout << "duration: " << time/1000000.0 << "[s]\n";
        // std::cout << "ratio: " << ((double)time)/att << "[us]\n";
        // std::cout << "\n";

    }

    printf("correct numbers: %d\n",correct_numbers);
}

void init_rook_bishop_lookup_tables(bool rook, GameState &game_state){
    for(int square = 0; square < 64; square++){
        for(int variation = 0; (variation < (rook ? 4096 : 512)); variation++){
            U64 relevant_occupancy = 0ULL;
            int index = 0;

            U64 occupation_mask = rook ? 
            rook_relevant_occupancy(square) : 
            bishop_relevant_occupancy(square);

            while(occupation_mask){
                int mask_bit = get_LS1B(occupation_mask);
                
                // get bit from variation and put under set (1) bit in relevant_occupancy
                U64 bit = (variation & (1ULL << index)) ? 1ULL : 0ULL;
                relevant_occupancy |= (bit << mask_bit);
                
                pop_bit(occupation_mask);
                index++;
            }
            int magic_index = rook ? 
            relevant_occupancy * rook_magic_numbers[square] >> (64-rook_relevant_occupancy_count[square]) : 
            relevant_occupancy * bishop_magic_numbers[square] >> (64-bishop_relevant_occupancy_count[square]);

            // set relevant occupancy to board
            game_state.both_occupancy_bitboard = relevant_occupancy;
            // oparates on both_occupancies
            U64 attacks = rook ? calculate_rook_attacks(square, game_state) : calculate_bishop_attacks(square, game_state);

            if(rook)
                rook_lookup_attacks[square][magic_index] = attacks;
            else
                bishop_lookup_attacks[square][magic_index] = attacks;
        }   
    }
}

void init_pawn_lookup_table(){
    for( int side = 0; side < 2; side++){
        for (int square = 0; square < 64; square++){
            pawn_lookup_attacks[side][square] = pawn_attacks(square, side);
        }
    }
}

void init_king_lookup_table(){
    for (int square = 0; square < 64; square++){
        king_lookup_attacks[square] = king_attacks(square);
    }
}

void init_knight_lookup_table(){
    for (int square = 0; square < 64; square++){
        knight_lookup_attacks[square] = knight_attacks(square);
    }
}

void init_all_lookup_tables(GameState &game_state){
    init_rook_bishop_lookup_tables(true, game_state);
    init_rook_bishop_lookup_tables(false, game_state);
    init_pawn_lookup_table();
    init_king_lookup_table();
    init_knight_lookup_table();
}

// ************************************
// *         MOVE GENERATION
// ************************************

// is given <square> attacked by <side>
bool is_square_attacked_by(int square, int side, GameState &game_state){
    // super-piece technic
    // set pieces on current <square> and intersect attacks with appropriate pieces
    // for pawn intersect enemy pawns => white intersect black and vice versa
    
    // pawns
    // set enemy piece on <square> and check does it attack friendly pawns
    if (pawn_lookup_attacks[!side][square] & game_state.bitboards[ static_cast<int>(PIECE::P) + (!side * 6) ])
        return true;

    // todo sprawdzić kiedyś różnice w wydajności
    /* if(side == static_cast<int>(COLOR::white)){
        if (pawn_attacks(square, static_cast<int>(COLOR::black)) & bitboards[static_cast<int>(PIECE::p)])
            return true;
    }
    else{
        if (pawn_attacks(square, static_cast<int>(COLOR::white)) & bitboards[static_cast<int>(PIECE::P)])
            return true;
    }*/

    // knight
    // intersect with friendly knights
    if (knight_lookup_attacks[square] & game_state.bitboards[static_cast<int>(PIECE::N) + (side * 6)])
        return true;

    // bishop
    // intersect with bishops and queens
    if (bishop_attacks(square, game_state) & (game_state.bitboards[static_cast<int>(PIECE::B) + (side * 6)] | game_state.bitboards[static_cast<int>(PIECE::Q) + (side * 6)]))
        return true;

    // rook
    if (rook_attacks(square, game_state) & (game_state.bitboards[static_cast<int>(PIECE::R) + (side * 6)] | game_state.bitboards[static_cast<int>(PIECE::Q) + (side * 6)]))
        return true;

    // king
    if (king_lookup_attacks[square] & game_state.bitboards[static_cast<int>(PIECE::K) + (side * 6)])
        return true;

    return false;
}

// debug / testing
U64 get_attacked_squares(int side, GameState &game_state){
    U64 result = 0ULL;

    for(int square = 0; square < 64; square++){
        result |= is_square_attacked_by(square, side, game_state) ? (1ULL << square) : 0ULL;
    }

    return result;
}

std::vector<Move> generate_moves(GameState &game_state){
    int from_square = 0, to_square = 0;
    U64 pice_bitboard_copy = 0ULL;
    U64 attacks = 0ULL;

    std::vector<Move> moves;

    //for each piece type in <color_to_move>
    for(int piece = static_cast<int>(PIECE::P) + (game_state.color_to_move*6); piece <= static_cast<int>(PIECE::K) + (game_state.color_to_move*6); piece++){
        pice_bitboard_copy = game_state.bitboards[piece];

        // while pieces on BB
        while (pice_bitboard_copy)
        {
            from_square = get_LS1B(pice_bitboard_copy);

            //* pawn moves
            if(piece == static_cast<int>(PIECE::P) || piece == static_cast<int>(PIECE::p)){
                bool is_on_promotion = 
                    (from_square >= static_cast<int>(SQUARE::a7) && from_square <= static_cast<int>(SQUARE::h7) && game_state.color_to_move == static_cast<int>(COLOR::white)) ||
                    (from_square >= static_cast<int>(SQUARE::a2) && from_square <= static_cast<int>(SQUARE::h2) && game_state.color_to_move == static_cast<int>(COLOR::black));

                // attacks
                attacks = pawn_lookup_attacks[game_state.color_to_move][from_square];

                // for each attacking square
                while (attacks){
                    to_square = get_LS1B(attacks);

                    // if en passant capture
                    if(game_state.en_passant_square == to_square){
                        // std::cout << "Pawn capture move en_passnat: " << square_str[from_square] << "x" << square_str[to_square] << "\n";
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6),MoveType::en_passant_capture);
                        moves.push_back(move);
                    }
                    
                    // if enemy add move
                    else if(game_state.color_occupancy_bitboards[ !game_state.color_to_move ] & (1ULL << to_square)){
                        // last rank promotion
                        if(is_on_promotion){
                            // add move
                            // std::cout << "Pawn capture promotion Rr: " << square_str[from_square] << "x" << square_str[to_square] << "\n";
                            // std::cout << "Pawn capture promotion Bb: " << square_str[from_square] << "x" << square_str[to_square] << "\n";
                            // std::cout << "Pawn capture promotion Nn: " << square_str[from_square] << "x" << square_str[to_square] << "\n";
                            // std::cout << "Pawn capture promotion Qq: " << square_str[from_square] << "x" << square_str[to_square] << "\n";
                            Move move[4];
                            move[0].encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::rook_promo_capture);
                            move[1].encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::bishop_promo_capture);
                            move[2].encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::knight_promo_capture);
                            move[3].encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::queen_promo_capture);
                            moves.push_back(move[0]);
                            moves.push_back(move[1]);
                            moves.push_back(move[2]);
                            moves.push_back(move[3]);
                        }
                        else{
                            // add move
                            // std::cout << "Pawn capture move: " << square_str[from_square] << "x" << square_str[to_square] << "\n";
                            Move move;
                            move.encode_move(from_square, to_square, static_cast<int>(PIECE::P) * (game_state.color_to_move*6), MoveType::capture);
                            moves.push_back(move);
                        }
                    }
                    
                    // remove LS1B
                    pop_bit(attacks);
                }

                // single push
                to_square = game_state.color_to_move == static_cast<int>(COLOR::white) ? from_square + 8 : from_square - 8;
                bool is_target_square_empty = ( game_state.both_occupancy_bitboard & (1ULL << to_square) ) == 0;

                if(is_target_square_empty){
                    if(is_on_promotion){
                        Move move[4];
                        move[0].encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::rook_promotion);
                        move[1].encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::bishop_promotion);
                        move[2].encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::knight_promotion);
                        move[3].encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::queen_promotion);
                        moves.push_back(move[0]);
                        moves.push_back(move[1]);
                        moves.push_back(move[2]);
                        moves.push_back(move[3]);
                        // std::cout << "Pawn promotion Rr: " << square_str[from_square] << square_str[to_square] << "\n";
                        // std::cout << "Pawn promotion Nn: " << square_str[from_square] << square_str[to_square] << "\n";
                        // std::cout << "Pawn promotion Bb: " << square_str[from_square] << square_str[to_square] << "\n";
                        // std::cout << "Pawn promotion Qq: " << square_str[from_square] << square_str[to_square] << "\n";
                    }
                    else{
                        // std::cout << "Pawn single push: " << square_str[from_square] << square_str[to_square] << "\n";
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::quiet_move);
                        moves.push_back(move);
                    }

                    // double push
                    to_square = game_state.color_to_move == static_cast<int>(COLOR::white) ? from_square + 16 : from_square - 16;
                    bool is_on_starting_rank = 
                        (from_square >= static_cast<int>(SQUARE::a2) && from_square <= static_cast<int>(SQUARE::h2) && game_state.color_to_move == static_cast<int>(COLOR::white)) ||
                        (from_square >= static_cast<int>(SQUARE::a7) && from_square <= static_cast<int>(SQUARE::h7) && game_state.color_to_move == static_cast<int>(COLOR::black));
                    is_target_square_empty = ( game_state.both_occupancy_bitboard & (1ULL << to_square) ) == 0;

                    if(is_on_starting_rank && is_target_square_empty){
                        // std::cout << "Pawn double push: " << square_str[from_square] << square_str[to_square] << "\n";
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::double_pawn_push);
                        moves.push_back(move);
                    }
                }
            }


            //* knight moves
            if(piece == static_cast<int>(PIECE::N) || piece == static_cast<int>(PIECE::n)){
                attacks = knight_lookup_attacks[from_square];

                while(attacks){
                    to_square = get_LS1B(attacks);

                    bool is_target_square_empty = ( game_state.both_occupancy_bitboard & (1ULL << to_square) ) == 0;
                    bool is_enemy = game_state.color_occupancy_bitboards[!game_state.color_to_move] & (1ULL << to_square);
                    if(is_target_square_empty){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::N) + (game_state.color_to_move*6), MoveType::quiet_move);
                        moves.push_back(move);
                        // std::cout << "Knight move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }
                    else if(is_enemy){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::N) + (game_state.color_to_move*6), MoveType::capture);
                        moves.push_back(move);
                        // std::cout << "Knight capture move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }

                    pop_bit(attacks);
                }
            }

            //* bishop moves
            if(piece == static_cast<int>(PIECE::B) || piece == static_cast<int>(PIECE::b)){
                attacks = bishop_attacks(from_square, game_state);

                while(attacks){
                    to_square = get_LS1B(attacks);

                    bool is_target_square_empty = ( game_state.both_occupancy_bitboard & (1ULL << to_square) ) == 0;
                    bool is_enemy = game_state.color_occupancy_bitboards[!game_state.color_to_move] & (1ULL << to_square);
                    if(is_target_square_empty){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::B) + (game_state.color_to_move*6), MoveType::quiet_move);
                        moves.push_back(move);
                        // std::cout << "Bishop move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }
                    else if(is_enemy){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::B) + (game_state.color_to_move*6), MoveType::capture);
                        moves.push_back(move);
                        // std::cout << "Bishop capture move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }

                    pop_bit(attacks);
                }
            }

            //* rook moves
            if(piece == static_cast<int>(PIECE::R) || piece == static_cast<int>(PIECE::r)){
                attacks = rook_attacks(from_square, game_state);

                while(attacks){
                    to_square = get_LS1B(attacks);

                    bool is_target_square_empty = ( game_state.both_occupancy_bitboard & (1ULL << to_square) ) == 0;
                    bool is_enemy = game_state.color_occupancy_bitboards[!game_state.color_to_move] & (1ULL << to_square);
                    if(is_target_square_empty){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::R) + (game_state.color_to_move*6), MoveType::quiet_move);
                        moves.push_back(move);
                        // std::cout << "Rook move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }
                    else if(is_enemy){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::R) + (game_state.color_to_move*6), MoveType::capture);
                        moves.push_back(move);
                        // std::cout << "Rook capture move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }

                    pop_bit(attacks);
                }
            }

            //* queen moves
            if(piece == static_cast<int>(PIECE::Q) || piece == static_cast<int>(PIECE::q)){
                attacks = queen_attacks(from_square, game_state);

                while(attacks){
                    to_square = get_LS1B(attacks);

                    bool is_target_square_empty = ( game_state.both_occupancy_bitboard & (1ULL << to_square) ) == 0;
                    bool is_enemy = game_state.color_occupancy_bitboards[!game_state.color_to_move] & (1ULL << to_square);
                    if(is_target_square_empty){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::Q) + (game_state.color_to_move*6), MoveType::quiet_move);
                        moves.push_back(move);
                        // std::cout << "Queen move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }
                    else if(is_enemy){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::Q) + (game_state.color_to_move*6), MoveType::capture);
                        moves.push_back(move);
                        // std::cout << "Queen capture move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }

                    pop_bit(attacks);
                }
            }

            //* king moves
            if(piece == static_cast<int>(PIECE::K) || piece == static_cast<int>(PIECE::k)){
                attacks = king_lookup_attacks[from_square];

                while(attacks){
                    to_square = get_LS1B(attacks);

                    bool is_target_square_empty = ( game_state.both_occupancy_bitboard & (1ULL << to_square) ) == 0;
                    bool is_enemy = game_state.color_occupancy_bitboards[!game_state.color_to_move] & (1ULL << to_square);
                    if(is_target_square_empty){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::quiet_move);
                        moves.push_back(move);
                        // std::cout << "King move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }
                    else if(is_enemy){
                        Move move;
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::capture);
                        moves.push_back(move);
                        // std::cout << "King capture move: " << square_str[from_square] << square_str[to_square] << "\n";
                    }

                    pop_bit(attacks);
                }

                // castling
                // castle system - each bit describes one possibility
                // | white queenside | white kingside | black queenside | black kingside |
                // |      bit 0/1    |     bit 0/1    |     bit 0/1     |    bit 0/1     |
                // castle state | empty squares | not attacked

                // white side
                if(game_state.color_to_move == static_cast<int>(COLOR::white)){
                    // is QUEENside castling possible (game state)
                    if(game_state.castles & 0b1000){
                        // are squares between king and rook empty
                        if((game_state.both_occupancy_bitboard & white_queenside_empty_squares_castling_mask) == 0){
                            // king and square next to him is not under attack
                            // is NOT attacked (e1) && is NOT attacked (d1)
                            if(!is_square_attacked_by(static_cast<int>(SQUARE::e1), !game_state.color_to_move, game_state) && !is_square_attacked_by(static_cast<int>(SQUARE::d1), !game_state.color_to_move, game_state)){
                                // std::cout << "White queenside castle: e1c1 O-O-O \n";
                                Move move;
                                move.encode_move(from_square, to_square, static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::queen_castle);
                                moves.push_back(move);
                            }
                        }
                    }

                    // is KINGside castling possible (game state)
                    if(game_state.castles & 0b0100){
                        // are squares between king and rook empty
                        if((game_state.both_occupancy_bitboard & white_kingside_empty_squares_castling_mask) == 0){
                            // king and square next to him is not under attack
                            // is NOT attacked (e1) && is NOT attacked (f1)
                            if(!is_square_attacked_by(static_cast<int>(SQUARE::e1), !game_state.color_to_move, game_state) && !is_square_attacked_by(static_cast<int>(SQUARE::f1), !game_state.color_to_move, game_state)){
                                // std::cout << "White kingside castle: e1g1 O-O \n";
                                Move move;
                                move.encode_move(from_square, to_square, static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::king_castle);
                                moves.push_back(move);
                            }
                        }
                    }
                }else{
                    // is QUEENside castling possible (game state)
                    if(game_state.castles & 0b0010){
                        // are squares between king and rook empty
                        if((game_state.both_occupancy_bitboard & black_queenside_empty_squares_castling_mask) == 0){
                            // king and square next to him is not under attack
                            // is NOT attacked (e8) && is NOT attacked (d8)
                            if(!is_square_attacked_by(static_cast<int>(SQUARE::e8), !game_state.color_to_move, game_state) && !is_square_attacked_by(static_cast<int>(SQUARE::d8), !game_state.color_to_move, game_state)){
                                // std::cout << "Black queenside castle: e8c8 O-O-O \n";
                                Move move;
                                move.encode_move(from_square, to_square, static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::queen_castle);
                                moves.push_back(move);
                            }
                        }
                    }

                    // is KINGside castling possible (game state)
                    if(game_state.castles & 0b0001){
                        // are squares between king and rook empty
                        if((game_state.both_occupancy_bitboard & black_kingside_empty_squares_castling_mask) == 0){
                            // king and square next to him is not under attack
                            // is NOT attacked (e8) && is NOT attacked (f8)
                            if(!is_square_attacked_by(static_cast<int>(SQUARE::e8), !game_state.color_to_move, game_state) && !is_square_attacked_by(static_cast<int>(SQUARE::f8), !game_state.color_to_move, game_state)){
                                // std::cout << "Black kingside castle: e8g8 O-O \n";
                                Move move;
                                move.encode_move(from_square, to_square, static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::king_castle);
                                moves.push_back(move);
                            }
                        }
                    }
                }

            }
            
            // remove LS1B
            pop_bit(pice_bitboard_copy);
        }
    }

    return moves;
}

// ************************************
// *          VISUALISATION
// ************************************

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

void print_board_ascii(GameState &game_state){
    std::vector<std::string> pieces(64, "0");


    // for each bitboard (piece type) set its characters
    for(int bitboard_index = 0; bitboard_index < 12; bitboard_index++){
        U64 piece_bitboard = game_state.bitboards[bitboard_index];
        // until 1 bits available in current piece bitboard
        while(piece_bitboard){
            // get LS1B index (count the number of consecutive 0 bits) 
            int square_number = get_LS1B(piece_bitboard);

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