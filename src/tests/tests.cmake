#### TESTS ####

set(TEST_OUTPUT_DIRS
	LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}/tests/modules"
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}/tests"
)
set(TEST_LINK_LIBRARIES
	dunamis-module
	dunamis-spine
	-lzmq
	-lpthread
	-ldl
	-lboost_system-mt
	-lboost_filesystem-mt
)
set(TEST_FILES
	SpineAPI
	ModuleInterface
	ModuleChain
	CommandMessage
)
set(TEST_COMMON_INCLUDE_DIRS
	"${CMAKE_SOURCE_DIR}/src"
)
set(TEST_EXTRA_INCLUDE_DIRS
)
set(TEST_EXTRA_COMPILE_FILES
	"SpineAPI\;$<TARGET_OBJECTS:SPINE>"
	"Logger\;${CMAKE_SOURCE_DIR}/src/main/logger.cpp"
	"ModuleInterface\;"
	"ModuleChain\;"
	"CommandMessage\;"
)

IF(TESTS)
	foreach(test ${TEST_FILES})
		createExtrasList(${test} EXTRA_COMPILE_FILES TEST_EXTRA_COMPILE_FILES)
		createExtrasList(${test} EXTRA_INCLUDE_DIRS TEST_EXTRA_INCLUDE_DIRS)
		add_executable(${test}
			${TEST_COMMON_COMPILE_FILES}
			${${test}_EXTRA_COMPILE_FILES}
			src/tests/${test}.cpp
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

		add_test(TEST_${test} "${OUTPUT_DIR}/tests/${test}")
	endforeach(test)
ENDIF(TESTS)
