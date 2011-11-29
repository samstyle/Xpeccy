cmake_minimum_required(VERSION 2.8)
project(xpeccy)

find_package(Qt4 REQUIRED)
find_package(SDL REQUIRED)
find_package(ALSA)
find_package(ZLIB)

file(GLOB SOURCES ./src/*.cpp ./src/filetypes/*.cpp)
file(GLOB HEADERS ./src/*.h ./src/filetypes/*.h)
file(GLOB UIFILES ./ui/*.ui)
set(QRCFILES ./xpeccy.qrc)
set(MOCFILES
	./src/emulwin.h
	./src/develwin.h
	./src/debuger.h
	./src/setupwin.h
)

add_definitions(-Wall -O3)

if (SDL_FOUND)
	add_definitions(-DHAVESDLSOUND)
endif()

if (ALSA_FOUND)
	add_definitions(-DHAVEALSASOUND)
endif()

if (ZLIB_FOUND)
	add_definitions(-DHAVEZLIB)
endif()

qt4_add_resources(RESOURCES ${QRCFILES})
qt4_wrap_ui(UIHEADERS ${UIFILES})
qt4_wrap_cpp(MOCHEADERS ${MOCFILES})

include(${QT_USE_FILE})

include_directories(
	${SDL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIRS}
	${ALSA_INCLUDE_DIR}
	${CMAKE_BINARY_DIR}
	/usr/local/include/z80ex
	/usr/include/z80ex
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
	z80ex
	z80ex_dasm
)

install_targets(/bin xpeccy)