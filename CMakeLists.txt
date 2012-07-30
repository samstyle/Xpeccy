cmake_minimum_required(VERSION 2.8)
project(xpeccy)

set(INCLUDIRS ${CMAKE_BINARY_DIR})

if(UNIX)
	set(INC_PATHS local/include local/include/z80ex include/z80ex include)
	set(LIB_PATHS lib lib64 local/lib local/lib64)
elseif(WIN32)
	set(INC_PATHS ${INCLUDE} ${PATH})
	set(LIB_PATHS ${LIBPATH} ${PATH})
endif()

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)

# z80ex section

if("${Z80EX_BUILD}" STREQUAL "internal")
	set (API_REVISION 1)
	set (VERSION_MAJOR 1)
	set (VERSION_MINOR 19)
	set (RELEASE_TYPE "")
	set (VERSION_STR "${API_REVISION}.${VERSION_MAJOR}.${VERSION_MINOR}${RELEASE_TYPE}")
	if (BIG_ENDIAN)
		set (ENDIANNESS WORDS_BIG_ENDIAN)
	else ()
		set (ENDIANNESS WORDS_LITTLE_ENDIAN)
	endif ()
	add_definitions (-D${ENDIANNESS}
		-DZ80EX_VERSION_STR=${VERSION_STR}
		-DZ80EX_API_REVISION=${API_REVISION}
		-DZ80EX_VERSION_MAJOR=${VERSION_MAJOR}
		-DZ80EX_VERSION_MINOR=${VERSION_MINOR}
		-DZ80EX_RELEASE_TYPE=${RELEASE_TYPE})
	set(INCLUDIRS ${INCLUDIRS} ./src/libz80ex/include)
	set(Z80EX_INTERNAL 1)
else()
	# find z80ex
	find_path(Z80EX_INCLUDE_PATH NAMES z80ex.h PATH_SUFFIXES ${INC_PATHS} DOC "z80ex include directory")
	find_library(Z80EX_LIBRARY NAMES z80ex PATH_SUFFIXES ${LIB_PATHS} DOC "z80ex library")
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(Z80EX DEFAULT_MSG Z80EX_LIBRARY Z80EX_INCLUDE_PATH)
	if (!Z80EX_FOUND)
		message(FATAL ERROR "...Could not find z80ex")
	endif()
	#find z80ex_dasm
	find_path(Z80EX_DASM_INCLUDE_PATH NAMES z80ex_dasm.h PATH_SUFFIXES ${INC_PATHS} DOC "z80ex include directory")
	find_library(Z80EX_DASM_LIBRARY NAMES z80ex_dasm PATH_SUFFIXES ${LIB_PATHS} DOC "z80ex_dasm library")
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(Z80EX_DASM DEFAULT_MSG Z80EX_DASM_LIBRARY Z80EX_DASM_INCLUDE_PATH)
	if (!Z80EX_DASM_FOUND)
		message(FATAL ERROR "...Could not find z80ex_dasm")
	endif()
	set(LIBRARIES ${Z80EX_LIBRARY} ${Z80EX_DASM_LIBRARY})
	set(INCLUDIRS ${INCLUDIRS} ${Z80EX_INCLUDE_PATH} ${Z80EX_DASM_INCLUDE_PATH})
endif()

file(GLOB SOURCES
	./src/*.cpp
	./src/libxpeccy/*.c
	./src/filetypes/*.cpp
	./src/xcore/*.cpp
	./src/xgui/*.cpp
if(WIN32)
	./xpeccy.rc
endif(WIN32)
)
file(GLOB UIFILES ./ui/*.ui)
set(QRCFILES ./xpeccy.qrc)
set(MOCFILES
	./src/emulwin.h
	./src/develwin.h
	./src/debuger.h
	./src/setupwin.h
	./src/xgui/xgui.h
)

if (Z80EX_INTERNAL)
	set(SOURCES ${SOURCES} ./src/libz80ex/z80ex.c ./src/libz80ex/z80ex_dasm.c)
endif()

set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O2")
set(CMAKE_CXX_FLAGS "-Wall -O2")
set(CMAKE_C_FLAGS_DEBUG "-g")

find_package(Qt4 REQUIRED QtCore QtGui)

if(UNIX)
	find_package(ALSA)
	if (ALSA_FOUND)
		add_definitions(-DHAVEALSA)
		set(INCLUDIRS ${INCLUDIRS} ${ALSA_INCLUDE_DIR})
		set(LIBRARIES ${LIBRARIES} ${ALSA_LIBRARY})
	endif(ALSA_FOUND)
endif(UNIX)

find_package(SDL)
if(SDL_FOUND)
	add_definitions(-DHAVESDL)
	set(INCLUDIRS ${INCLUDIRS} ${SDL_INCLUDE_DIR})
	set(LIBRARIES ${LIBRARIES} ${SDL_LIBRARY})
	if (QTDRAWING OR WIN32)
		add_definitions(-DXQTPAINT)
	endif ()
else(SDL_FOUND)
	add_definitions(-DXQTPAINT)
endif(SDL_FOUND)

find_package(ZLIB)
if(ZLIB_FOUND)
	add_definitions(-DHAVEZLIB)
	set(INCLUDIRS ${INCLUDIRS} ${ZLIB_INCLUDE_DIR})
	set(LIBRARIES ${LIBRARIES} ${ZLIB_LIBRARY})
endif(ZLIB_FOUND)

if(WIN32)
	set(LIBRARIES ${LIBRARIES} winmm.dll)
endif(WIN32)

qt4_add_resources(RESOURCES ${QRCFILES})
qt4_wrap_ui(UIHEADERS ${UIFILES})
qt4_wrap_cpp(MOCHEADERS ${MOCFILES})

include(${QT_USE_FILE})
set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})

include_directories(${INCLUDIRS})

add_executable(xpeccy WIN32
	${SOURCES}
	${UIHEADERS}
	${RESOURCES}
	${MOCHEADERS}
)

target_link_libraries(xpeccy ${LIBRARIES})

if(UNIX)
	install_targets(/bin xpeccy)
endif(UNIX)

# result messages

message(STATUS "\n")
message(STATUS "Project will be compiled with following options:")
if(QTDRAWING OR WIN32)
	message(STATUS "Painter : Qt")
else()
	message(STATUS "Painter : SDL")
endif()
if(Z80EX_INTERNAL)
	message(STATUS "libz80ex : internal")
else()
	message(STATUS "libz80ex : external")
endif()
message(STATUS "\n")
