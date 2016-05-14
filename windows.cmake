include("${CMAKE_SOURCE_DIR}/submodules/ucm/cmake/ucm.cmake")
ucm_set_runtime(STATIC)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -D_SCL_SECURE_NO_WARNINGS"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

#### COMMON ####
include_directories(AFTER "${CMAKE_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/generated")
include_directories(AFTER
    "C:/Program Files/ZeroMQ 4.0.4/include"
    "C:/boost/include/boost-1_60"
    "${CMAKE_SOURCE_DIR}/submodules/dlfcn-win32"
	"${CMAKE_SOURCE_DIR}/submodules/jsoncpp/dist"
	"${CMAKE_SOURCE_DIR}/submodules/spdlog/include"
	"${CMAKE_SOURCE_DIR}/submodules/catch/single_include"
	"${CMAKE_SOURCE_DIR}/submodules/cppzmq"
	"${CMAKE_SOURCE_DIR}/submodules/boost-predef/include"
    "${CMAKE_SOURCE_DIR}/submodules/cppevent/include"

)
link_directories(
    "${CMAKE_SOURCE_DIR}/submodules/dlfcn-win32/visual-studio/12/x64/Release"
    "C:/Program Files/ZeroMQ 4.0.4/lib"
    "C:/boost/lib"
)

set(BOTH_LINK_LIBRARIES
	libzmq-v120-mt-4_0_4
)

#### PER SYSTEM ####
set(MAIN_LINK_LIBRARIES
    dl
	libboost_filesystem-vc120-mt-s-1_60
	libboost_system-vc120-mt-s-1_60
)

#### MAIN ####
add_executable(cppmodules
	src/main/main.cpp
	src/main/spine.cpp
	${BOTH_COMPILE_FILES}
)
configure_file("${CMAKE_SOURCE_DIR}/src/main/spine.hpp.in" "${CMAKE_BINARY_DIR}/generated/main/spine.hpp")
set_target_properties(cppmodules
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(cppmodules
	${BOTH_LINK_LIBRARIES}
	${MAIN_LINK_LIBRARIES}
)
