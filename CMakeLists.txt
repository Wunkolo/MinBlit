cmake_minimum_required( VERSION 3.2.2 )
project( MinBlit )

### Standard
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

### Verbosity
set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )

### Optimizations
if( MSVC )
	add_compile_options( /arch:AVX2 )
	add_compile_options( /W3 )
elseif( CMAKE_COMPILER_IS_GNUCXX )
	add_compile_options( -m64 )
	add_compile_options( -march=native )
	add_compile_options( -Ofast )
	add_compile_options( -Wall )
	add_compile_options( -Wextra )
endif()

### Tests
enable_testing()
# Benchmark
add_executable(
	Draw
	tests/draw.cpp
)
target_include_directories(
	Draw
	PRIVATE
	include
)

target_compile_definitions(
	Draw
	PRIVATE
)

add_test(
	NAME Draw
	COMMAND Draw
)
