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
    set(CMAKE_CXX_FLAGS "-std=c++11 ${warnings} -g"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

### FUNCTIONS & VARS ###

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build")
set(MODULES_LOCATION "${OUTPUT_DIR}/modules")
message(STATUS "${CMAKE_VERSION}")
if (${CMAKE_VERSION} VERSION_GREATER 3.4)
    set(ZMQ_VOID_CONVERT "*")
endif() 
macro(createExtrasList name output mappings)
    foreach(files ${${mappings}})
        list(GET files 0 ${name}_mapping)
        string(FIND ${name} ${${name}_mapping} ${name}_mapping_sub)
        if (NOT "${${name}_mapping_sub}" STREQUAL "-1")
            foreach(file ${files})
                string(FIND ${name} ${file} ${name}_mapping_sub)
                if ("${${name}_mapping_sub}" STREQUAL "-1")
                    if ("${${name}_${output}}" STREQUAL "")
                        set(${name}_${output} "${file}")
                    else()
                        set(${name}_${output} "${${name}_${output}};${file}")
                    endif()
                endif()
            endforeach(file)
        endif()
    endforeach(files)
    if (NOT "${${name}_${output}}" STREQUAL "")
        message(STATUS "Included for ${name}: ${${name}_${output}}")
    endif()
endmacro(createExtrasList)


### BUILD TARGETS ###

#### COMMON ####

include_directories(AFTER "${CMAKE_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/generated")
set(BOTH_LINK_LIBRARIES
    -lzmq
)
set(LINUX_LINK_LIBRARIES
    -lboost_filesystem
    -lboost_system
    -ldl
    -lpthread
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
    src/main/spine.cpp
)
add_executable(main ${MAIN_SOURCE_FILES})
configure_file("src/main/spine.hpp.in" "${CMAKE_BINARY_DIR}/generated/main/spine.hpp")
set_target_properties(main
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(main
    ${BOTH_LINK_LIBRARIES}
    ${MAIN_LINK_LIBRARIES}
)


#### MODULES ####

set(MODULES
    "mainline\;core\;config"
)

foreach(group ${MODULES})
    list(GET group 0 MODULE_GROUP_NAME)
    createExtrasList(${MODULE_GROUP_NAME} GROUP_MODULES MODULES)
    foreach(module ${${MODULE_GROUP_NAME}_GROUP_MODULES})
        include(${CMAKE_CURRENT_SOURCE_DIR}/src/modules/${MODULE_GROUP_NAME}_${module}.cmake)
    	set_target_properties(${MODULE_GROUP_NAME}_${module}
    	    PROPERTIES
    	    LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
    	)
        foreach(file_to_copy ${MODULE_COPY_FILES})
                add_custom_command(
                    OUTPUT ${OUTPUT_DIR}/modules/${file_to_copy}
                    COMMAND ${CMAKE_COMMAND}
                        -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/${MODULE_GROUP_NAME}/${file_to_copy}
                        ${OUTPUT_DIR}/modules/${file_to_copy}
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/${MODULE_GROUP_NAME}/${file_to_copy}
                )
                add_custom_target(${MODULE_GROUP_NAME}_${module}_${file_to_copy} ALL
                    DEPENDS ${OUTPUT_DIR}/modules/${file_to_copy}
                )
                add_dependencies(${MODULE_GROUP_NAME}_${module}
                    ${MODULE_GROUP_NAME}_${module}_${file_to_copy}
                )
        endforeach(file_to_copy)
    endforeach(module)
endforeach(group)


#### TESTS ####

set(TEST_OUTPUT_DIRS
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}/tests/modules"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}/tests"
)
set(TEST_LINK_LIBRARIES
    -lzmq
)
set(TEST_FILES
    ModuleTests
)
set(TEST_EXTRA_COMPILE_FILES
    "ModuleTests\;"
)
foreach(test ${TEST_FILES})
    createExtrasList(${test} EXTRA_FILES TEST_EXTRA_COMPILE_FILES)
    add_executable(${test} EXCLUDE_FROM_ALL
        src/tests/${test}.cpp
        ${${test}_EXTRA_COMPILE_FILES}
    )
    set_target_properties(${test}
        PROPERTIES
        ${TEST_OUTPUT_DIRS}
    )
    target_link_libraries(${test}
        ${TEST_LINK_LIBRARIES}
    )
endforeach(test)
