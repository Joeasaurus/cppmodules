#### COMMON ####
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(flags "-O3 -Wall -Wextra -Wno-unused-parameter")

	set(CMAKE_CXX_FLAGS "${flags} -std=c++11"
		CACHE STRING "Flags used by the compiler during all build types." FORCE)
	set(CMAKE_C_FLAGS   "${flags}"
		CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

find_package(Boost REQUIRED)
include_directories("${CMAKE_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/generated")
include_directories(
	${Boost_INCLUDE_DIRS}
	"${CMAKE_SOURCE_DIR}/submodules/cppevent/include"
	"${CMAKE_SOURCE_DIR}/submodules/spdlog/include"
	"${CMAKE_SOURCE_DIR}/submodules/catch/single_include"
	"${CMAKE_SOURCE_DIR}/submodules/cppzmq"
	"${CMAKE_SOURCE_DIR}/submodules/boost-predef/include"
	"${CMAKE_SOURCE_DIR}/submodules/simple-cpp-router/src"
)

find_library(LIBZMQ   zmq)

set(BOTH_LINK_LIBRARIES
	${LIBZMQ}
)

#### MAIN ####
add_library(dunamis-module
	$<TARGET_OBJECTS:ROUTING>
	$<TARGET_OBJECTS:SOCKETER>
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main/uri/uri.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main/uri/urirouter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main/module.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main/logger.cpp"
)
set_target_properties(dunamis-module
	PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(dunamis-module
	${BOTH_LINK_LIBRARIES}
)

add_library(dunamis-spine
	$<TARGET_OBJECTS:ROUTING>
	$<TARGET_OBJECTS:SPINE>
)
set_target_properties(dunamis-spine
	PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(dunamis-spine
	${BOTH_LINK_LIBRARIES}
	${BOOSTFS}
	${BOOSTSYS}
	dunamis-module
)

#### TESTS ####
include("${CMAKE_CURRENT_SOURCE_DIR}/src/tests/tests.cmake")

#### MODULES ####
IF(DEFAULT_MODULES)
	add_library(mainline_config SHARED
		${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/config.cpp
	)
	set_target_properties(mainline_config
		PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
	)
	target_link_libraries(mainline_config
		${BOTH_LINK_LIBRARIES}
		${BOOSTFS}
		${BOOSTSYS}
		dunamis-module
	)

	add_library(mainline_output SHARED
		${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/output.cpp
	)
	set_target_properties(mainline_output
		PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
	)
	target_link_libraries(mainline_output
		${BOTH_LINK_LIBRARIES}
		dunamis-module
	)

	add_library(mainline_input SHARED
		${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/input.cpp
	)
	set_target_properties(mainline_input
		PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
	)
	target_link_libraries(mainline_input
		${BOTH_LINK_LIBRARIES}
		dunamis-module
	)

	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/submodules/mongoose")
	add_library(mainline_webui SHARED
		${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/webui.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/submodules/mongoose/mongoose.c
	)
	set_target_properties(mainline_webui
		PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
	)
	target_link_libraries(mainline_webui
		${BOTH_LINK_LIBRARIES}
		dunamis-module
	)
ENDIF(DEFAULT_MODULES)
