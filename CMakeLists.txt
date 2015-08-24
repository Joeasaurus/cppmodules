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
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	include_directories(AFTER "C:\\Boost\\include\\boost-1_58" "C:\\Program Files\\ZeroMQ 4.0.4\\include" "D:\\local_scratch\\dlfcn-win32")
	link_directories("C:\\Boost\\lib" "C:\\Program Files\\ZeroMQ 4.0.4\\lib" "D:\\local_scratch\\dlfcn-win32\\cmake-out\\" "D:\\local_scratch\\cppmodules\\src\\lib\\jsoncpp")
	set(BOTH_LINK_LIBRARIES_Debug
			jsoncpp-d.lib
			libzmq-v120-mt-gd-4_0_4.lib
		)
	set(MAIN_LINK_LIBRARIES_Debug
		dl.lib
		libboost_system-vc140-mt-sgd-1_58.lib
		libboost_filesystem-vc140-mt-sgd-1_58.lib
	)
	set(BOTH_LINK_LIBRARIES_Release
		jsoncpp.lib
		libzmq-v120-mt-4_0_4.lib
	)
	set(MAIN_LINK_LIBRARIES_Release
		dl.lib
		libboost_system-vc140-mt-s-1_58.lib
		libboost_filesystem-vc140-mt-s-1_58.lib
	)
	message(STATUS "${CMAKE_BUILD_TYPE} ${BOTH_LINK_LIBRARIES_${CMAKE_BUILD_TYPE}}")
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
set(mainline_EXTRA_COMPILE_FILES
	"starter\;"
	"config\;"
)
set(mainline_EXTRA_COPY_FILES
	"starter\;"
	"config\;main.cfg"
)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
	set(mainline_EXTRA_LINK_LIBRARIES_Release
		"starter\;"
		"config\;boost_filesystem-mt\;boost_system-mt"
	)
	set(mainline_EXTRA_LINK_LIBRARIES_Debug
		"starter\;"
		"config\;boost_filesystem-mt\;boost_system-mt"
	)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	set(mainline_EXTRA_LINK_LIBRARIES_Release
		"starter\;"
		"config\;libboost_system-vc140-mt-s-1_58.lib\;libboost_filesystem-vc140-mt-s-1_58.lib"
	)
	set(mainline_EXTRA_LINK_LIBRARIES_Debug
		"starter\;"
		"config\;libboost_system-vc140-mt-sgd-1_58.lib\;libboost_filesystem-vc140-mt-sgd-1_58.lib"
	)
endif()

OPTION (BUILD_SHARED_LIBS "Build shared libraries." ON)
foreach(group ${MODULES})
	list(GET group 0 MODULE_GROUP_NAME)
	createExtrasList(${MODULE_GROUP_NAME} GROUP_MODULES MODULES)
	foreach(module ${${MODULE_GROUP_NAME}_GROUP_MODULES})
		createExtrasList(${MODULE_GROUP_NAME}_${module} EXTRA_COMPILE_FILES
						${MODULE_GROUP_NAME}_EXTRA_COMPILE_FILES
		)
		createExtrasList(${MODULE_GROUP_NAME}_${module} EXTRA_LINK_LIBRARIES
						${MODULE_GROUP_NAME}_EXTRA_LINK_LIBRARIES_${CMAKE_BUILD_TYPE}
		)
		createExtrasList(${MODULE_GROUP_NAME}_${module} EXTRA_COPY_FILES
						${MODULE_GROUP_NAME}_EXTRA_COPY_FILES
		)
		if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/modules/${group}/${module}.hpp.in")
			configure_file("${CMAKE_CURRENT_SOURCE_DIR}/modules/${group}/${module}.hpp.in"
						   "${CMAKE_BINARY_DIR}/gen/modules/${group}/${module}.hpp"
			)
		endif()
		add_library(${MODULE_GROUP_NAME}_${module} SHARED
			${CMAKE_CURRENT_SOURCE_DIR}/src/modules/${MODULE_GROUP_NAME}/${module}.cpp
			${${MODULE_GROUP_NAME}_${module}_EXTRA_COMPILE_FILES}
			${BOTH_COMPILE_FILES}
		)
		set_target_properties(${MODULE_GROUP_NAME}_${module}
			PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
		)
		target_link_libraries(${MODULE_GROUP_NAME}_${module}
			${BOTH_LINK_LIBRARIES_${CMAKE_BUILD_TYPE}}
			${${MODULE_GROUP_NAME}_${module}_EXTRA_LINK_LIBRARIES}
		)
		foreach(file_to_copy ${${MODULE_GROUP_NAME}_${module}_EXTRA_COPY_FILES})
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
	-lboost_system-mt
	-lboost_filesystem-mt
)
set(TEST_FILES
    SpineAPI
    CreateSpineLogger
)
set(TEST_COMMON_INCLUDE_DIRS
)
set(TEST_COMMON_COMPILE_FILES
	submodules/jsoncpp/dist/jsoncpp.cpp
)
set(TEST_EXTRA_INCLUDE_DIRS
)
set(TEST_EXTRA_COMPILE_FILES
	"SpineAPI\;src/main/spine.cpp"
	"CreateSpineLogger\;src/main/spine.cpp"
)
foreach(test ${TEST_FILES})
	createExtrasList(${test} EXTRA_COMPILE_FILES TEST_EXTRA_COMPILE_FILES)
	createExtrasList(${test} EXTRA_INCLUDE_DIRS TEST_EXTRA_INCLUDE_DIRS)
	add_executable(${test}
		src/tests/${test}.cpp
		${TEST_COMMON_COMPILE_FILES}
		${${test}_EXTRA_COMPILE_FILES}
	)
	include_directories(AFTER
		${TEST_COMMON_INCLUDE_DIRS}
		${${test}_EXTRA_INCLUDE_DIRS}
	)
	set_target_properties(${test}
		PROPERTIES
		${TEST_OUTPUT_DIRS}
	)
	target_link_libraries(${test}
		${TEST_LINK_LIBRARIES}
	)
endforeach(test)


