cmake_minimum_required(VERSION 2.8.11)
set(PROJECT_NAME "xpeccy")
project(${PROJECT_NAME})
file(STRINGS "Release" XVER)

#set(XVER "0.6.20160119")

#set(CMAKE_C_COMPILER gcc)
#set(CMAKE_CXX_COMPILER g++)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)

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

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${XVER})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION "ZX Spectrum emulator")
set(CPACK_PACKAGE_VENDOR "SAM style")
set(CPACK_PACKAGE_CONTACT "samstyle@list.ru")

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

#	install_targets(/bin ${PROJECT_NAME})
	install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
		DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE WORLD_READ WORLD_EXECUTE GROUP_READ GROUP_EXECUTE
	)
	install(FILES images/xpeccy.png
		DESTINATION share/icons
	)
	install(FILES Xpeccy.desktop
		DESTINATION share/applications
	)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

	set(INC_PATHS local/include include)
	set(LIB_PATHS lib lib64 local/lib local/lib64)

	set(CPACK_GENERATOR "DragNDrop")
	set(CPACK_DMG_NAME ${PROJECT_NAME})
	set(CPACK_DMG_FORMAT UDZO)

	set(CPACK_BUNDLE_NAME ${PROJECT_NAME}.app)
	set(CPACK_BUNDLE_ICON ${CMAKE_SOURCE_DIR}/images/xpeccy.icns)

	set_source_files_properties(${CMAKE_BINARY_DIR}/${PROJECT_NAME} PROPERTIES MACOS_PACKAGE_LOCATION MacOSX)
	set_source_files_properties(${CPACK_BUNDLE_ICON} PROPERTIES MACOS_PACKAGE_LOCATION Resources)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	set(LIBRARIES ${LIBRARIES} winmm.dll)
	set(INC_PATHS $ENV{INCLUDE} $ENV{PATH})
	set(LIB_PATHS $ENV{LIBPATH} $ENV{PATH})

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

	set(QT_VER ${Qt5Widgets_VERSION})

else()

	find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
	include(${QT_USE_FILE})
	qt4_add_resources(RESOURCES ${QRCFILES})
	qt4_wrap_ui(UIHEADERS ${UIFILES})
	qt4_wrap_cpp(MOCHEADERS ${MOCFILES})
	set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})
	set(CPACK_RPM_PACKAGE_REQUIRES "libqt4 >= 4.6, libqt4-x11 >= 4.6")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqtcore4 (>=4.6), libqtgui4 (>=4.6)")

	set(QT_VER ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH})

endif()

# alsa (sound, linux)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(ALSA)
	if (ALSA_FOUND)
		add_definitions(-DHAVEALSA)
		set(INCLUDIRS ${INCLUDIRS} ${ALSA_INCLUDE_DIR})
		set(LIBRARIES ${LIBRARIES} ${ALSA_LIBRARY})
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libasound2 (>=1.0)")
		set(CPACK_RPM_PACKAGE_DEPENDS "${CPACK_RPM_PACKAGE_DEPENDS}, libasound2 >= 1.0")
	endif(ALSA_FOUND)
endif()

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

include_directories(${INCLUDIRS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UIHEADERS} ${RESOURCES} ${MOCHEADERS})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${UIHEADERS} ${RESOURCES} ${MOCHEADERS})
	find_program(MACDEPLOYQTEXE macdeployqt)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND mkdir ARGS -p ${CMAKE_BINARY_DIR}/${CPACK_BUNDLE_NAME}/Contents/Frameworks
		COMMAND cp ARGS -R /Library/Frameworks/SDL.framework ${CMAKE_BINARY_DIR}/${CPACK_BUNDLE_NAME}/Contents/Frameworks
		COMMAND ${MACDEPLOYQTEXE} ARGS ${CMAKE_BINARY_DIR}/${CPACK_BUNDLE_NAME} -always-overwrite
	)
	set(CMAKE_INSTALL_PREFIX "/Applications")
	install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION .)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${UIHEADERS} ${RESOURCES} ${MOCHEADERS})
endif()

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
message(STATUS "SDL version : ${SDL_VERSION_STRING}")

if (BIG_ENDIAN)
	message(STATUS "Endianness: BE")
else ()
	message(STATUS "Endianness: LE")
endif()

message(STATUS "\n")
