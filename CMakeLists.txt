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
    set(CMAKE_CXX_FLAGS "-std=c++11 ${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

### FUNCTIONS & VARS ###
set(OUTPUT_DIR build)
function(createExtraFilesList name mappings)
    foreach(files ${mappings})
        foreach(file ${files})
            if (NOT "${name}" STREQUAL "${file}")
                set(${name}_EXTRA_FILES "${${name}_EXTRA_FILES} ${file}")
            endif()
        endforeach(file)
    endforeach(files)
    message(STATUS ${${name}_EXTRA_FILES})
endfunction(createExtraFilesList)


### BUILD TARGETS ###
#### COMMON ####
include_directories(src)
set(BOTH_LINK_LIBRARIES
    -lzmq
)
set(MAIN_LINK_LIBRARIES
    -lboost_filesystem
    -lboost_system
)
set(OUTPUT_DIRS
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}"
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
target_link_libraries(main
    ${BOTH_LINK_LIBRARIES}
    ${MAIN_LINK_LIBRARIES}
)
#add_dependencies(main clean)

#### MODULES ####
set(MODULES
	core
)
# This contains escaped lists of extra files to build for you modules
# The first item in each list should be your module name
set(MODULES_EXTRA_FILES
    "core\;"
)
foreach(module ${MODULES})
    createExtraFilesList(${module} ${MODULES_EXTRA_FILES})
	add_library(${module} SHARED
        src/main/module.hpp
		src/modules/${module}.hpp
		src/modules/${module}.cpp
        ${${module}_EXTRA_FILES}
	)
	set_target_properties(${module}
	    PROPERTIES
	    ${OUTPUT_DIRS}
	)
 	target_link_libraries(${module}
        ${BOTH_LINK_LIBRARIES}
    )
endforeach(module)

#### PHONY ####
option(CLEAN_OUTPUT "Clean the output directory (${OUTPUT_DIR}) before build" OFF)
if(CLEAN_OUTPUT)
    set(CLEAN_OUTPUT_ALL ALL)
endif(CLEAN_OUTPUT)
add_custom_target(clean-output ${CLEAN_OUTPUT_ALL}
                  rm -rf build && mkdir -p build/modules
                  COMMENT "Cleaning previous build" VERBATIM
)