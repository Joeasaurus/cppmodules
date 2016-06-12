cmake_minimum_required(VERSION 3.2)
project(cppmodules)
enable_testing()
set(CMAKE_VERBOSE_MAKEFILE false)


#### SET YOUR BINARY NAME HERE ####
set(BINARY_NAME "example")


set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build")
set(MODULES_LOCATION "${OUTPUT_DIR}/modules")
include("${CMAKE_SOURCE_DIR}/functions.cmake")

add_library(SPINE OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/spine.cpp
)
add_library(SOCKETER OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/messages/socketer.cpp
)

include("${CMAKE_SOURCE_DIR}/${CMAKE_SYSTEM_NAME}.cmake")


add_executable(${BINARY_NAME}
    $<TARGET_OBJECTS:SPINE>
	$<TARGET_OBJECTS:SOCKETER>
	${CMAKE_CURRENT_SOURCE_DIR}/src/mainfile/mainfile.cpp
)
set_target_properties(${BINARY_NAME}
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(${BINARY_NAME}
    ${BOTH_LINK_LIBRARIES}
    dunamis-spine
)

install(TARGETS ${BINARY_NAME} dunamis-spine dunamis-module
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
)



## cppcheck --force --inline-suppr --template '{file}:{line},{severity},{id},{message}' \
##  --xml --enable=all --max-configs=25 -i /Users/jme/GDrive/code/cppmodules/src ./ 2> err.xml
## cppcheck-htmlreport --file=err.xml --title=CppModules --report-dir=cppcheck_reports --source-dir=.
## valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes -v cppmodules
