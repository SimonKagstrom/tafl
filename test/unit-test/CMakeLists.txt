enable_testing()

find_package(doctest REQUIRED)

add_executable(ut
    main.cpp
    test_Board.cpp
    test_MoveCalculation.cpp
    test_MoveTrait.cpp
    test_Piece.cpp
    test_Pos.cpp
)

target_link_libraries(ut
PRIVATE
    tafl_release
    trompeloeil::trompeloeil
    doctest::doctest
)
