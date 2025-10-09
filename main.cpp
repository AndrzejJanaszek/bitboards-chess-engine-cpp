#include <iostream>
#include <cstdint>
#include <string>
#include <bit>
#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>
#include <bitset>

// ************************************
// *         DEFINE STATEMENTS
// ************************************

#define U64 uint64_t

// ************************************
// *              ENUMS
// ************************************

enum class COLOR {
    white, black
};

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

inline int str_square_to_index(std::string square){
    if(square.length() < 2){
        printf("Error: str_square_to_index(std::string square) square length < 1");
        exit(-1);
    }
    return (square[0] - 'a') + (square[1] - '1') * 8;
}

// ************************************
// *               CONST
// ************************************

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

const char ascii_pieces[] = {
    'P', 'R', 'N', 'B', 'Q', 'K',
    'p', 'r', 'n', 'b', 'q', 'k'
};

const std::string square_str[] = {
    "a1","b1","c1","d1","e1","f1","g1","h1",
    "a2","b2","c2","d2","e2","f2","g2","h2",
    "a3","b3","c3","d3","e3","f3","g3","h3",
    "a4","b4","c4","d4","e4","f4","g4","h4",
    "a5","b5","c5","d5","e5","f5","g5","h5",
    "a6","b6","c6","d6","e6","f6","g6","h6",
    "a7","b7","c7","d7","e7","f7","g7","h7",
    "a8","b8","c8","d8","e8","f8","g8","h8"
};

const std::vector<const std::string> unicode_pieces = {
    // white pieces
    "♙","♖","♘","♗","♕","♔",
    // black pieces
    "♟","♜","♞","♝","♛","♚"
};

