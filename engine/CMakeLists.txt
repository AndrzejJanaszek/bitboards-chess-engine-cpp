cmake_minimum_required(VERSION 3.24)
project(ChessEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(engine
    src/board.cpp
    src/moves.cpp
    src/perft.cpp
    src/attacks.cpp
    src/constants.cpp
    src/visualisation.cpp
)

target_include_directories(engine PUBLIC include)

add_executable(engine_test src/main.cpp)
target_link_libraries(engine_test PRIVATE engine)
