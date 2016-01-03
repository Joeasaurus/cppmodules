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