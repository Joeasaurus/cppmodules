add_library(mainline_core SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/core.cpp
)

target_link_libraries(mainline_core
    ${BOTH_LINK_LIBRARIES}
)