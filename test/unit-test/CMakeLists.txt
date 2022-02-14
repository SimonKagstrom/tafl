set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

enable_testing()

include_directories(
    ../../include/
    Catch2/single_include
    trompeloeil/include
)

add_executable(ut
    main.cpp
    ../../src/Board.cpp
    ../../src/Piece.cpp
    ../../src/MoveTrait.cpp
    test_Board.cpp
    test_MoveTrait.cpp
    test_Piece.cpp
    test_Pos.cpp
)
set_property(TARGET ut PROPERTY CXX_STANDARD 20)
