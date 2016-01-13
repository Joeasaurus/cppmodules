### CMAKE FLAGS ###

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()
if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${warnings} -Os -s"
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

