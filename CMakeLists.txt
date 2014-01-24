CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(nitz-plugin C)

# INCLUDE(FindPkgConfig)
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")

# Set required packages
INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED glib-2.0 gio-2.0 tcore vconf sysman)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wextra -fvisibility=hidden -fPIC")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-missing-field-initializers -Wdeclaration-after-statement -Wmissing-declarations -Wredundant-decls -Wcast-align")

ADD_DEFINITIONS("-DFEATURE_TLOG_DEBUG")
ADD_DEFINITIONS("-DTCORE_LOG_TAG=\"NITZ\"")
ADD_DEFINITIONS("-DPLUGIN_VERSION=${VERSION}")
ADD_DEFINITIONS("-DEXPORT_API=__attribute__((visibility(\"default\")))")

MESSAGE(${CMAKE_C_FLAGS})
MESSAGE(${pkgs_LDFLAGS})

SET(SRCS
	src/desc-nitz.c
	src/time_update.c
	src/citylist.c
)

# library build
ADD_LIBRARY(nitz-plugin SHARED ${SRCS})
TARGET_LINK_LIBRARIES(nitz-plugin ${pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(nitz-plugin PROPERTIES PREFIX "" OUTPUT_NAME nitz-plugin)

# install
INSTALL(TARGETS nitz-plugin
		LIBRARY DESTINATION lib/telephony/plugins)
