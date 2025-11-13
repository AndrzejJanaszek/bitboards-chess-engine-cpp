cmake_minimum_required(VERSION 3.24)
project(GUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Statyczne linkowanie SFML
add_definitions(-DSFML_STATIC)

set(GUI_SOURCES
    src/main.cpp
)

add_executable(gui ${GUI_SOURCES})

# ${CMAKE_SOURCE_DIR} to główny katalog projektu
set(SFML_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/SFML-3.0.2/include")
set(SFML_LIB_DIR "${CMAKE_SOURCE_DIR}/SFML-3.0.2/build/lib")

target_include_directories(gui PRIVATE ${SFML_INCLUDE_DIR})

target_link_libraries(gui PRIVATE
    "${SFML_LIB_DIR}/libsfml-graphics-s.a"
    "${SFML_LIB_DIR}/libsfml-window-s.a"
    "${SFML_LIB_DIR}/libsfml-system-s.a"
    gdi32
    winmm
    opengl32
    user32
    ole32
    shell32
    imm32
)


add_custom_command(TARGET gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/src/img"
    $<TARGET_FILE_DIR:gui>/img
)

target_link_libraries(gui PRIVATE engine chess_bot)