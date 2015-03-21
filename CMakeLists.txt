cmake_minimum_required(VERSION 3.1)
project(cppmodules)

### CMAKE FLAGS ###
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()
if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

### FUNCTIONS & VARS ###
function(clean target)
	add_custom_command(TARGET ${target}
                       PRE_BUILD
                       COMMAND rm -rf build && mkdir -p build
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                       COMMENT "Cleaning previous build" VERBATIM)
endfunction()

### BUILD TARGETS ###
#### COMMON ####
include_directories(src)
set(MAIN_LINK_LIBRARIES
    -lboost_filesystem
    -lboost_system
    -lzmq
)
set(OUTPUT_DIRS
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build"
)

#### MAIN ####
set(MAIN_SOURCE_FILES
    src/main/main.cpp
    src/main/module.hpp
    src/main/spine.cpp
    src/main/spine.hpp
)
add_executable(main ${MAIN_SOURCE_FILES})
set_target_properties(main
    PROPERTIES
    ${OUTPUT_DIRS}
)
target_link_libraries(main ${MAIN_LINK_LIBRARIES})

#### MODULES ####
set(MODULES
	core
)
foreach(module ${MODULES})
	add_library(${module} SHARED
		src/main/module.hpp
		src/modules/${module}.hpp
		src/modules/${module}.cpp
	)
	set_target_properties(${module}
	    PROPERTIES
	    SUFFIX ".so"
	    ${OUTPUT_DIRS}
	)
 	target_link_libraries(${module} ${MAIN_LINK_LIBRARIES})
endforeach(module)
