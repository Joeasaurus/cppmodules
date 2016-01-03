cmake_minimum_required(VERSION 3.2)
project(cppmodules)
#find_package(PkgConfig)

set(CMAKE_VERBOSE_MAKEFILE false)

## cppcheck --force --inline-suppr --template '{file}:{line},{severity},{id},{message}' \
##  --xml --enable=all --max-configs=25 -i /Users/jme/GDrive/code/cppmodules/src ./ 2> err.xml
## cppcheck-htmlreport --file=err.xml --title=CppModules --report-dir=cppcheck_reports --source-dir=.

include("${CMAKE_SOURCE_DIR}/flags_and_functions.cmake")

### BUILD TARGETS ###

#### COMMON ####

include_directories(AFTER "${CMAKE_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/generated")
include_directories(AFTER
	"${CMAKE_SOURCE_DIR}/submodules/jsoncpp/dist"
	"${CMAKE_SOURCE_DIR}/submodules/spdlog/include"
	"${CMAKE_SOURCE_DIR}/submodules/catch/single_include"
	"${CMAKE_SOURCE_DIR}/submodules/cppzmq"
	"${CMAKE_SOURCE_DIR}/submodules/boost-predef/include"
    "${CMAKE_SOURCE_DIR}/submodules/cppevent/include"
)
set(BOTH_COMPILE_FILES
	submodules/jsoncpp/dist/jsoncpp.cpp
)
set(BOTH_LINK_LIBRARIES
	-lzmq
)
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
	${BOTH_LINK_LIBRARIES}
	${MAIN_LINK_LIBRARIES}
)


#### MODULES ####

set(MODULES
    "mainline\;config"
)

foreach(group ${MODULES})
    list(GET group 0 MODULE_GROUP_NAME)
    createExtrasList(${MODULE_GROUP_NAME} GROUP_MODULES MODULES)
    foreach(module ${${MODULE_GROUP_NAME}_GROUP_MODULES})
        include("${CMAKE_CURRENT_SOURCE_DIR}/src/modules/${MODULE_GROUP_NAME}_${module}.cmake")
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

include("${CMAKE_CURRENT_SOURCE_DIR}/src/tests/tests.cmake")