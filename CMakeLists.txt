cmake_minimum_required(VERSION 3.22...3.28)

#compiler settings

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# common

set(PROJECT_NAME "xpeccy")
project(${PROJECT_NAME})

option(USEOPENGL    "Enable OpenGL-accelerated rendering" ON)
option(USEQTNETWORK "Enable Qt network support" OFF)
set(QTVERSION "" CACHE STRING "Force Qt version (5 or 6). Empty = auto-detect.")

file(STRINGS "Release" XVER)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)

# endianness

include (TestBigEndian)
test_big_endian(BIG_ENDIAN)
if (BIG_ENDIAN)
	add_definitions(-DWORDS_BIG_ENDIAN)
else()
	add_definitions(-DWORDS_LITTLE_ENDIAN)
endif()

# compilation flags

set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG ${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_DEBUG "-g -DISDEBUG ${CMAKE_C_FLAGS_DEBUG}")

set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DISDEBUG ${CMAKE_CXX_FLAGS_DEBUG}")

# OS-depended section

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${XVER})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION "ZX Spectrum emulator")
set(CPACK_PACKAGE_VENDOR "SAM style")
set(CPACK_PACKAGE_CONTACT "samstyle@list.ru")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	set(INC_PATHS local/include include)
	set(LIB_PATHS lib lib64 local/lib local/lib64)

	set(LIBRARIES ${LIBRARIES} dl)

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

	install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME} DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE WORLD_READ WORLD_EXECUTE GROUP_READ GROUP_EXECUTE
	)
	install(FILES images/xpeccy.png DESTINATION share/icons)
	install(FILES Xpeccy.desktop DESTINATION share/applications)
	# Ship bundled palettes under the XDG Data readonly search path. The
	# framework resolves ResourceKind::Palette via $XDG_DATA_DIRS entries +
	# samstyle/xpeccy/palettes, so files installed here appear as system
	# defaults alongside any user copies in $XDG_DATA_HOME.
	install(DIRECTORY conf/palettes/
	        DESTINATION share/samstyle/xpeccy/palettes
	        FILES_MATCHING PATTERN "*.txt" PATTERN "*.pal")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "BSD")

	set(INC_PATHS pkg/include include)
	set(LIB_PATHS lib lib64 pkg/lib local/lib64)

	add_definitions("-D__BSD")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

	set(INC_PATHS local/include include)
	set(LIB_PATHS lib lib64 local/lib local/lib64)

	set(CPACK_GENERATOR "DragNDrop")
	set(CPACK_DMG_NAME ${PROJECT_NAME})
	set(CPACK_DMG_FORMAT UDZO)
