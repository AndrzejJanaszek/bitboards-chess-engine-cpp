#include <SFML/Graphics.hpp>
#include <optional>

#include <iostream>
#include <map>
#include <array>

#include "board.hpp"
#include "attacks.hpp"
#include "utility.hpp"
#include "moves.hpp"

std::map<char, sf::Texture> loadPieceTextures(const std::string &img_dir_path) {
    std::map<char, sf::Texture> textures;

    std::map<char, std::string> filenames = {
        {'P', "white-pawn.png"},   {'R', "white-rook.png"},   {'N', "white-knight.png"},
        {'B', "white-bishop.png"}, {'Q', "white-queen.png"},  {'K', "white-king.png"},
        {'p', "black-pawn.png"},   {'r', "black-rook.png"},   {'n', "black-knight.png"},
        {'b', "black-bishop.png"}, {'q', "black-queen.png"},  {'k', "black-king.png"}
    };

    for (const auto& [key, filename] : filenames) {
        sf::Texture texture;
        if (!texture.loadFromFile(img_dir_path + filename)) {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            continue;
        }
        textures[key] = std::move(texture);
    }

    return textures;
}

int main()
{
    constexpr unsigned int windowSize = 1000;
    constexpr unsigned int boardSize = 8;
    const float squareSize = static_cast<float>(windowSize) / boardSize;

    const sf::Color lightSquareColor = sf::Color(240, 217, 181);  // jasny
    const sf::Color darkSquareColor  = sf::Color(181, 136, 99);   // ciemny
    const sf::Color activeSquareColor  = sf::Color(200, 40, 30);   // ciemny
    const sf::Color possibleSquareColor  = sf::Color(40, 200, 60);   // ciemny

    int active_square = -1;

    std::vector<int> possible_squares;
    std::vector<Move> possible_moves;

    Board board;
    init_all_lookup_tables(board);
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    auto legal_moves = generate_legal_moves(board);

    // std::array<char, 64> board_position
    // 'P','R','N','B','Q','K','p','r','n','b','q','k','-'

    std::array<char, 64> board_position = {
        'R','N','B','Q','K','B','N','R',  // 1. rząd (a1-h1)
        'P','P','P','P','P','P','P','P',  // 2. rząd (a2-h2)
        '-','-','-','-','-','-','-','-',  // 3. rząd
        '-','-','-','-','-','-','-','-',  // 4. rząd
        '-','-','-','-','-','-','-','-',  // 5. rząd
        '-','-','-','-','-','-','-','-',  // 6. rząd
        'p','p','p','p','p','p','p','p',  // 7. rząd (a7-h7)
        'r','n','b','q','k','b','n','r'   // 8. rząd (a8-h8)
    };

    
    // std::map<char, texture> pieces_textures
    // key = {'P','R','N','B','Q','K','p','r','n','b','q','k'}
    std::map<char, sf::Texture> pieces_textures = loadPieceTextures("img/");



    // Tworzenie okna (SFML 3)
    auto window = sf::RenderWindow(sf::VideoMode({windowSize, windowSize}), "Chessboard");
    window.setFramerateLimit(60);

    // Główna pętla programu
    while (window.isOpen())
    {
        // Obsługa zdarzeń (SFML 3 -> std::optional)
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mouseButtonPressed->button == sf::Mouse::Button::Left)
                {
                    std::cout << "the Left button was pressed" << std::endl;
                    std::cout << "mouse x: " << mouseButtonPressed->position.x << std::endl;
                    std::cout << "mouse y: " << mouseButtonPressed->position.y << std::endl;

                    int mx = mouseButtonPressed->position.x;
                    int my = mouseButtonPressed->position.y;

                    int rx = mx / squareSize;
                    int ry = 8 - my / squareSize;

                    int current_square = rx + 8*ry;

                    if(possible_moves.size() > 0){
                        for(Move& move : possible_moves){
                            if(move.get_from_square() == active_square && move.get_to_square() == current_square){
                                // make move
                                make_move(move, board);
                                legal_moves = generate_legal_moves(board);
                                board_position = board.board_to_char_array();

                                possible_moves.clear();
                                possible_squares.clear();
                                active_square = -1;
                                current_square = -1;
                                continue;
                            }
                        }
                    }

                    // set possible moves
                    possible_moves.clear();
                    possible_squares.clear();
                    for(Move& move : legal_moves){
                        if(move.get_from_square() == current_square){
                            possible_moves.push_back(move);
                            possible_squares.push_back(move.get_to_square());
                        }
                    }


                    // U64 attacks = queen_attacks(active_square, board);
                    
                    // possible_squares.clear();
                    // while(attacks){
                    //     possible_squares.push_back(get_LS1B(attacks));
                    //     pop_bit(attacks);
                    // }
                    
                    active_square = current_square;
                }
            }
        }

        window.clear();

        // rysowanie szachownicy - background
        for (int square = 0; square < 64; square++){
            int col = square % 8;
                int row = 7 - (square / 8);  // reszta z dzielenia daje kolumnę

            sf::RectangleShape squareShape({squareSize, squareSize});
            squareShape.setPosition(sf::Vector2f(col * squareSize, row * squareSize));

            bool isDark = (row + col) % 2 != 0;
            squareShape.setFillColor(isDark ? darkSquareColor : lightSquareColor);
            
            if(square == active_square){
                squareShape.setFillColor(activeSquareColor);
            }

            if( std::find( possible_squares.begin(), possible_squares.end(), square ) != possible_squares.end() ){
                squareShape.setFillColor(possibleSquareColor);
            }

            window.draw(squareShape);
        }

        // rysowanie figur
        for (int square = 0; square < 64; ++square) {
            char square_content = board_position.at(square);
            if (square_content != '-') {
                // Obliczamy kolumnę i wiersz z indeksu pola
                int col = square % 8;
                int row = 7 - (square / 8); // a1 = 0 w lewym dolnym rogu, więc rząd od dołu

                // Tworzymy sprite i przypisujemy teksturę
                sf::Sprite sprite(pieces_textures[square_content]);

                // Skaluje sprite do wymiaru pola
                sf::Vector2u texSize = pieces_textures[square_content].getSize();
                sprite.setScale(
                    sf::Vector2f(
                        squareSize / float(texSize.x),
                        squareSize / float(texSize.y)
                ));

                // Ustawiamy pozycję sprite na polu
                sprite.setPosition(sf::Vector2f(col * squareSize, row * squareSize));

                // Rysujemy sprite w oknie
                window.draw(sprite);
            }
        }

        if(isCheckMate(board)){
            sf::RectangleShape squareShape({windowSize, windowSize});

            squareShape.setFillColor(!board.color_to_move ? darkSquareColor : lightSquareColor);

            window.draw(squareShape);
        }

        window.display();
    }

    return 0;
}