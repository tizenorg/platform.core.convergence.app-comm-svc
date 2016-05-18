CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(msf-api-test)
INCLUDE(GNUInstallDirs)

# Targets
SET(target "msf-api-test")

# Source Lists
SET(SRCS main.cpp)
MESSAGE("Sources: ${SRCS}")

# Dependencies
SET(DEPS "glib-2.0 gio-2.0 json-glib-1.0 dlog openssl msf-api libwebsockets libcurl")

INCLUDE(FindPkgConfig)
INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}/include
)
ADD_DEFINITIONS(-g -O2 -Wall -fPIC -Wl,--as-needed)

# Building Library
pkg_check_modules(lib_pkg REQUIRED ${DEPS})

FOREACH(flag ${lib_pkg_CFLAGS})
SET(LIB_EXTRA_CFLAGS "${LIB_EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

ADD_EXECUTABLE(${target} ${SRCS})
TARGET_LINK_LIBRARIES(${target} ${lib_pkg_LDFLAGS} -lcurl msf-api nsd-dns-sd glib-2.0 gio-2.0 json-glib-1.0)
SET_TARGET_PROPERTIES(${target} PROPERTIES COMPILE_FLAGS ${LIB_EXTRA_CFLAGS})
SET_TARGET_PROPERTIES(${target} PROPERTIES COMPILE_DEFINITIONS "LOG_TAG=\"MSF\"")
SET_TARGET_PROPERTIES(${target} PROPERTIES SOVERSION ${MAJORVER})
SET_TARGET_PROPERTIES(${target} PROPERTIES VERSION ${FULLVER})

# Installing Library
INSTALL(TARGETS ${target} DESTINATION bin)
