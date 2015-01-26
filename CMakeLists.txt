cmake_minimum_required(VERSION 2.8)

project(exception CXX)
set(EXCEPTION_MAJOR_VERSION 0)
set(EXCEPTION_MINOR_VERSION 1)
set(EXCEPTION_PATCH_VERSION 0)
set(EXCEPTION_VERSION ${EXCEPTION_MAJOR_VERSION}.${EXCEPTION_MINOR_VERSION}.${EXCEPTION_PATCH_VERSION})

IF(BUILD_QT OR SUPPORT_QT)
    FIND_PACKAGE(Qt4 REQUIRED)
    INCLUDE(${QT_USE_FILE})
    ADD_DEFINITIONS(${QT_DEFINITIONS})
    set(CONF_INCLUDE_DIRS "${INSTALL_INCLUDE_DIR}")
    set(SUPPORT_DEFINITION "#define SUPPORT_QT 1")
    set(QT_CONF "
        FIND_PACKAGE(Qt4 REQUIRED)
        INCLUDE(${QT_USE_FILE})
        ADD_DEFINITIONS(${QT_DEFINITIONS})
    ")
    IF(BUILD_QT)
            set(EXCEPTION_CONF "Qt")
            set(USE_DEFINITION "#define USE_QT 1")
    ELSE()
            set(EXCEPTION_CONF "C++11+Qt")
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
            set(USE_DEFINITION "#define USE_CXX11 1")
    ENDIF()
ELSE()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(EXCEPTION_CONF "C++11")
        set(USE_DEFINITION "#define USE_CXX11 1")
        set(SUPPORT_DEFINITION "")
        set(QT_CONF "")
ENDIF()

set(LIBSUFFIX "")
IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
	if (${LIB64} STREQUAL "TRUE")
	    set(LIBSUFFIX 64)
	else()
	    set(LIBSUFFIX "")
	endif()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLINUX")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
		SET(CONF_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--wrap,__cxa_throw -Wl,--wrap,__cxa_bad_cast") 
ENDIF()
export(PACKAGE exception)
set(INSTALL_CMAKE_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIBSUFFIX}/cmake/Exception")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIBSUFFIX}")
set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")

set(CONF_INCLUDE_DIRS "${INSTALL_INCLUDE_DIR}")
set(CONF_BUILD_CONF "${EXCEPTION_CONF}")
configure_file(ExceptionConfig.cmake.in
  "${PROJECT_BINARY_DIR}/ExceptionConfig.cmake" @ONLY)

configure_file(ExceptionConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/ExceptionConfigVersion.cmake" @ONLY)
 
install(FILES
  "${PROJECT_BINARY_DIR}/ExceptionConfig.cmake"
  "${PROJECT_BINARY_DIR}/ExceptionConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)


configure_file(config.h.cmake.in
  "${PROJECT_BINARY_DIR}/config.h" @ONLY)

install(FILES "${PROJECT_BINARY_DIR}/config.h" DESTINATION "${INSTALL_INCLUDE_DIR}/exception" COMPONENT dev)

ENABLE_TESTING()
add_subdirectory(project)
