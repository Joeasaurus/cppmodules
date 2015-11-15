cmake_minimum_required(VERSION 3.2)
project(cppmodules)
#find_package(PkgConfig)

set(CMAKE_VERBOSE_MAKEFILE false)

## cppcheck --force --inline-suppr --template '{file}:{line},{severity},{id},{message}' \
##  --xml --enable=all --max-configs=25 -i /Users/jme/GDrive/code/cppmodules/src ./ 2> err.xml
## cppcheck-htmlreport --file=err.xml --title=CppModules --report-dir=cppcheck_reports --source-dir=.

### CMAKE FLAGS ###

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	if (NOT CONFIGURED_ONCE)
		set(CXX_DEFAULT_FLAGS "-std=c++0x")
	endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	if (NOT CONFIGURED_ONCE)
		set(CXX_DEFAULT_FLAGS "-std=c++11")
	endif()
endif()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
	("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
	if(NOT DEFINED CMAKE_BUILD_TYPE OR "${CMAKE_BUILD_TYPE}" STREQUAL "")
		set(CMAKE_BUILD_TYPE Release)
	endif()
	if (NOT CONFIGURED_ONCE)
		set(CXX_WARNING_FLAGS "-Wall -Wextra")
		set(CXX_SPDLOG_FLAGS "-DSPDLOG_NO_NAME -DSPDLOG_NO_THREAD_ID -DSPDLOG_NO_REGISTRY_MUTEX")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CXX_DEFAULT_FLAGS} ${CXX_WARNING_FLAGS} ${CXX_SPDLOG_FLAGS}")
		set(CMAKE_CXX_FLAGS_DEBUG "-O0 ${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS_RELEASE}")
	endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set (CMAKE_CONFIGURATION_TYPES Debug;Release)
	# _CRT_SECURE_NO_WARNINGS stops it complaining about ctime

	if (NOT CONFIGURED_ONCE)
		set(CMAKE_CXX_FLAGS "/W3 /WX /EHsc /sdl /Od /D _SCL_SECURE_NO_WARNINGS /D _CRT_SECURE_NO_WARNINGS")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:library")
	endif()
endif()
message(STATUS "Build Type : ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Flags: ${CMAKE_CXX_FLAGS_RELEASE}")

### FUNCTIONS & VARS ###

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build")
set(MODULES_LOCATION "${OUTPUT_DIR}/modules")
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
include_directories(AFTER
	"${CMAKE_SOURCE_DIR}/submodules/jsoncpp/dist"
	"${CMAKE_SOURCE_DIR}/submodules/spdlog/include"
	"${CMAKE_SOURCE_DIR}/submodules/catch/single_include"
	"${CMAKE_SOURCE_DIR}/submodules/cppzmq"
	"${CMAKE_SOURCE_DIR}/submodules/boost-predef/include"
)
set(BOTH_COMPILE_FILES
	submodules/jsoncpp/dist/jsoncpp.cpp
)
set(BOTH_LINK_LIBRARIES_Release
	-lzmq
)
set(BOTH_LINK_LIBRARIES_Debug ${BOTH_LINK_LIBRARIES_Release})
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set(MAIN_LINK_LIBRARIES_Release
		-ldl
		-lpthread
		-lboost_filesystem
		-lboost_system
	)
	set(MAIN_LINK_LIBRARIES_Debug ${MAIN_LINK_LIBRARIES_Release})
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
	set(MAIN_LINK_LIBRARIES_Release
		-lboost_filesystem-mt
		-lboost_system-mt
	)
	set(MAIN_LINK_LIBRARIES_Debug ${MAIN_LINK_LIBRARIES_Release})
endif()


#### MAIN ####

add_executable(cppmodules
	src/main/main.cpp
	src/main/spine.cpp
	${BOTH_COMPILE_FILES}
)
configure_file("src/main/spine.hpp.in" "${CMAKE_BINARY_DIR}/generated/main/spine.hpp")
set_target_properties(cppmodules
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(cppmodules
	${BOTH_LINK_LIBRARIES_${CMAKE_BUILD_TYPE}}
	${MAIN_LINK_LIBRARIES_${CMAKE_BUILD_TYPE}}
)


#### MODULES ####

set(MODULES
    "mainline\;config"
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

#include(${CMAKE_CURRENT_SOURCE_DIR}/src/tests/tests.cmake)