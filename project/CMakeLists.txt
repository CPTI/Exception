include_directories(src)

SET(HEADERS
	src/config.h
	src/Software.h
	src/MapUtils.h
	src/SymbolCache.h
	src/Error.h
	src/string_format.h
	src/VectorOf.h
	src/ArrayPtr.h
	src/Logger.h
	src/NullType.h
	src/ScopeGuard.h
	src/svector.h
	src/LoggerFwd.h
	src/Typelist.h
	src/DebugSymbolLoader.h
	src/Exception.h
	src/TypeManip.h
	src/StackAddressLoader.h
	src/BackTrace.h
	src/TypelistMacros.h
	src/NotNull.h
	src/Demangling.h
	src/str_conversion.h
	src/VectorIO.h
)

SET(SOURCES
	src/Error.cpp
	src/Exception.cpp
	src/main.cpp
	src/VectorIO.cpp
	src/SymbolCache.cpp
	src/string_format.cpp
	src/Logger.cpp
	src/BackTracePlatIndep.cpp
	src/Demangling.cpp
)

IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	SET(SOURCES
		${SOURCES}
		src/windows/StackLoader.cpp
		src/windows/BackTrace.cpp
	)
	IF(CMAKE_COMPILER_IS_GNUCXX)
		SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CONF_LINKER_FLAGS}") 
		SET(LIBS bfd iberty)
	ELSE()
		SET(LIBS imagehlp.lib)
	ENDIF()
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CONF_LINKER_FLAGS}") 
	SET(SOURCES ${SOURCES}
		src/linux/BackTrace.cpp
		src/linux/StackLoader.cpp
	)
	IF(USE_ADDR2LINE)
		SET(SOURCES ${SOURCES} src/linux/DebugSymbolLoader.cpp)
	ELSE()
		SET(SOURCES ${SOURCES} src/bfd/DebugSymbolLoader.cpp)
		SET(LIBS bfd dl z iberty)
	ENDIF()
ELSE()
	SET(SOURCES
		${SOURCES}
		src/default/StackLoader.cpp
		src/default/DebugSymbolLoader.cpp
	)
ENDIF()

IF(BUILD_QT OR SUPPORT_QT)
SET(LIBS ${LIBS} ${QT_LIBRARIES})
ENDIF()

add_library(exception SHARED ${HEADERS} ${SOURCES})
target_link_libraries(exception ${LIBS})
export(TARGETS exception FILE "${PROJECT_BINARY_DIR}/ExceptionTargets.cmake")
install(EXPORT ExceptionTargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)

IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
install(TARGETS exception EXPORT ExceptionTargets LIBRARY DESTINATION "${INSTALL_LIB_DIR}")
install(DIRECTORY src/ DESTINATION "${INSTALL_INCLUDE_DIR}/exception" FILES_MATCHING PATTERN "*.h" PATTERN "*config.h" EXCLUDE)
ENDIF()
