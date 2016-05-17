#### COMMON ####
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(flags "-O3 -std=c++11 -Wall -Wextra -Wno-unused-parameter")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags}"
		CACHE STRING "Flags used by the compiler during all build types." FORCE)
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${flags}"
		CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

include_directories("${CMAKE_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/generated")
include_directories(
    "${CMAKE_SOURCE_DIR}/submodules/cppevent/include"
	"${CMAKE_SOURCE_DIR}/submodules/spdlog/include"
	"${CMAKE_SOURCE_DIR}/submodules/catch/single_include"
	"${CMAKE_SOURCE_DIR}/submodules/cppzmq"
	"${CMAKE_SOURCE_DIR}/submodules/boost-predef/include"
)

find_library(LIBZMQ   zmq)
find_library(BOOSTFS  boost_filesystem-mt)
find_library(BOOSTSYS boost_system-mt)

set(BOTH_LINK_LIBRARIES
    ${LIBZMQ}
)

#### MAIN ####
add_library(cppmodules #STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/src/main/spine.cpp
	$<TARGET_OBJECTS:SOCKETER>
)

set_target_properties(cppmodules
	PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(cppmodules
    ${BOTH_LINK_LIBRARIES}
    ${BOOSTFS}
	${BOOSTSYS}
)

#### TESTS ####
include("${CMAKE_CURRENT_SOURCE_DIR}/src/tests/tests.cmake")

#### MODULES ####
add_library(mainline_config SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/config.cpp
    $<TARGET_OBJECTS:SOCKETER>
)
set_target_properties(mainline_config
	PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
)
target_link_libraries(mainline_config
	${BOTH_LINK_LIBRARIES}
    ${BOOSTFS}
	${BOOSTSYS}
)

add_library(mainline_output SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/output.cpp
	$<TARGET_OBJECTS:SOCKETER>
)
set_target_properties(mainline_output
	PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
)
target_link_libraries(mainline_output
    ${BOTH_LINK_LIBRARIES}
)
