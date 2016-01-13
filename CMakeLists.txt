cmake_minimum_required(VERSION 3.2)
project(cppmodules)
#find_package(PkgConfig)

set(CMAKE_VERBOSE_MAKEFILE false)

## cppcheck --force --inline-suppr --template '{file}:{line},{severity},{id},{message}' \
##  --xml --enable=all --max-configs=25 -i /Users/jme/GDrive/code/cppmodules/src ./ 2> err.xml
## cppcheck-htmlreport --file=err.xml --title=CppModules --report-dir=cppcheck_reports --source-dir=.

include("${CMAKE_SOURCE_DIR}/flags_and_functions.cmake")

### BUILD TARGETS ###
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    include("${CMAKE_SOURCE_DIR}/linux_darwin.cmake")
    include("${CMAKE_SOURCE_DIR}/linux_darwin_modules.cmake")
    #include("${CMAKE_CURRENT_SOURCE_DIR}/src/tests/tests.cmake")
endif()