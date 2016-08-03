CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(remote-server CXX)
SET(EXTERNAL_RESOURCES_DIR ${CMAKE_CURRENT_BINARY_DIR}/external)


# --- VARIABLES

# -- building options

# C++
SET(CMAKE_CXX_FLAGS "-Wall -DHAVE_NETINET_IN_H -lsystemd-daemon -lpthread -Wno-variadic-macros -fPIC")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_VERBOSE_MAKEFILE ON) # make verbosity

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET(NODE_DIR "/usr/apps/")

LINK_DIRECTORIES(/usr/lib)

include_directories(${EXTERNAL_RESOURCES_DIR}/include)
link_directories(${EXTERNAL_RESOURCES_DIR}/lib)
include_directories(${DBUS_INCLUDE})
include_directories(${DBUS_TRY})



FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GLIB2 REQUIRED glib-2.0)
PKG_CHECK_MODULES(DBUS-GLIB REQUIRED dbus-glib-1)

INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS} ${DBUS-GLIB_INCLUDE_DIRS})
LINK_DIRECTORIES(${GLIB2_LIBRARY_DIRS} ${DBUS-GLIB_LIBRARY_DIRS})

set(CMAKE_FIND_ROOT_PATH ${EXTERNAL_RESOURCES_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Available Modules : ACL, DIAL, SmartView(Pincode), MSF
# All support : High grade TV model (ex. HawkP, HawkM)
# ACL only support : LFD

IF("${PRODUCT_TYPE}" STREQUAL "LFD")
	ADD_DEFINITIONS("-DSUPPORT_ACL_ONLY")

SET( CM-MANAGER-SRCS
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTEMutex.cpp
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTESemaphore.cpp
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTEThread.cpp
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTEString.cpp
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTEApp.cpp
        )

else()
SET( CM-MANAGER-SRCS
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTEMutex.cpp
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTESemaphore.cpp
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTEThread.cpp
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTEString.cpp
        ${CMAKE_SOURCE_DIR}/Src/Common/REMOTEApp.cpp
		${CMAKE_SOURCE_DIR}/PinCodeService/IPCServer.cpp
		${CMAKE_SOURCE_DIR}/PinCodeService/MSFServer.cpp
		${CMAKE_SOURCE_DIR}/PinCodeService/Util/MSFSend.cpp
		${CMAKE_SOURCE_DIR}/PinCodeService/Util/RemoteLogger.cpp
		)

ENDIF("${PRODUCT_TYPE}" STREQUAL "LFD")

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/Inc
#        ${CMAKE_SOURCE_DIR}/usr/include/webconv
        ${CMAKE_SOURCE_DIR}/Inc/Common
	${CMAKE_SOURCE_DIR}/PinCodeService
	${CMAKE_SOURCE_DIR}/PinCodeService/Util
	${DBUS_INCLUDE}
	/usr/include/vd_kernel-headers
	)

INCLUDE(FindPkgConfig)
pkg_check_modules(packages REQUIRED
	capi-system-info
	capi-network-wifi
	capi-network-connection
	capi-appfw-app-manager
	glib-2.0
	libcurl
	dlog
	dbus-glib-1
	gobject-2.0
	gmodule-2.0
	syspopup
	syspopup-caller
	capi-appfw-application
	vconf
	capi-system-system-settings
	gio-2.0
	libsystemd
	jsoncpp
	deviced
	libxml-2.0
	bluetooth-api
	argos_watchdog
	aul
)


FOREACH(flag ${packages_CFLAGS})
       SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -fvisibility=hidden -Wall")
SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -g ")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")

ADD_LIBRARY(CManager ${CM-MANAGER-SRCS})

ADD_EXECUTABLE(${PROJECT_NAME}  ${CMAKE_SOURCE_DIR}/main.cpp ) # compile the main executable file

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    ${GLIB2_LIBRARIES}
    ${DBUS-GLIB_LIBRARIES}
    ${packages_LDFLAGS}
    boost_system
    boost_thread
    libexpat.so
    libcrypto.so
    CManager
) # link the main executable file with libraries



INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/MSF-Node/org.tizen.multiscreen DESTINATION ${NODE_DIR})

INSTALL(FILES packaging/org.tizen.multiscreen.service packaging/remote-server.service DESTINATION /usr/lib/systemd/user)
INSTALL(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
