#### TESTS ####

set(TEST_OUTPUT_DIRS
	LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}/tests/modules"
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}/tests"
)
set(TEST_LINK_LIBRARIES
	-lzmq
	-lpthread
	-ldl
	-lboost_system
	-lboost_filesystem
)
set(TEST_FILES
    SpineAPI
    CreateSpineLogger
    ModuleInterface
)
set(TEST_COMMON_INCLUDE_DIRS
)
set(TEST_COMMON_COMPILE_FILES
	submodules/jsoncpp/dist/jsoncpp.cpp
)
set(TEST_EXTRA_INCLUDE_DIRS
	src
)
set(TEST_EXTRA_COMPILE_FILES
	"SpineAPI\;src/main/spine.cpp"
	"CreateSpineLogger\;src/main/spine.cpp"
	"ModuleInterface\;src/tests/ModuleInterfaceFixture.cpp"
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