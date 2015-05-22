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
macro(createExtrasList name output mappings)
    foreach(files ${${mappings}})
        list(GET files 0 ${name}_mapping)
        if ("${name}" STREQUAL "${${name}_mapping}")
            foreach(file ${files})
                if (NOT "${name}" STREQUAL "${file}")
                    if ("${${name}_${output}}" STREQUAL "")
                        set(${name}_${output} "${file}")
                    else()
                        set(${name}_${output} "${${name}_${output}} ${file}")
                    endif()
                endif()
            endforeach(file)
        endif()
    endforeach(files)
    if (NOT "${${name}_${output}}" STREQUAL "")
        message(STATUS "Included for module ${name}: ${${name}_${output}}")
    endif()
endmacro(createExtrasList)


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
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(main
    ${BOTH_LINK_LIBRARIES}
    ${MAIN_LINK_LIBRARIES}
)
#add_dependencies(main clean)

#### MODULES ####
set(MODULES
    core
    settings
)
# This contains escaped lists of extra files to build for you modules
# Lists are \; seperated
# The first item in each list should be your module name
set(MODULES_EXTRA_FILES
    "core\;"
    "settings\;"
)
set(MODULES_EXTRA_LINK_LIBRARIES
    "core\;"
    "settings\;-lconfig++"
)
foreach(module ${MODULES})
    createExtrasList(${module} EXTRA_FILES MODULES_EXTRA_FILES)
    createExtrasList(${module} EXTRA_LINK_LIBRARIES MODULES_EXTRA_LINK_LIBRARIES)
	add_library(${module} SHARED
        src/main/module.hpp
		src/modules/${module}.hpp
		src/modules/${module}.cpp
        ${${module}_EXTRA_FILES}
	)
	set_target_properties(${module}
	    PROPERTIES
	    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}/modules"
	)
 	target_link_libraries(${module}
        ${BOTH_LINK_LIBRARIES}
        ${${module}_EXTRA_LINK_LIBRARIES}
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
    createExtrasList(${test} EXTRA_FILES TEST_EXTRA_FILES)
    add_executable(${test}
        src/tests/${test}.cpp
        ${${test}_EXTRA_FILES}
    )
    set_target_properties(${test}
        PROPERTIES
        ${TEST_OUTPUT_DIRS}
    )
    target_link_libraries(${test}
        ${TEST_LINK_LIBRARIES}
    )
endforeach(test)
