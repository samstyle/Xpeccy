cmake_minimum_required(VERSION 2.8)
project(libxpeccy)

if (UNIX)
	set(INC_PATHS local/include local/include/z80ex include/z80ex include)
	set(LIB_PATHS lib lib64 local/lib local/lib64)
elseif(WIN32)
	set(INC_PATHS C:\\WINDOWS\\system32)
	set(LIB_PATHS C:\\WINDOWS\\system32)
endif()

find_path(Z80EX_INCLUDE_PATH NAMES z80ex.h PATH_SUFFIXES ${INC_PATHS} DOC "z80ex include directory")
find_library(Z80EX_LIBRARY NAMES z80ex PATH_SUFFIXES ${LIB_PATHS} DOC "z80ex library")

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Z80EX DEFAULT_MSG Z80EX_LIBRARY Z80EX_INCLUDE_PATH)

if (Z80EX_FOUND)
	set(Z80EX_LIBRARIES ${Z80EX_LIBRARY})
	set(Z80EX_INCLUDE_DIR ${Z80EX_INCLUDE_PATH})
else ()
	message(FATAL ERROR "...Could not find z80ex...")
endif()

file(GLOB SOURCES ./*.c)
file(GLOB HEADERS ./*.h)

add_definitions(-Wall -O2 -std=gnu99)

include_directories(
	${CMAKE_BINARY_DIR}
	${Z80EX_INCLUDE_DIR}
)

add_library(xpeccy SHARED
	${SOURCES}
	${HEADERS}
)

target_link_libraries(xpeccy
	${Z80EX_LIBRARIES}
)
