cmake_minimum_required(VERSION 3.24)
project(ChessBot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Wczytanie wszystkich plików źródłowych i nagłówków
file(GLOB_RECURSE CHESS_BOT_SOURCES src/*.cpp)
file(GLOB_RECURSE CHESS_BOT_HEADERS include/*.hpp include/*.h)

# Usuń src/main.cpp, jeśli istnieje
list(REMOVE_ITEM CHESS_BOT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Tworzymy bibliotekę statyczną
add_library(chess_bot STATIC
    ${CHESS_BOT_SOURCES}
    ${CHESS_BOT_HEADERS}
)

target_include_directories(chess_bot PUBLIC include)

# Bot będzie używał engine
target_link_libraries(chess_bot PUBLIC engine)

# Plik wykonywalny do testów (opcjonalny)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(chess_bot_test src/main.cpp)
    target_link_libraries(chess_bot_test PRIVATE chess_bot engine)
endif()
