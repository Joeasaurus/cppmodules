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
	"${CMAKE_SOURCE_DIR}/submodules/cppevent/include"
    "${CMAKE_SOURCE_DIR}/submodules/dlfcn-win32"
	"${CMAKE_SOURCE_DIR}/submodules/spdlog/include"
	"${CMAKE_SOURCE_DIR}/submodules/catch/single_include"
	"${CMAKE_SOURCE_DIR}/submodules/cppzmq"
	"${CMAKE_SOURCE_DIR}/submodules/boost-predef/include"

)
link_directories(
    "${CMAKE_SOURCE_DIR}/submodules/dlfcn-win32/visual-studio/12/x64/Release"
    "C:/Program Files/ZeroMQ 4.0.4/lib"
    "C:/boost/lib"
)

set(LIBZMQ   libzmq-v120-mt-4_0_4)
set(BOOSTFS  libboost_filesystem-vc120-mt-s-1_60)
set(BOOSTSYS libboost_system-vc120-mt-s-1_60)
set(LIBDLFCN dl)

set(BOTH_LINK_LIBRARIES
	${LIBZMQ}
)

#### MAIN ####
add_library(dunamis-module
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/messages/socketer.cpp
)
set_target_properties(dunamis-module
	PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(dunamis-module
    ${BOTH_LINK_LIBRARIES}
)


add_library(dunamis-spine
	$<TARGET_OBJECTS:SPINE>
)
set_target_properties(dunamis-spine
	PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
target_link_libraries(dunamis-spine
    ${BOTH_LINK_LIBRARIES}
	${LIBDLFCN}
    ${BOOSTFS}
	${BOOSTSYS}
    dunamis-module
)

##### MODULES ####
IF(DEFAULT_MODULES)
    add_library(mainline_config SHARED
    	${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/config.cpp
    )
    set_target_properties(mainline_config
    	PROPERTIES
    	LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
    )
    target_link_libraries(mainline_config
    	${BOTH_LINK_LIBRARIES}
        ${BOOSTFS}
    	${BOOSTSYS}
        dunamis-module
    )

    add_library(mainline_output SHARED
    	${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/output.cpp
    )
    set_target_properties(mainline_output
    	PROPERTIES
    	LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
    )
    target_link_libraries(mainline_output
        ${BOTH_LINK_LIBRARIES}
        dunamis-module
    )

	add_library(mainline_input SHARED
    	${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/input.cpp
    )
    set_target_properties(mainline_input
    	PROPERTIES
    	LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
    )
    target_link_libraries(mainline_input
        ${BOTH_LINK_LIBRARIES}
        dunamis-module
    )

	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/submodules/mongoose")
	add_library(mainline_webui SHARED
    	${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/webui.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/submodules/mongoose/mongoose.c
    )
    set_target_properties(mainline_webui
    	PROPERTIES
    	LIBRARY_OUTPUT_DIRECTORY ${MODULES_LOCATION}
    )
    target_link_libraries(mainline_webui
        ${BOTH_LINK_LIBRARIES}
        dunamis-module
    )
ENDIF(DEFAULT_MODULES)
