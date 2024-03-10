set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

enable_testing()

find_package(doctest REQUIRED)
find_package(fmt REQUIRED)
find_package(trompeloeil REQUIRED)

add_compile_options(-fsanitize=address,undefined -g)
add_link_options(-fsanitize=address,undefined -g)


include_directories(
    ../../include/
)

add_executable(ut
    main.cpp
    ../../src/Board.cpp
    ../../src/Piece.cpp
    ../../src/MoveTrait.cpp
    test_Board.cpp
    test_MoveCalculation.cpp
    test_MoveTrait.cpp
    test_Piece.cpp
    test_Pos.cpp
)

target_link_libraries(ut
PRIVATE
    trompeloeil::trompeloeil
    doctest::doctest
)

set_property(TARGET ut PROPERTY CXX_STANDARD 20)
