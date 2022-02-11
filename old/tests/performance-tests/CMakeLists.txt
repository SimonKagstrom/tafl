cmake_minimum_required (VERSION 2.6)

# ====================================
# project name and version
# ====================================
project (tafl-performance-tests)
set (TGT performance-tests)

set (CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH}
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE  Release)
endif(NOT CMAKE_BUILD_TYPE)

set (${TGT}_SRCS
	../../src/ai.cc
	../../src/board.cc
	../../src/utils.cc
	main.cc
    )


set (CMAKE_CXX_FLAGS "-std=c++0x -Wall -D_GLIBCXX_USE_NANOSLEEP")

include_directories(
	../../src/include/
	)

add_executable (${TGT} ${${TGT}_SRCS})
