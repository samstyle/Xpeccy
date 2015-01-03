cmake_minimum_required(VERSION 2.8)
project(xpeccy)
set(XVER "0.5.20150103")

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)

set(INCLUDIRS ${CMAKE_BINARY_DIR})

if(UNIX)
	set(INC_PATHS local/include local/include/z80ex include/z80ex include)
	set(LIB_PATHS lib lib64 local/lib local/lib64)
elseif(WIN32)
	set(INC_PATHS ${INCLUDE} ${PATH})
	set(LIB_PATHS ${LIBPATH} ${PATH})
endif()

file(GLOB SOURCES
	./src/*.cpp
	./src/libxpeccy/*.c
	./src/libxpeccy/hardware/*.c
	./src/filetypes/*.cpp
	./src/xcore/*.cpp
	./src/xgui/*.cpp
	./src/filetypes/unpackers/*.cpp
if(WIN32)
	./xpeccy.rc
endif(WIN32)
)
file(GLOB HEADERS
	./src/*.h
	./src/libxpeccy/*.h
	./src/libxpeccy/hardware/*.h
	./src/filetypes/*.h
	./src/xcore/*.h
	./src/xgui/*.h
	./src/filetypes/unpackers/*.h
)
file(GLOB UIFILES ./ui/*.ui)
set(QRCFILES ./xpeccy.qrc)
set(MOCFILES
	./src/emulwin.h
	./src/debuger.h
	./src/setupwin.h
	./src/xgui/xgui.h
)

add_definitions(-DVERSION=${XVER})

# endianess

include (TestBigEndian)
test_big_endian(BIG_ENDIAN)
if (BIG_ENDIAN)
	add_definitions(-DWORDS_BIG_ENDIAN)
else()
	add_definitions(-DWORDS_LITTLE_ENDIAN)
endif()

# z80ex section ( external | internal | * )

if("${Z80EX_BUILD}" STREQUAL "external")
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
	set(Z80EX_TYPE 1)
elseif("${Z80EX_BUILD}" STREQUAL "internal")
	set (API_REVISION 1)
	set (VERSION_MAJOR 1)
	set (VERSION_MINOR 19)
	set (RELEASE_TYPE "")
	set (VERSION_STR "${API_REVISION}.${VERSION_MAJOR}.${VERSION_MINOR}${RELEASE_TYPE}")
	add_definitions (
		-DZ80EX_VERSION_STR=${VERSION_STR}
		-DZ80EX_API_REVISION=${API_REVISION}
		-DZ80EX_VERSION_MAJOR=${VERSION_MAJOR}
		-DZ80EX_VERSION_MINOR=${VERSION_MINOR}
		-DZ80EX_RELEASE_TYPE=${RELEASE_TYPE})
	set(INCLUDIRS ${INCLUDIRS} ./src/libxpeccy/libz80ex/include)
	set(SOURCES ${SOURCES} ./src/libxpeccy/libz80ex/z80ex.c ./src/libxpeccy/libz80ex/z80ex_dasm.c)
	set(Z80EX_TYPE 2)
else()
	set(SOURCES ${SOURCES} ./src/libxpeccy/z80/z80.c)
	add_definitions(-DSELFZ80)
	set(Z80EX_TYPE 0)
endif()

set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O2 ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -O2 ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "-g -DISDEBUG ${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DISDEBUG ${CMAKE_CXX_FLAGS_DEBUG}")

# -DDRAWING=[GL | *] (Qt by default)
if("${DRAWING}" STREQUAL "GL")
find_package(OpenGL REQUIRED)
	if(OPENGL_FOUND)
		set(INCLUDIRS ${INCLUDIRS} ${OPENGL_INCLUDE_DIR})
		set(LIBRARIES ${LIBRARIES} ${OPENGL_LIBRARY})
	else()
		message(ERROR "OpenGL libraries not found")
	endif()
	add_definitions(-DDRAWGL)
	find_package(Qt4 REQUIRED QtCore QtGui QtOpenGL)
	set(DRAWTYPE 1)
else()
	add_definitions(-DDRAWQT)
	find_package(Qt4 REQUIRED QtCore QtGui)
	set(DRAWTYPE 0)
endif()

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
endif()

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
	${HEADERS}
	${UIHEADERS}
	${RESOURCES}
	${MOCHEADERS}
)

target_link_libraries(xpeccy ${LIBRARIES})

# end. print messages

message(STATUS "\n")
message(STATUS "Project will be compiled with following options:")

if(UNIX)
	install_targets(/bin xpeccy)
	install(FILES images/xpeccy.png DESTINATION share/icons)
	install(FILES Xpeccy.desktop DESTINATION share/applications)

set(CPACK_PACKAGE_NAME "Xpeccy")
set(CPACK_PACKAGE_VERSION ${XVER})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION "ZX Spectrum emulator")
set(CPACK_PACKAGE_VENDOR "SAM style")
set(CPACK_PACKAGE_CONTACT "samstyle@list.ru")

if ("${PACK}" STREQUAL "rpm")

# RPM package

message(STATUS "Package type : RPM")
set(CPACK_GENERATOR "RPM")
set(CPACK_RPM_PACKAGE_LICENSE "GPLv2")
set(CPACK_RPM_PACKAGE_GROUP "Games")
set(CPACK_RPM_PACKAGE_REQUIRES "SDL >= 1.2, libqt4 >= 4.6, libqt4-x11 >= 4.6, libasound2 >= 1.0, zlib >= 1.2")
if ("${DRAWTYPE}" EQUAL "1")
	set(CPACK_RPM_PACKAGE_DEPENDS ${CPACK_RPM_PACKAGE_DEPENDS}", libqt4-opengl >=4.6")
endif()
if("${Z80TYPE}" EQUAL "1")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libz80ex1 >= 1.1.21")
endif()

else ()

# DEB package

message(STATUS "Package type : DEB")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libSDL1.2debian (>=1.2), libqtcore4 (>=4.6), libqtgui4 (>=4.6), libasound2 (>=1.0), zlib1g (>=1.2)")
if (DRAWTYPE EQUAL 1)
	set(CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS} ", libqt4-opengl (>=4.6)")
endif()
set(CPACK_DEBIAN_PACKAGE_SECTION "Games")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
if(Z80TYPE EQUAL 1)
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libz80ex1 (>=1.1.21)")
endif()

endif()

include(${CMAKE_ROOT}/Modules/CPack.cmake)

endif(UNIX)

# result messages

if(DRAWTYPE EQUAL 1)
	message(STATUS "Painter : Qt OpenGL")
else()
	message(STATUS "Painter : Qt")
endif()

if(Z80EX_TYPE EQUAL 0)
	message(STATUS "libz80ex : not used")
elseif(Z80EX_TYPE EQUAL 1)
	message(STATUS "libz80ex : external")
elseif(Z80EX_TYPE EQUAL 2)
	message(STATUS "libz80ex : internal")
endif()

if (BIG_ENDIAN)
	message(STATUS "Endianness: big endian")
else ()
	message(STATUS "Endianness: little endian")
endif()

message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "\n")
