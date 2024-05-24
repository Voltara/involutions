if(DEFINED ENV{CPPUTEST_HOME})
	message(STATUS "Using CppUTest found in $ENV{CPPUTEST_HOME}")
	set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
	set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
	set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
else()
	find_package(PkgConfig REQUIRED)
	pkg_search_module(CPPUTEST REQUIRED cpputest>=4.0)
	message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")
endif()

include_directories(${CPPUTEST_INCLUDE_DIRS} ../src/)
link_directories(${CPPUTEST_LIBRARIES})

add_executable(check
	RunAllTests.cpp
	CpermTest.cpp
	EpermTest.cpp
	CcoordTest.cpp
	EcoordTest.cpp
	CpruneTest.cpp
	EpruneTest.cpp
	CubeTest.cpp
	OrientTest.cpp
	SymTest.cpp
	MoveTest.cpp
	InvolutionTest.cpp
	CornerHashTest.cpp
	TrackerTest.cpp
)
target_link_libraries(check involutions ${CPPUTEST_LDFLAGS})
add_custom_command(TARGET check COMMAND cd .. && tests/check POST_BUILD)
