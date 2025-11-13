cmake_minimum_required(VERSION 3.24)
project(BitboardsChessEngine LANGUAGES CXX)

# Ustawienia globalne
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dodaj podprojekty
add_subdirectory(engine)
add_subdirectory(gui)
add_subdirectory(chess_bot)