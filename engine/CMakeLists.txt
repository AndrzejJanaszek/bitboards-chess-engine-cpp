cmake_minimum_required(VERSION 3.24)
project(ChessEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatyczne wyszukiwanie wszystkich plików źródłowych i nagłówków
file(GLOB_RECURSE ENGINE_SOURCES src/*.cpp)
file(GLOB_RECURSE ENGINE_HEADERS include/*.hpp include/*.h)

# Usuń src/main.cpp, jeśli istnieje
list(REMOVE_ITEM ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Tworzymy bibliotekę silnika
add_library(engine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

target_include_directories(engine PUBLIC include)

# Plik wykonywalny do testów
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(engine_test src/main.cpp)
    target_link_libraries(engine_test PRIVATE engine)
endif()