const char rank_names[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

const U64 A_FILE_MASK = 0x101010101010101ULL;
const U64 B_FILE_MASK = A_FILE_MASK << 1;
const U64 C_FILE_MASK = A_FILE_MASK << 2;
const U64 D_FILE_MASK = A_FILE_MASK << 3;
const U64 E_FILE_MASK = A_FILE_MASK << 4;
const U64 F_FILE_MASK = A_FILE_MASK << 5;
const U64 G_FILE_MASK = A_FILE_MASK << 6;
const U64 H_FILE_MASK = A_FILE_MASK << 7;

const U64 RANK_1_MASK = 0x000000000000ffULL;
const U64 RANK_2_MASK = RANK_1_MASK << (8*1);
const U64 RANK_3_MASK = RANK_1_MASK << (8*2);
const U64 RANK_4_MASK = RANK_1_MASK << (8*3);
const U64 RANK_5_MASK = RANK_1_MASK << (8*4);
const U64 RANK_6_MASK = RANK_1_MASK << (8*5);
const U64 RANK_7_MASK = RANK_1_MASK << (8*6);
const U64 RANK_8_MASK = RANK_1_MASK << (8*7);

const U64 FILE_MASK_ARR[] = {
    A_FILE_MASK,
    B_FILE_MASK,
    C_FILE_MASK,
    D_FILE_MASK,
    E_FILE_MASK,
    F_FILE_MASK,
    G_FILE_MASK,
    H_FILE_MASK
};

const U64 RANK_MASK_ARR[] = {
    RANK_1_MASK,
    RANK_2_MASK,
    RANK_3_MASK,
    RANK_4_MASK,
    RANK_5_MASK,
    RANK_6_MASK,
    RANK_7_MASK,
    RANK_8_MASK
};

const U64 NOT_A_FILE = (~0Ull) ^ A_FILE_MASK;
const U64 NOT_AB_FILE = (~0Ull) ^ (A_FILE_MASK | B_FILE_MASK);
const U64 NOT_H_FILE = (~0Ull) ^ H_FILE_MASK;
const U64 NOT_GH_FILE = (~0Ull) ^ (G_FILE_MASK | H_FILE_MASK);

constexpr U64 rook_magic_numbers[64]={
2630102260767531144ULL,
234222385128030208ULL,
2522025687206854848ULL,
9295436263664394496ULL,
144132917837431297ULL,
72060892977299458ULL,
2377973725069967364ULL,
4935946291246022784ULL,
14988120309188682368ULL,
13835339805145236128ULL,
563294092001664ULL,
140771856519168ULL,
864972637841918992ULL,
311311392995017736ULL,
9241667957585739780ULL,
4611967511666237696ULL,
141287247396864ULL,
81074139686879232ULL,
9289774013612096ULL,
4758618155430993988ULL,
581105639242139648ULL,
2315132783057698856ULL,
2308974688928264ULL,
145243287012311956ULL,
35736275533840ULL,
27024365874841185ULL,
9876623514467860992ULL,
9043487433953280ULL,
83316629621965824ULL,
10133107785076744ULL,
1170953512490768664ULL,
270217085743825092ULL,
18014983723942048ULL,
4629701172853213184ULL,
2380720164489400961ULL,
18155204725708800ULL,
18384934011930625ULL,
563018706470924ULL,
5188234766356316418ULL,
144115480234295429ULL,
158604554960896ULL,
2305913515665539080ULL,
634418746163218ULL,
144546196902412416ULL,
9083065691340816ULL,
38562123350605826ULL,
78830594389442561ULL,
563509389819908ULL,
6341209031621804288ULL,
1162492204661486080ULL,
4611864690174726656ULL,
2310504942944813952ULL,
2704415974416810112ULL,
5794428426125440ULL,
12738572326455541888ULL,
4964998989312ULL,
21445701569880321ULL,
81135166333980801ULL,
162164775263424577ULL,
32932641001048069ULL,
23081154786100234ULL,
9223653529018174081ULL,
36045848307632132ULL,
576480698173554754ULL
};

constexpr U64 bishop_magic_numbers[64] = {
18089199962039328ULL,
1914034534219796745ULL,
298364592021577728ULL,
9811102870857056258ULL,
23656027031470080ULL,
216322471387734018ULL,
16447717604732452874ULL,
141013576321156ULL,
144125496137908480ULL,
10309055952273608ULL,
4569854181384ULL,
6918657169554441728ULL,
2307076687298822272ULL,
1162201008177745ULL,
288241444551020608ULL,
1126040098054145ULL,
76561284128180224ULL,
4620693252076519941ULL,
600539574386696ULL,
14637282927583233ULL,
4617878485397014689ULL,
2392567377297664ULL,
9232959789021077504ULL,
7246573277601140768ULL,
310915569082839040ULL,
4756375168888112128ULL,
6759797525385728ULL,
1130298024722944ULL,
844493666402322ULL,
4625197096490782754ULL,
649081571206303776ULL,
1135250052878592ULL,
2324178602578413584ULL,
2487271344219104308ULL,
583050770514048ULL,
4505800798634112ULL,
380589370333266176ULL,
149535729455104ULL,
290772455629587712ULL,
2317173484054127108ULL,
14412645843645513728ULL,
4611916924996587554ULL,
577023848302548996ULL,
9259684512206423040ULL,
36345593957716096ULL,
18157352217806920ULL,
289782920963424320ULL,
144415535147401728ULL,
2612651902462328836ULL,
1152961091723608576ULL,
422354333730944ULL,
689543849984ULL,
4645711645900800ULL,
5260362772021051393ULL,
22553191107404353ULL,
74327004363177984ULL,
3513409146772987906ULL,
9308940713376942080ULL,
4652218451589865769ULL,
211107845277696ULL,
41099814439814912ULL,
2305843627723596032ULL,
5197440255661244933ULL,
1155179937326440833ULL
};

const int bishop_relevant_occupancy_count[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

const int rook_relevant_occupancy_count[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

U64 rook_lookup_attacks[64][4096];
U64 bishop_lookup_attacks[64][512];

// ************************************
// *      BITBOARDS & GAME STATE
// ************************************

// bitboards; index: PIECE enum
U64 bitboards[12] = {0};

// 0 - white; 1 - black
U64 color_occupancy_bitboards[2] = {0};

U64 both_occupancy_bitboard = 0ULL;

// castle system - each bit describes one possibility
// | white queenside | white kingside | black queenside | black kingside |
// |      bit 0/1    |     bit 0/1    |     bit 0/1     |    bit 0/1     |
int castles = 0;

// from enum COLOR -> white = 0, black = 1
int color_to_move = -1;

// from enum 0->63 square | -1 none
int en_passant_square = -1;

// halfmoves since last capture or pawn advance for fifty-move rule
int halfmove_counter = 0;

// Fullmove number: The number of the full moves. It starts at 1 and is incremented after Black's move.
int fullmove_number = 1;

// ************************************
// *            FUNCTIONS
// ************************************

inline void set_bit(U64 &bitboard, int square){
    bitboard |= (1ULL << square);
}

inline void pop_bit(U64 &bitboard){
    bitboard &= (bitboard-1);
}

void clear_bitboards(){
    for(U64 &b : bitboards)
        b = 0ULL;
}

void load_fen(std::string fen){
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
    for(char &c : fen){
        if(c == ' '){
            fragment_index++;
        }
        else{
            fen_fragments[fragment_index] += c;
        }
    }

    // *** 1.st fragment ***
    // split 1. fragment into ranks
    // !!! ORDER REVERSED !!!
    // fen construction: rank8/rank7/../rank2/rank1 from black to white
    // !position_fragments are from rank1 to rank8

    std::vector<std::string> position_fragments(8, "");
    fragment_index = 7;
    for(char &c : fen_fragments[0]){
        if(c == '/'){
            fragment_index--;
        }
        else{
            position_fragments[fragment_index] += c;
        }
    }


    // clear bitboards
    clear_bitboards();

    // set bitboards with pieces
    for(int rank = 0; rank < 8; rank++){
        for(int file = 0; file < 8; file++){
            char c = position_fragments[rank][file];
            // if digit <1;8> skip that number of squares
            // file is incremented so we need to add only X-1 squares (files)
            if('0' < c && c < '9') 
                file+=(c-'0')-1;
            else
                set_bit(bitboards[piece_ascii_to_number[c]], rank*8+file);
        }
    }

    // *** 2.st fragment ***
    // set other game state variables
    if(fen_fragments[1] == "w")
        color_to_move = static_cast<int>(COLOR::white);
    else if(fen_fragments[1] == "b")
        color_to_move = static_cast<int>(COLOR::black);
    else{
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
    if(fen_fragments[3] == "-")
        en_passant_square = -1;
    else{
        if(fen_fragments[3].length() > 2){
            // todo rise exception
            printf("Error: fen en passant square not '-' and length > 2");
            exit(-1);
        }
    
        en_passant_square = str_square_to_index( fen_fragments[3] );
    }
    
    // *** 5.st fragment ***
    // set halfmove counter
    halfmove_counter = std::stoi(fen_fragments[4]);
    // *** 6.st fragment ***
    // set fullmove number
    fullmove_number = std::stoi(fen_fragments[5]);
}

// ------------------------------------------

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

void generate_magic_numbers(bool rook){
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
                    int mask_bit = std::countr_zero(occupation_mask);
                    
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
                both_occupancy_bitboard = relevant_occupancy;
                // oparates on both_occupancies
                U64 attacks = rook ? calculate_rook_attacks(square) : calculate_bishop_attacks(square);
                
                // reset both_occupancy_bitboard
                both_occupancy_bitboard = 0ULL;

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
                // todo
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

void init_attacks_lookup_tables(bool rook){
    for(int square = 0; square < 64; square++){
        for(int variation = 0; (variation < (rook ? 4096 : 512)); variation++){
            U64 relevant_occupancy = 0ULL;
            int index = 0;

            U64 occupation_mask = rook ? 
            rook_relevant_occupancy(square) : 
            bishop_relevant_occupancy(square);

            while(occupation_mask){
                int mask_bit = std::countr_zero(occupation_mask);
                
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
            both_occupancy_bitboard = relevant_occupancy;
            // oparates on both_occupancies
            U64 attacks = rook ? calculate_rook_attacks(square) : calculate_bishop_attacks(square);

            if(rook)
                rook_lookup_attacks[square][magic_index] = attacks;
            else
                bishop_lookup_attacks[square][magic_index] = attacks;
        }   
    }
}

//todo: functions (for example init lookup tables) must clear state (both_occupancies)

// ************************************
// *         ATTACK GENERATION
// ************************************

U64 pawn_attaks(int square, int color){
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

U64 rook_attacks(int square){
    U64 magic_number = rook_magic_numbers[square];
    U64 relevant_occupancy = rook_relevant_occupancy(square) & both_occupancy_bitboard;
    int magic_index = relevant_occupancy * magic_number >> (64-rook_relevant_occupancy_count[square]);

    return rook_lookup_attacks[square][magic_index];
}

U64 bishop_attacks(int square){
    U64 magic_number = bishop_magic_numbers[square];
    U64 relevant_occupancy = bishop_relevant_occupancy(square) & both_occupancy_bitboard;
    int magic_index = relevant_occupancy * magic_number >> (64-bishop_relevant_occupancy_count[square]);

    return bishop_lookup_attacks[square][magic_index];
}

U64 calculate_bishop_attacks(int square){
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
        if(cursor & both_occupancy_bitboard)
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
        if(cursor & both_occupancy_bitboard)
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
        if(cursor & both_occupancy_bitboard)
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
        if(cursor & both_occupancy_bitboard)
            break;
    }
    
    return attacks;
}

U64 calculate_rook_attacks(int square){
    U64 attacks = 0ULL;
    // * up
    U64 cursor = 1ULL << square;
    for(int sq = square; (sq / 8 < 7); sq+=8){
        // move cursor
        cursor <<= 8;

        // save relevant square
        attacks |= cursor;

        // blocker
        if(cursor & both_occupancy_bitboard)
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
        if(cursor & both_occupancy_bitboard)
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
        if(cursor & both_occupancy_bitboard)
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
        if(cursor & both_occupancy_bitboard)
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


// ************************************
// *          VISUALISATION
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

void print_game_state(){

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
        (castles & 8) ? '1' : '0'
    );
    printf("\tstr: %c%c%c%c\n", 
        (castles & 1) ? 'Q' : '-',
        (castles & 2) ? 'K' : '-',
        (castles & 4) ? 'q' : '-',
        (castles & 8) ? 'k' : '-'
    );
    printf("- En passant square: \n");
    printf("\tint: %d\n", en_passant_square);
    std::cout << "\tstr: " << ((en_passant_square < 0) ? "-" : square_str[en_passant_square]) << std::endl;
    printf("- Halfmove counter: %d\n", halfmove_counter);
    printf("- Fullmove number: %d\n", fullmove_number);

    printf("---------------------------\n");
}

// ************************************
// *             MAIN
// ************************************

int main(int argc, char const *argv[])
{
    U64 board = 0ULL;
    
    // generate_magic_numbers(false);

    init_attacks_lookup_tables(true);
    init_attacks_lookup_tables(false);
    
    both_occupancy_bitboard = 0ULL;
    set_bit(both_occupancy_bitboard, (int)SQUARE::d3);
    set_bit(both_occupancy_bitboard, (int)SQUARE::a8);
    set_bit(both_occupancy_bitboard, (int)SQUARE::f5);
    set_bit(both_occupancy_bitboard, (int)SQUARE::g2);

    U64 ppp = bishop_attacks( (int)SQUARE::e4 );
    print_bitboard_bits(ppp);

    return 0;
}
