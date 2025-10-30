#include <iostream>
#include <random>

#include "attacks.h"
#include "constants.h"
#include "attacks.h"
#include "utility.h"

using U64 = uint64_t;

U64 pawn_lookup_attacks[2][64];

U64 knight_lookup_attacks[64];
U64 king_lookup_attacks[64];

U64 rook_lookup_attacks[64][4096];
U64 bishop_lookup_attacks[64][512];

U64 calculate_bishop_attacks(int square, Board &game_state){
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

U64 calculate_rook_attacks(int square, Board &game_state){
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

U64 rook_attacks(int square, Board &game_state){
    U64 magic_number = rook_magic_numbers[square];
    U64 relevant_occupancy = rook_relevant_occupancy(square) & game_state.both_occupancy_bitboard;
    int magic_index = relevant_occupancy * magic_number >> (64-rook_relevant_occupancy_count[square]);

    return rook_lookup_attacks[square][magic_index];
}

U64 bishop_attacks(int square, Board &game_state){
    U64 magic_number = bishop_magic_numbers[square];
    U64 relevant_occupancy = bishop_relevant_occupancy(square) & game_state.both_occupancy_bitboard;
    int magic_index = relevant_occupancy * magic_number >> (64-bishop_relevant_occupancy_count[square]);

    return bishop_lookup_attacks[square][magic_index];
}

U64 queen_attacks(int square, Board &game_state){
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

void generate_magic_numbers(bool rook, Board &game_state){
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

void init_rook_bishop_lookup_tables(bool rook, Board &game_state){
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

void init_all_lookup_tables(Board &game_state){
    init_rook_bishop_lookup_tables(true, game_state);
    init_rook_bishop_lookup_tables(false, game_state);
    init_pawn_lookup_table();
    init_king_lookup_table();
    init_knight_lookup_table();
}
