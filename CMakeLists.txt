cmake_minimum_required(VERSION 2.8)
project(xpeccy)

find_package(Qt4 REQUIRED)
find_package(ZLIB)

find_package(SDL)

if (UNIX)
	find_package(ALSA)
	if (ALSA_FOUND)
		add_definitions(-DHAVEALSASOUND)
	endif(ALSA_FOUND)
	set(INC_PATHS local/include local/include/z80ex include/z80ex include)
	set(LIB_PATHS lib lib64 local/lib local/lib64)
elseif(WIN32)
	set(INC_PATHS C:\\WINDOWS\\system32)
	set(LIB_PATHS C:\\WINDOWS\\system32)
endif()

find_path(Z80EX_INCLUDE_PATH NAMES z80ex.h PATH_SUFFIXES ${INC_PATHS} DOC "z80ex include directory")
find_library(Z80EX_LIBRARY NAMES z80ex PATH_SUFFIXES ${LIB_PATHS} DOC "z80ex library")

find_path(Z80EX_DASM_INCLUDE_PATH NAMES z80ex_dasm.h PATH_SUFFIXES local/include local/include/z80ex include/z80ex include DOC "z80ex include directory")
find_library(Z80EX_DASM_LIBRARY NAMES z80ex_dasm PATH_SUFFIXES lib lib64 local/lib local/lib64 DOC "z80ex_dasm library")

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Z80EX DEFAULT_MSG Z80EX_LIBRARY Z80EX_INCLUDE_PATH)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Z80EX_DASM DEFAULT_MSG Z80EX_DASM_LIBRARY Z80EX_DASM_INCLUDE_PATH)

if (Z80EX_FOUND AND Z80EX_DASM_FOUND)
	set(Z80EX_LIBRARIES ${Z80EX_LIBRARY} ${Z80EX_DASM_LIBRARY})
	set(Z80EX_INCLUDE_DIR ${Z80EX_INCLUDE_PATH} ${Z80EX_DASM_INCLUDE_PATH})
else ()
	message(FATAL ERROR "Could not find z80ex and/or z80ex_dasm")
endif()

file(GLOB SOURCES
	./src/*.cpp
	./src/libxpeccy/*.cpp
	./src/libxpeccy/*.c
	./src/filetypes/*.cpp
	./src/xcore/*.cpp
	./src/xgui/*.cpp
	./src/lib765/*.c
	./src/libdsk/*.c
)
file(GLOB HEADERS
	./src/*.h
	./src/libxpeccy/*.h
	./src/filetypes/*.h
	./src/xcore/*.h
	./src/xgui/*.h
	./src/lib765/*.h
	./src/libdsk/*.h
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

add_definitions(-Wall -O2)

set(CMAKE_C_FLAGS -std=gnu99)
set(CMAKE_C_FLAGS_DEBUG -DISDEBUG)

if (SDL_FOUND)
	add_definitions(-DHAVESDLSOUND)
	if (QTDRAWING OR WIN32)
		add_definitions(-DXQTPAINT)
	endif ()
else(SDL_FOUND)
	add_definitions(-DXQTPAINT)
endif(SDL_FOUND)

if (ZLIB_FOUND)
	add_definitions(-DHAVEZLIB)
endif(ZLIB_FOUND)

qt4_add_resources(RESOURCES ${QRCFILES})
qt4_wrap_ui(UIHEADERS ${UIFILES})
qt4_wrap_cpp(MOCHEADERS ${MOCFILES})

include(${QT_USE_FILE})

if (WIN32)

include_directories(
	${SDL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIRS}
	${CMAKE_BINARY_DIR}
	${Z80EX_INCLUDE_DIR}
)

add_executable(xpeccy WIN32
	${SOURCES}
	${HEADERS}
	${UIHEADERS}
	${RESOURCES}
	${MOCHEADERS}
)

target_link_libraries(xpeccy
	${QT_LIBRARIES}
	${SDL_LIBRARY}
	${ZLIB_LIBRARY}
	${Z80EX_LIBRARIES}
	winmm.dll
)

elseif(UNIX)

include_directories(
	${SDL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIRS}
	${ALSA_INCLUDE_DIR}
	${CMAKE_BINARY_DIR}
	${Z80EX_INCLUDE_DIR}
)

add_executable(xpeccy
	${SOURCES}
	${HEADERS}
	${UIHEADERS}
	${RESOURCES}
	${MOCHEADERS}
)

target_link_libraries(xpeccy
	${QT_QTCORE_LIBRARY}
	${QT_QTGUI_LIBRARY}
	${SDL_LIBRARY}
	${ALSA_LIBRARY}
	${ZLIB_LIBRARY}
	${Z80EX_LIBRARIES}
)

install_targets(/bin xpeccy)

endif()
