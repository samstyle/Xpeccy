cmake_minimum_required(VERSION 2.8)
project(xpeccy)
file(STRINGS "Release" XVER)

#set(XVER "0.6.20160119")

#set(CMAKE_C_COMPILER gcc)
#set(CMAKE_CXX_COMPILER g++)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
include(${CMAKE_ROOT}/Modules/CPack.cmake)

# endianess

include (TestBigEndian)
test_big_endian(BIG_ENDIAN)
if (BIG_ENDIAN)
	add_definitions(-DWORDS_BIG_ENDIAN)
else()
	add_definitions(-DWORDS_LITTLE_ENDIAN)
endif()

# compilation flags

set(CMAKE_C_FLAGS "-std=gnu99 -Wall -O2 ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG ${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_DEBUG "-g -DISDEBUG ${CMAKE_C_FLAGS_DEBUG}")

set(CMAKE_CXX_FLAGS "-Wall -O2 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DISDEBUG ${CMAKE_CXX_FLAGS_DEBUG}")

# OS-depended section

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	set(INC_PATHS local/include include)
	set(LIB_PATHS lib lib64 local/lib local/lib64)

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

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

	set(INC_PATHS local/include include)
	set(LIB_PATHS lib lib64 local/lib local/lib64)

	set(CPACK_GENERATOR "DragNDrop")
	set(CPACK_DMG_NAME "Xpeccy")
	set(CPACK_DMG_FORMAT UDZO)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	set(INC_PATHS ${INCLUDE} ${PATH})
	set(LIB_PATHS ${LIBPATH} ${PATH})

else()

	message(ERROR "Unsupported system : ${CMAKE_SYSTEM_NAME}")

endif()

set(INCLUDIRS ${CMAKE_BINARY_DIR})

# sources
file(GLOB SOURCES
	./src/*.cpp
	./src/*/*.cpp
	./src/*/*.c
	./src/*/*/*.c
	./src/*/*/*/*.c
	./src/*/*/*/*/*.c
	./xpeccy.rc
)

# headers
file(GLOB HEADERS
	./src/*.h
	./src/*/*.h
	./src/*/*/*.h
	./src/*/*/*/*.h
	./src/*/*/*/*/*.h
)

# Qt user interface
file(GLOB UIFILES ./ui/*.ui)

# Qt resources
set(QRCFILES ./xpeccy.qrc)

# Qt moc files
set(MOCFILES
	./src/ethread.h
	./src/emulwin.h
	./src/debuger.h
	./src/dbg_sprscan.h
	./src/dbg_memfill.h
	./src/dbg_finder.h
	./src/watcher.h
	./src/setupwin.h
	./src/xgui/xgui.h
)

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

# packaging

set(CPACK_PACKAGE_NAME "Xpeccy")
set(CPACK_PACKAGE_VERSION ${XVER})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION "ZX Spectrum emulator")
set(CPACK_PACKAGE_VENDOR "SAM style")
set(CPACK_PACKAGE_CONTACT "samstyle@list.ru")

# install

install_targets(/bin xpeccy)
install(FILES images/xpeccy.png DESTINATION share/icons)
install(FILES Xpeccy.desktop DESTINATION share/applications)

# print messages

message(STATUS "\n")
message(STATUS "Project will be compiled with following options:")
message(STATUS "OS : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Version : ${XVER}")

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
