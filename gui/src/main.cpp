#include <SFML/Graphics.hpp>
#include <optional>

#include <iostream>
#include <map>
#include <array>

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
        }

        window.clear();

        // rysowanie szachownicy - background
        for (int square = 0; square < 64; square++){
            int row = square / 8;  // dzieląc przez 8 dostajemy numer wiersza
            int col = square % 8;  // reszta z dzielenia daje kolumnę

            sf::RectangleShape squareShape({squareSize, squareSize});
            squareShape.setPosition(sf::Vector2f(col * squareSize, row * squareSize));

            bool isDark = (row + col) % 2 != 0;
            squareShape.setFillColor(isDark ? darkSquareColor : lightSquareColor);

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

        window.display();
    }

    return 0;
}