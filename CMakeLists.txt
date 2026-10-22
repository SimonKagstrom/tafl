cmake_minimum_required (VERSION 3.28)
project (tafl LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
  etl
  GIT_REPOSITORY https://github.com/ETLCPP/etl.git
  GIT_TAG        20.47.1
  GIT_SHALLOW    TRUE
  SYSTEM
)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        11.2.0
  GIT_SHALLOW    TRUE
  SYSTEM
)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        v2.5.3
  GIT_SHALLOW    TRUE
  SYSTEM
)
FetchContent_Declare(
  trompeloeil
  GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
  GIT_TAG        v49
  GIT_SHALLOW    TRUE
  SYSTEM
)
FetchContent_MakeAvailable(etl fmt doctest trompeloeil)

#add_link_options(-fsanitize=address,undefined -g)

add_library(tafl_interface INTERFACE
)

target_include_directories(tafl_interface
INTERFACE
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
    etl::etl
)

target_compile_options(tafl
PUBLIC
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
    etl::etl
)

target_compile_options(tafl_release
PUBLIC
    -O3
    -g
)



add_subdirectory(src/auto-player)
add_subdirectory(src/text-player)
add_subdirectory(test/unit-test)
