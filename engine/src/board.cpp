#include <string>

#include "board.hpp"
#include "constants.hpp"
#include "utility.hpp"
#include "visualisation.hpp"


Board& Board::operator=(const Board &other)
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

void Board::clear_bitboards()
    {
        for (U64 &b : bitboards)
            b = 0ULL;

        color_occupancy_bitboards[0] = 0ULL;
        color_occupancy_bitboards[1] = 0ULL;
        both_occupancy_bitboard = 0ULL;
    }

void Board::load_fen(std::string fen)
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

void Board::print_game_state()
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

void Board::print_board_unicode()
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

void Board::print_board_ascii(Board &game_state){
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

std::array<char, 64> Board::board_to_char_array()
{
    std::array<char, 64> arr{};
    arr.fill('-'); // wypełniamy domyślnie pustym polem

    // iterujemy po wszystkich typach figur (12 bitboardów)
    for (int bitboard_index = 0; bitboard_index < 12; bitboard_index++)
    {
        U64 piece_bitboard = bitboards[bitboard_index];

        while (piece_bitboard)
        {
            int square_number = get_LS1B(piece_bitboard);

            // przypisujemy odpowiedni znak ASCII do pola
            arr[square_number] = ascii_pieces[bitboard_index];

            // usuwamy LS1B
            piece_bitboard &= (piece_bitboard - 1);
        }
    }

    return arr;
}