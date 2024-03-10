set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

enable_testing()

add_compile_options(-fsanitize=address,undefined)
add_link_options(-fsanitize=address,undefined)


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
    test_MoveCalculation.cpp
    test_MoveTrait.cpp
    test_Piece.cpp
    test_Pos.cpp
)

#target_link_libraries(ut
#    PRIVATE nlohmann_json
#)

set_property(TARGET ut PROPERTY CXX_STANDARD 20)
