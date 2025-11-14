#include "moves.hpp"
#include "constants.hpp"
#include "utility.hpp"
#include "attacks.hpp"

void Move::encode_move(int from_square, int to_square, int piece, MoveType move_type){
    encoded_value = 0;
    encoded_value |= from_square;
    encoded_value |= (to_square << 6);
    encoded_value |= (piece << 12);
    encoded_value |= (static_cast<int>(move_type) << 16);
}

void Move::print() const{
    std::cout << "Move: " << square_str[this->get_from_square()] << square_str[this->get_to_square()] << " " 
    << ascii_pieces[this->get_piece()] << " " << move_type_str[this->get_move_type()] << "\n";
}

bool is_square_attacked_by(int square, int side, Board &game_state){
    // super-piece technic
    // set pieces on current <square> and intersect attacks with appropriate pieces
    // for pawn intersect enemy pawns => white intersect black and vice versa
    
    // pawns
    // set enemy piece on <square> and check does it attack friendly pawns
    if (pawn_lookup_attacks[!side][square] & game_state.bitboards[ static_cast<int>(PIECE::P) + (side * 6) ])
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
U64 get_attacked_squares(int side, Board &game_state){
    U64 result = 0ULL;

    for(int square = 0; square < 64; square++){
        result |= is_square_attacked_by(square, side, game_state) ? (1ULL << square) : 0ULL;
    }

    return result;
}

std::vector<Move> generate_moves(Board &game_state){
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
                        move.encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::en_passant_capture);
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
                            move.encode_move(from_square, to_square, static_cast<int>(PIECE::P) + (game_state.color_to_move*6), MoveType::capture);
                            moves.push_back(move);
                        }
                    }
                    
                    // remove LS1B
                    pop_bit(attacks);
                }

                //* single push
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
                                move.encode_move(from_square, static_cast<int>(SQUARE::c1), static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::queen_castle);
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
                                move.encode_move(from_square, static_cast<int>(SQUARE::g1), static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::king_castle);
                                moves.push_back(move);
                            }
                        }
                    }
                }
                // black side
                else{
                    // is QUEENside castling possible (game state)
                    if(game_state.castles & 0b0010){
                        // are squares between king and rook empty
                        if((game_state.both_occupancy_bitboard & black_queenside_empty_squares_castling_mask) == 0){
                            // king and square next to him is not under attack
                            // is NOT attacked (e8) && is NOT attacked (d8)
                            if(!is_square_attacked_by(static_cast<int>(SQUARE::e8), !game_state.color_to_move, game_state) && !is_square_attacked_by(static_cast<int>(SQUARE::d8), !game_state.color_to_move, game_state)){
                                // std::cout << "Black queenside castle: e8c8 O-O-O \n";
                                Move move;
                                move.encode_move(from_square, static_cast<int>(SQUARE::c8), static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::queen_castle);
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
                                move.encode_move(from_square, static_cast<int>(SQUARE::g8), static_cast<int>(PIECE::K) + (game_state.color_to_move*6), MoveType::king_castle);
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


void make_move(Move move, Board &board){
    // GAME STATE UPDATE AT THE END OF FUNCTION DEFINITION
    // color_to_move & color_occupancies & full- half move count
    //
    // enemy color occupancy update on CAPTURE MOVE block

    // reset en passant square
    // if double push flag will be set
    board.en_passant_square = -1;

    //* UPDATE CASTLE RIGHTS
    // if rook ON A or H rank update castle rights
    if(move.get_piece() == static_cast<int>(PIECE::R)){
        if( A_FILE_MASK & (1ULL << move.get_from_square()) ){
            // white queen
            board.castles &= 0b0111;
        }
        else if( H_FILE_MASK & (1ULL << move.get_from_square()) ){
            // white king
            board.castles &= 0b1011;
        }
    }
    else if(move.get_piece() == static_cast<int>(PIECE::r)){
        if( A_FILE_MASK & (1ULL << move.get_from_square()) ){
            // black queen
            board.castles &= 0b1101;
        }
        else if( H_FILE_MASK & (1ULL << move.get_from_square()) ){
            // black king
            board.castles &= 0b1110;
        }
    }

    // if king remove castle right for that color
    else if(move.get_piece() == static_cast<int>(PIECE::K)){
        // white
        board.castles &= 0b0011;
    }
    else if(move.get_piece() == static_cast<int>(PIECE::K)){
        // black
        board.castles &= 0b1100;
    }


    //* QUIET MOVE
    if(move.get_move_type() == static_cast<int>(MoveType::quiet_move)){
        // UPDATE BITBOARDS [PICES]
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        board.bitboards[move.get_piece()] |= 1ULL << move.get_to_square();
    }


    //* CAPTURE
    if(move.get_move_type() == static_cast<int>(MoveType::capture)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        
        // remove target square
        for(int i = static_cast<int>(PIECE::P); i < static_cast<int>(PIECE::p); i++){
            board.bitboards[i+((!board.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        board.color_occupancy_bitboards[!board.color_to_move] &= ~( 1ULL << move.get_to_square() );
        
        // set target square
        board.bitboards[move.get_piece()] |= 1ULL << move.get_to_square();

    }

    
    //* MoveType::double_pawn_push
    // double push
    // sett en passant flag
    if(move.get_move_type() == static_cast<int>(MoveType::double_pawn_push)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        board.bitboards[move.get_piece()] |= 1ULL << move.get_to_square();

        // set en passant square
        board.en_passant_square = (move.get_from_square() + move.get_to_square()) / 2;
    }


    //* MoveType::en_passant_capture
    if(move.get_move_type() == static_cast<int>(MoveType::en_passant_capture)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        board.bitboards[move.get_piece()] |= 1ULL << move.get_to_square();

        // remove enemy pawn
        // to target square add offset (for white -8 for black +8)
        const int enemy_pawn_square = move.get_to_square() + (board.color_to_move*2 - 1)*8;
        board.bitboards[static_cast<int>(PIECE::P) + ((!board.color_to_move)*6)] &= ~(1ULL << enemy_pawn_square);
        // OCCUPANCIES
        // remove target square
        board.color_occupancy_bitboards[!board.color_to_move] &= ~(1ULL << enemy_pawn_square);
    }

    //* PAWN PROMOTION
    // rook
    if(move.get_move_type() == static_cast<int>(MoveType::rook_promotion)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        board.bitboards[static_cast<int>(PIECE::R) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // bishop
    else if(move.get_move_type() == static_cast<int>(MoveType::bishop_promotion)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        board.bitboards[static_cast<int>(PIECE::B) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // knight
    else if(move.get_move_type() == static_cast<int>(MoveType::knight_promotion)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        board.bitboards[static_cast<int>(PIECE::N) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // queen
    else if(move.get_move_type() == static_cast<int>(MoveType::queen_promotion)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        board.bitboards[static_cast<int>(PIECE::Q) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();
    }

    //* MoveType::rook_promo_capture
    // rook
    if(move.get_move_type() == static_cast<int>(MoveType::rook_promo_capture)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        
        // remove target square
        for(int i = 0; i < 6; i++){
            board.bitboards[i+((!board.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        board.color_occupancy_bitboards[!board.color_to_move] &= ~( 1ULL << move.get_to_square() );

        // set target square
        board.bitboards[static_cast<int>(PIECE::R) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // bishop
    else if(move.get_move_type() == static_cast<int>(MoveType::bishop_promo_capture)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );

        // remove target square
        for(int i = 0; i < 6; i++){
            board.bitboards[i+((!board.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        board.color_occupancy_bitboards[!board.color_to_move] &= ~( 1ULL << move.get_to_square() );


        // set target square
        board.bitboards[static_cast<int>(PIECE::B) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // knight
    else if(move.get_move_type() == static_cast<int>(MoveType::knight_promo_capture)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );

        // remove target square
        for(int i = 0; i < 6; i++){
            board.bitboards[i+((!board.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        board.color_occupancy_bitboards[!board.color_to_move] &= ~( 1ULL << move.get_to_square() );

        // set target square
        board.bitboards[static_cast<int>(PIECE::N) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();
    }
    // queen
    else if(move.get_move_type() == static_cast<int>(MoveType::queen_promo_capture)){
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );

        // remove target square
        for(int i = 0; i < 6; i++){
            board.bitboards[i+((!board.color_to_move)*6)] &= ~( 1ULL << move.get_to_square() );
        }
        // OCCUPANCIES
        // remove target square
        board.color_occupancy_bitboards[!board.color_to_move] &= ~( 1ULL << move.get_to_square() );

        // set target square
        board.bitboards[static_cast<int>(PIECE::Q) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();
    }

    //* MoveType::king_castle
    if(move.get_move_type() == static_cast<int>(MoveType::king_castle)){
        // king
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        board.bitboards[static_cast<int>(PIECE::K) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();

        // rook
        const int rook_target_square = (move.get_from_square() + move.get_to_square())/2;
        const int rook_from_square = board.color_to_move ? static_cast<int>(SQUARE::h8) : static_cast<int>(SQUARE::h1);
        // remove rook
        board.bitboards[rook_from_square] &= ~(1ULL << move.get_from_square());
        //set rook
        board.bitboards[static_cast<int>(PIECE::R) + (board.color_to_move*6)] |= 1ULL << rook_target_square;

        // castle rights updated at the start of function
    }

    //* MoveType::queen_castle
    if(move.get_move_type() == static_cast<int>(MoveType::queen_castle)){
        // king
        // remove from square
        board.bitboards[move.get_piece()] &= ~( 1ULL << move.get_from_square() );
        // set target square
        board.bitboards[static_cast<int>(PIECE::K) + (board.color_to_move*6)] |= 1ULL << move.get_to_square();

        // rook
        const int rook_target_square = (move.get_from_square() + move.get_to_square())/2;
        const int rook_from_square = board.color_to_move ? static_cast<int>(SQUARE::a8) : static_cast<int>(SQUARE::a1);
        // remove rook
        board.bitboards[rook_from_square] &= ~(1ULL << move.get_from_square());
        //set rook
        board.bitboards[static_cast<int>(PIECE::R) + (board.color_to_move*6)] |= 1ULL << rook_target_square;

        // castle rights updated at the start of function
    }


    // update game state
    board.halfmove_counter += 1;
    board.fullmove_number = board.halfmove_counter/2;

    // UPDATE OCCUPANCIES [COLOR]
    // remove from square
    board.color_occupancy_bitboards[board.color_to_move] &= ~( 1ULL << move.get_from_square() );
    // add target square
    board.color_occupancy_bitboards[board.color_to_move] |= 1ULL << move.get_to_square();
    board.both_occupancy_bitboard = board.color_occupancy_bitboards[0] | board.color_occupancy_bitboards[1];

    // update color to move game state
    board.color_to_move = !board.color_to_move;

    // // check wheter king is in check
    // int king_square = get_LS1B(board.bitboards[static_cast<int>(PIECE::K) + (!board.color_to_move * 6)]);
    // bool isLegal = !is_square_attacked_by(king_square, board.color_to_move, board);


    // // return 1 if legal; 0 if not legal
    // return isLegal;
}

bool isMoveLegal(Move &move, Board &current_board){
    Board board_copy = current_board;

    // make move
    make_move(move, board_copy);

    // check wheter king is in check
    int king_square = get_LS1B(board_copy.bitboards[static_cast<int>(PIECE::K) + (!board_copy.color_to_move * 6)]);
    bool isLegal = !is_square_attacked_by(king_square, board_copy.color_to_move, board_copy);

    return isLegal;
}

std::vector<Move> generate_legal_moves(Board &game_state){
    std::vector<Move> pseudo_legal_moves = generate_moves(game_state);
    std::vector<Move> legal_moves;
    legal_moves.reserve(pseudo_legal_moves.size());

    for(Move &move : pseudo_legal_moves){
        bool isLegal = isMoveLegal(move, game_state);
        // printf("bool : %d\n", isLegal);
        if(isLegal){
            legal_moves.push_back(move);
        }
    }

    return legal_moves;
}

bool isKingUnderAttack(Board &board, bool other_side){
    // if other side change king color
    // default king color = color to move
    int king_color = other_side ? !board.color_to_move : board.color_to_move;

    int king_square = get_LS1B(board.bitboards[static_cast<int>(PIECE::K) + (king_color*6)]);

    if(king_square > 63){
        throw std::runtime_error("Squre > 64 propably no king");
    }

    return is_square_attacked_by(king_square, !king_color, board);
}

bool isCheckMate(Board &board){
    if(isKingUnderAttack(board) && generate_legal_moves(board).size() == 0){
        return true;
    }

    return false;
}