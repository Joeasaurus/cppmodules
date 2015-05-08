cmake_minimum_required(VERSION 3.1)
project(cppmodules)

### CMAKE FLAGS ###
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra")
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
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build")
function(createExtraFilesList name mappings)
    foreach(files ${mappings})
        foreach(file ${files})
            if (NOT "${name}" STREQUAL "${file}")
                set(${name}_EXTRA_FILES "${file} ${${name}_EXTRA_FILES}")
            endif()
        endforeach(file)
    endforeach(files)
    if (NOT "${${name}_EXTRA_FILES}" STREQUAL "")
        message(STATUS "Included for module ${name}: ${${name}_EXTRA_FILES}")
    endif()
endfunction(createExtraFilesList)


### BUILD TARGETS ###
#### COMMON ####
include_directories("${CMAKE_SOURCE_DIR}/src")
set(BOTH_LINK_LIBRARIES
    -lzmq
)
set(LINUX_LINK_LIBRARIES
    -lboost_filesystem
    -lboost_system
    -ldl
)
set(OSX_LINK_LIBRARIES
    -lboost_filesystem-mt
    -lboost_system-mt
)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(MAIN_LINK_LIBRARIES
         ${LINUX_LINK_LIBRARIES}
    )
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(MAIN_LINK_LIBRARIES
         ${OSX_LINK_LIBRARIES}
    )
endif()
set(RUNTIME_OUTPUT_DIRS
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}/modules"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
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
    ${RUNTIME_OUTPUT_DIRS}
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
	    ${RUNTIME_OUTPUT_DIRS}
	)
 	target_link_libraries(${module}
        ${BOTH_LINK_LIBRARIES}
    )
endforeach(module)

#### TESTS ####
set(TEST_OUTPUT_DIRS
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}/tests/modules"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}/tests"
)
set(TEST_LINK_LIBRARIES
)
set(TEST_FILES
    ModuleTests
)
set(TEST_EXTRA_FILES
    "ModuleTests\;"
)
foreach(test ${TEST_FILES})
    createExtraFilesList(test-${test} ${TEST_EXTRA_FILES})
    add_executable(test-${test}
        src/tests/${test}.cpp
        ${test-${test}_EXTRA_FILES}
    )
    set_target_properties(test-${test}
        PROPERTIES
        ${TEST_OUTPUT_DIRS}
    )
    target_link_libraries(test-${test}
        ${TEST_LINK_LIBRARIES}
    )
endforeach(test)
