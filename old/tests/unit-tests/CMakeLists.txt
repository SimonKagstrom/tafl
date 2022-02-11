cmake_minimum_required (VERSION 2.6)

project (tafl-ut)
set (CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH}
                        ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
find_package (LibCRPCUT REQUIRED)

# ====================================
# project name and version
# ====================================
project (unit-test)
set (TGT ut)

set (${TGT}_SRCS
	../../src/utils.cc
    main.cc
    tests-ai.cc
    tests-board.cc
    )
set (CMAKE_BUILD_TYPE debug)

set (CMAKE_CXX_FLAGS "-std=c++0x -Wall -D_GLIBCXX_USE_NANOSLEEP")


include_directories(
	../../src/include/
	${GMOCK_INCLUDE_DIRS}
	/usr/src/gmock/include
	/usr/src/gmock/gtest/include
	${LIBCRPCUT_INCLUDE_DIRS}
	)

link_directories (
	/home/ska/local/lib
	/usr/src/gmock/gtest/lib/.libs
  )

add_executable (${TGT} ${${TGT}_SRCS})

target_link_libraries(${TGT}
	${LIBCRPCUT_LIBRARIES}
	${GMOCK_BOTH_LIBRARIES}
	gtest
	dl
)
