set(MODULE_COPY_FILES
    main.cfg
)

set(LINUX_MODULE_LINK_LIBRARIES
    -lconfig++
    -lboost_filesystem
    -lboost_system
)
set(OSX_MODULE_LINK_LIBRARIES
    -lconfig++
    -lboost_filesystem-mt
    -lboost_system-mt
)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(MODULE_LINK_LIBRARIES
         ${LINUX_MODULE_LINK_LIBRARIES}
    )
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(MODULE_LINK_LIBRARIES
         ${OSX_MODULE_LINK_LIBRARIES}
    )
endif()

add_library(mainline_config SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/src/modules/mainline/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/submodules/jsoncpp/dist/jsoncpp.cpp
)

target_link_libraries(mainline_config
    ${BOTH_LINK_LIBRARIES}
    ${MODULE_LINK_LIBRARIES}
)