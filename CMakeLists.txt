cmake_minimum_required (VERSION 3.27)

project(involutions)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -std=c++20 -Wall -Wno-sign-compare")

set(CMAKE_C_COMPILER "clang" CACHE STRING "clang compiler" FORCE)
set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "clang++ compiler" FORCE)

add_library(involutions
	src/cube.cpp
	src/sym.cpp
	src/moveseq.cpp
	src/cperm16.cpp
	src/eperm48.cpp
	src/orient.cpp
	src/alloc.cpp
	src/cprune.cpp
	src/eprune.cpp
	src/involution.cpp
	src/ecsolver.cpp
	src/corner_hash.cpp
	src/tracker.cpp
	src/interrupt.cpp
	src/neighborsolver.cpp
	src/status.cpp
)

add_executable(invo
	src/main.cpp
)
target_link_libraries(invo involutions)

add_subdirectory(tests)
