cmake_minimum_required(VERSION 3.0)
project(xpeccy)
file(STRINGS "Release" XVER)

#set(XVER "0.6.20160119")

#set(CMAKE_C_COMPILER clang)
#set(CMAKE_CXX_COMPILER clang++)

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
	./src/xcore/*.cpp
	./src/xgui/*.cpp
	./src/libxpeccy/*.c
	./src/libxpeccy/z80/z80.c
	./src/libxpeccy/sound/*.c
	./src/libxpeccy/video/*.c
	./src/libxpeccy/hardware/*.c
	./src/libxpeccy/filetypes/*.c
	./src/libxpeccy/filetypes/*.cpp
	./src/libxpeccy/filetypes/unpackers/*.c
	./src/libxpeccy/filetypes/unpackers/lh1/*.c
if(WIN32)
	./xpeccy.rc
endif(WIN32)
)
file(GLOB HEADERS
	./src/*.h
	./src/xcore/*.h
	./src/xgui/*.h
	./src/libxpeccy/*.h
	./src/libxpeccy/z80/*.h
	./src/libxpeccy/sound/*.h
	./src/libxpeccy/video/*.h
	./src/libxpeccy/hardware/*.h
	./src/libxpeccy/filetypes/*.h
	./src/libxpeccy/filetypes/unpackers/*.h
	./src/libxpeccy/filetypes/unpackers/lh1/*.h
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

set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O2 ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG ${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_DEBUG "-g -DISDEBUG ${CMAKE_C_FLAGS_DEBUG}")

set(CMAKE_CXX_FLAGS "-Wall -O2 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DISDEBUG ${CMAKE_CXX_FLAGS_DEBUG}")

# Qt4 / Qt5

if("${QT5BUILD}")

	find_package(Qt5Widgets)
	set(LIBRARIES ${LIBRARIES} Qt5::Widgets)
	qt5_add_resources(RESOURCES ${QRCFILES})
	qt5_wrap_ui(UIHEADERS ${UIFILES})
	qt5_wrap_cpp(MOCHEADERS ${MOCFILES})
	set(CPACK_RPM_PACKAGE_REQUIRES "libQt5Gui5 >= 5.3, libQt5Widgets5 >= 5.3")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5gui5 (>=5.3), libqt5widgets5 (>=5.3)")

else()

	find_package(Qt4 REQUIRED QtCore QtGui)
	qt4_add_resources(RESOURCES ${QRCFILES})
	qt4_wrap_ui(UIHEADERS ${UIFILES})
	qt4_wrap_cpp(MOCHEADERS ${MOCFILES})
	include(${QT_USE_FILE})
	set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})
	set(CPACK_RPM_PACKAGE_REQUIRES "libqt4 >= 4.6, libqt4-x11 >= 4.6")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqtcore4 (>=4.6), libqtgui4 (>=4.6)")

endif()

# alsa (sound, linux)

if(UNIX)
	find_package(ALSA)
	if (ALSA_FOUND)
		add_definitions(-DHAVEALSA)
		set(INCLUDIRS ${INCLUDIRS} ${ALSA_INCLUDE_DIR})
		set(LIBRARIES ${LIBRARIES} ${ALSA_LIBRARY})
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libasound2 (>=1.0)")
		set(CPACK_RPM_PACKAGE_DEPENDS "${CPACK_RPM_PACKAGE_DEPENDS}, libasound2 >= 1.0")
	endif(ALSA_FOUND)
endif(UNIX)

# SDL (sound)

find_package(SDL)
if(SDL_FOUND)
	add_definitions(-DHAVESDL)
	set(INCLUDIRS ${INCLUDIRS} ${SDL_INCLUDE_DIR})
	set(LIBRARIES ${LIBRARIES} ${SDL_LIBRARY})
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libSDL1.2debian (>=1.2)")
	set(CPACK_RPM_PACKAGE_DEPENDS "${CPACK_RPM_PACKAGE_DEPENDS}, SDL >= 1.2")
endif()

# zlib (for rzx)

find_package(ZLIB)
if(ZLIB_FOUND)
	add_definitions(-DHAVEZLIB)
	set(INCLUDIRS ${INCLUDIRS} ${ZLIB_INCLUDE_DIR})
	set(LIBRARIES ${LIBRARIES} ${ZLIB_LIBRARY})
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, zlib1g (>=1.2)")
	set(CPACK_RPM_PACKAGE_DEPENDS "${CPACK_RPM_PACKAGE_DEPENDS}, zlib >= 1.2")
endif(ZLIB_FOUND)

# other

if(WIN32)
	set(LIBRARIES ${LIBRARIES} winmm.dll)
endif(WIN32)

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
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Games")

else ()

# DEB package

message(STATUS "Package type : DEB")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_SECTION "Games")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

endif()

include(${CMAKE_ROOT}/Modules/CPack.cmake)

endif(UNIX)

# result messages

message(STATUS "Version: ${XVER}")

if(QT5BUILD)
	message(STATUS "Qt version : 5")
else()
	message(STATUS "Qt version : 4")
endif()

if (BIG_ENDIAN)
	message(STATUS "Endianness: big endian")
else ()
	message(STATUS "Endianness: little endian")
endif()

message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "\n")
