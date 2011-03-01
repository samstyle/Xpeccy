cmake_minimum_required(VERSION 2.6)
project(xpeccy)
find_package(Qt4 REQUIRED)
find_package(SDL REQUIRED)
find_package(ALSA REQUIRED)
find_package(ZLIB REQUIRED)

file(GLOB SOURCES ./src/*.cpp)
file(GLOB HEADERS ./src/*.h)
file(GLOB UIFILES ./ui/*.ui)
set(QRCFILES ./xpeccy.qrc)
set(MOCFILES
	./src/emulwin.h
	./src/develwin.h
	./src/debuger.h
	./src/setupwin.h
)

add_definitions(-O3 -Wall)
qt4_add_resources(RESOURCES ${QRCFILES})
qt4_wrap_ui(UIHEADERS ${UIFILES})
qt4_wrap_cpp(MOCHEADERS ${MOCFILES})

include(${QT_USE_FILE})

include_directories(
	${SDL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIRS}
	${ALSA_INCLUDE_DIR}
	${CMAKE_BINARY_DIR}
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
)