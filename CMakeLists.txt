cmake_minimum_required (VERSION 3.21)
project (tafl)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)
find_package(trompeloeil REQUIRED)
find_package(doctest REQUIRED)

add_link_options(-fsanitize=address,undefined -g)

add_library(tafl_interface INTERFACE
)

target_include_directories(tafl_interface INTERFACE
    include
)


add_library(tafl EXCLUDE_FROM_ALL
    src/Board.cpp
    src/Piece.cpp
    src/MoveTrait.cpp
)

target_link_libraries(tafl
PUBLIC
    tafl_interface
    fmt::fmt
)

target_compile_options(tafl
PUBLIC
    -fsanitize=address,undefined
    -g
)


add_library(tafl_release EXCLUDE_FROM_ALL
    src/Board.cpp
    src/Piece.cpp
    src/MoveTrait.cpp
)

target_link_libraries(tafl_release
PUBLIC
    tafl_interface
    fmt::fmt
)

target_compile_options(tafl_release
PUBLIC
    -O3
    -g
)



add_subdirectory(src/auto-player)
add_subdirectory(test/unit-test)