#	set(CPACK_DMG_BACKGROUND_IMAGE ${CMAKE_SOURCE_DIR}/background.png)

	set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}.app)
	set(MACOSX_BUNDLE_ICON_FILE ${PROJECT_NAME}.icns)
	set(MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist)
	set(MACOSX_BUNDLE_BUNDLE_VERSION ${XVER})

	set(BUNDLE_ICON_PATH ${CMAKE_SOURCE_DIR}/images/${MACOSX_BUNDLE_ICON_FILE})
	set(BUNDLE_PATH ${CMAKE_BINARY_DIR}/${MACOSX_BUNDLE_BUNDLE_NAME})

	set_source_files_properties(${CMAKE_BINARY_DIR}/${PROJECT_NAME} PROPERTIES MACOSX_PACKAGE_LOCATION MacOSX)
	set_source_files_properties(${BUNDLE_ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	set(CPACK_GENERATOR "ZIP")
	set(LIBRARIES ${LIBRARIES} winmm.dll)
	set(INC_PATHS $ENV{INCLUDE} $ENV{PATH})
	set(LIB_PATHS $ENV{LIBPATH} $ENV{PATH})

else()

	message(FATAL_ERROR "Unsupported system : ${CMAKE_SYSTEM_NAME}")

endif()

set(INCLUDIRS
	${CMAKE_BINARY_DIR}
	./src
)

# sources
file(GLOB_RECURSE SOURCES
	./src/*.cpp
	./src/*.c
)

# headers
file(GLOB_RECURSE HEADERS
	./src/*.h
)

# Qt user interface
file(GLOB_RECURSE UIFILES
	./ui/*.ui
)

# Qt resources
set(QRCFILES ./xpeccy.qrc)

# Qt moc files
set(MOCFILES
	./src/ethread.h
	./src/emulwin.h
	./src/xapp.h
	./src/xgui/debuga/debuger.h
	./src/xgui/debuga/dbg_sprscan.h
	./src/xgui/debuga/dbg_memfill.h
	./src/xgui/debuga/dbg_finder.h
	./src/xgui/debuga/dbg_dump.h
	./src/xgui/debuga/dbg_disasm.h
	./src/xgui/debuga/dbg_brkpoints.h
	./src/xgui/debuga/dbg_diskdump.h
	./src/xgui/debuga/dbg_vmem_dump.h
	./src/xgui/debuga/dbg_cmos_dump.h
	./src/xgui/debuga/dbg_widgets.h
	./src/xgui/debuga/dbg_rdump.h
	./src/watcher.h
	./src/xgui/options/setupwin.h
	./src/xgui/options/padbinder.h
	./src/xgui/options/opt_romset.h
	./src/xgui/options/opt_diskcat.h
	./src/xgui/options/opt_tapecat.h
	./src/xgui/options/opt_hotkeytab.h
	./src/xgui/options/opt_paledit.h
	./src/xgui/options/opt_gamepad.h
	./src/vkeyboard.h
#	./src/xcore/xcore.h
	./src/xcore/gamepad.h
	./src/xgui/xgui.h
	./src/xgui/classes.h
	./src/xgui/labelist.h
)

# Qt version detection — prefer Qt6, fall back to Qt5.
# Override with -DQTVERSION=5 or -DQTVERSION=6 (or legacy -DQT6BUILD=1/0).

if(DEFINED QT6BUILD)
	if(QT6BUILD)
		set(QTVERSION 6 CACHE STRING "" FORCE)
	else()
		set(QTVERSION 5 CACHE STRING "" FORCE)
	endif()
endif()

if(QTVERSION STREQUAL "")
	find_package(QT NAMES Qt6 Qt5 REQUIRED)
else()
	find_package(QT NAMES Qt${QTVERSION} REQUIRED)
endif()

set(_QTV ${QT_VERSION_MAJOR})
if(_QTV EQUAL 6)
	set(_QT_MIN_VER 6.1)
else()
	set(_QT_MIN_VER 5.15)
endif()

# Core Qt components
find_package(Qt${_QTV} ${_QT_MIN_VER} REQUIRED COMPONENTS Core Gui Widgets)
set(LIBRARIES ${LIBRARIES} Qt${_QTV}::Widgets)

if(_QTV EQUAL 6)
	find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
	set(LIBRARIES ${LIBRARIES} Qt6::Core Qt6::Gui Qt6::Core5Compat)
endif()

cmake_language(CALL qt${_QTV}_add_resources RESOURCES ${QRCFILES})
cmake_language(CALL qt${_QTV}_wrap_ui UIHEADERS ${UIFILES})
cmake_language(CALL qt${_QTV}_wrap_cpp MOCHEADERS ${MOCFILES})

# Optional: network
if(USEQTNETWORK)
	find_package(Qt${_QTV} REQUIRED COMPONENTS Network)
	set(LIBRARIES ${LIBRARIES} Qt${_QTV}::Network)
	add_definitions(-DUSENETWORK)
endif()

# Optional: OpenGL
if(USEOPENGL)
	find_package(OpenGL REQUIRED)
	if(_QTV EQUAL 6)
		set(_QT_GL OpenGLWidgets)
	else()
		set(_QT_GL OpenGL)
	endif()
	find_package(Qt${_QTV} REQUIRED COMPONENTS ${_QT_GL})
	set(LIBRARIES ${LIBRARIES} Qt${_QTV}::${_QT_GL} ${OPENGL_LIBRARIES})
	add_definitions(-DUSEOPENGL)
endif()

# Packaging dependencies
if(_QTV EQUAL 6)
	set(CPACK_RPM_PACKAGE_REQUIRES "libQt6Gui6 >= ${_QT_MIN_VER}, libQt6Widgets6 >= ${_QT_MIN_VER}, libQt6Core5Compat6 >= ${_QT_MIN_VER}")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6gui6 (>=${_QT_MIN_VER}), libqt6widgets6 (>=${_QT_MIN_VER}), libqt6core5compat6 (>=${_QT_MIN_VER})")
	if(USEQTNETWORK)
		set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libQt6Network6 >= ${_QT_MIN_VER}")
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libqt6network6 (>=${_QT_MIN_VER})")
	endif()
	if(USEOPENGL)
		set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libQt6OpenGL6 >= ${_QT_MIN_VER}")
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libqt6opengl6 (>=${_QT_MIN_VER})")
	endif()
else()
	set(CPACK_RPM_PACKAGE_REQUIRES "libQt5Gui5 >= ${_QT_MIN_VER}, libQt5Widgets5 >= ${_QT_MIN_VER}")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5gui5 (>=${_QT_MIN_VER}), libqt5widgets5 (>=${_QT_MIN_VER})")
	if(USEQTNETWORK)
		set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libQt5Network5 >= ${_QT_MIN_VER}")
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libqt5network5 (>=${_QT_MIN_VER})")
	endif()
	if(USEOPENGL)
		set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libQt5OpenGL5 >= ${_QT_MIN_VER}")
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libqt5opengl5 (>=${_QT_MIN_VER})")
	endif()
endif()

set(QT_VER ${QT_VERSION})

# SDL

find_package(SDL2 REQUIRED)
if (${SDL2_FOUND})
	add_definitions(-DHAVESDL2)
	include_directories(${SDL2_INCLUDE_DIR})
	set(LIBRARIES ${LIBRARIES} ${SDL2_LIBRARY})
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libsdl2-2.0-0 (>=2.0) | libSDL2 (>=2.0)")
	set(CPACK_RPM_PACKAGE_DEPENDS "${CPACK_RPM_PACKAGE_DEPENDS}, SDL2 >= 2.0")
	set(SDL_VER "2.x")
	set(SDL_NAME "SDL2")
	message(STATUS "SDL2_INCLUDE_DIR = " ${SDL2_INCLUDE_DIR})
endif(${SDL2_FOUND})

# zlib (for rzx)

find_package(ZLIB)
if(ZLIB_FOUND)
	add_definitions(-DHAVEZLIB)
	set(INCLUDIRS ${INCLUDIRS} ${ZLIB_INCLUDE_DIR})
	set(LIBRARIES ${LIBRARIES} ${ZLIB_LIBRARY})
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, zlib1g (>=1.2)")
	set(CPACK_RPM_PACKAGE_DEPENDS "${CPACK_RPM_PACKAGE_DEPENDS}, zlib >= 1.2")
endif(ZLIB_FOUND)

# vendored dependencies

include_directories(${PROJECT_SOURCE_DIR}/3rdparty/xdgpp)

# executable

set(ALL_SRCS ${SOURCES} ${HEADERS} ${UIHEADERS} ${RESOURCES} ${MOCHEADERS})

if(APPLE)
	add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${ALL_SRCS} ${BUNDLE_ICON_PATH})
	find_program(MACDEPLOYQTEXE macdeployqt)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${MACDEPLOYQTEXE} ARGS ${BUNDLE_PATH} -always-overwrite
		COMMAND mkdir ARGS -p ${BUNDLE_PATH}/Contents/Frameworks
                COMMAND cp ARGS -R -X /Library/Frameworks/${SDL_NAME}.framework ${BUNDLE_PATH}/Contents/Frameworks
                COMMAND install_name_tool -change @rpath/${SDL_NAME}.framework/Versions/A/${SDL_NAME} @executable_path/../Frameworks/${SDL_NAME}.framework/Versions/A/${SDL_NAME} ${BUNDLE_PATH}/Contents/MacOS/xpeccy
	)
	set(CMAKE_INSTALL_PREFIX "/Applications")
	install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION .)
elseif(WIN32)
	add_executable(${PROJECT_NAME} WIN32 ${ALL_SRCS} ${CMAKE_SOURCE_DIR}/xpeccy.rc)
else()
	add_executable(${PROJECT_NAME} ${ALL_SRCS})
endif()

include_directories(${INCLUDIRS})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

include(${CMAKE_ROOT}/Modules/CPack.cmake)

# install

# print messages

message(STATUS "\n")
message(STATUS "Project will be compiled with following options:\n")
message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "Xpeccy version : ${XVER}")
message(STATUS "OS : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Qt version : ${QT_VER}")
message(STATUS "SDL version : ${SDL_VER}")

if (${USEQTNETWORK})
	message(STATUS "Network: yes")
else()
	message(STATUS "Network: no")
endif()

if (${USEOPENGL})
	message(STATUS "OpenGL: yes")
else()
	message(STATUS "OpenGL: no")
endif()

if (BIG_ENDIAN)
	message(STATUS "Endianness: BE")
else ()
	message(STATUS "Endianness: LE")
endif()

message(STATUS "\n")
