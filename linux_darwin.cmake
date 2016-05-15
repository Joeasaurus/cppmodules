#### COMMON ####
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(flags "-std=c++11 -Wall -Wextra -g -Wno-unused-parameter")

	set(CMAKE_CXX_FLAGS "${flags}"
		CACHE STRING "Flags used by the compiler during all build types." FORCE)
	set(CMAKE_C_FLAGS   "${flags}"
		CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

include_directories(AFTER "${CMAKE_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/generated")
include_directories(AFTER
	"${CMAKE_SOURCE_DIR}/submodules/spdlog/include"
	"${CMAKE_SOURCE_DIR}/submodules/catch/single_include"
	"${CMAKE_SOURCE_DIR}/submodules/cppzmq"
	"${CMAKE_SOURCE_DIR}/submodules/boost-predef/include"
)

set(BOTH_LINK_LIBRARIES
	-lzmq
)
set(BOTH_COMPILE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/messages/socketer.cpp
)

#### PER SYSTEM ####
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set(MAIN_LINK_LIBRARIES
		-ldl
		-lpthread
		-lboost_filesystem
		-lboost_system
	)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
	set(MAIN_LINK_LIBRARIES
		-lboost_filesystem-mt
		-lboost_system-mt
	)
endif()

#### MAIN ####
add_executable(cppmodules
	${CMAKE_CURRENT_SOURCE_DIR}/src/main/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/main/spine.cpp
	${BOTH_COMPILE_FILES}
)
configure_file("src/main/spine.hpp.in" "${CMAKE_BINARY_DIR}/generated/main/spine.hpp")
set_target_properties(cppmodules
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(cppmodules
	${BOTH_LINK_LIBRARIES}
	${MAIN_LINK_LIBRARIES}
)
