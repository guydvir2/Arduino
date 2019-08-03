# ArduinoJson - arduinojson.org
# Copyright Benoit Blanchon 2014-2019
# MIT License

<<<<<<< HEAD
add_executable(MiscTests 
	empty.cpp
	deprecated.cpp
=======
add_executable(MiscTests
	conflicts.cpp
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6
	FloatParts.cpp
	StreamReader.cpp
	StringAdapters.cpp
	StringWriter.cpp
	TypeTraits.cpp
	unsigned_char.cpp
	version.cpp
<<<<<<< HEAD
	vla.cpp
=======
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6
)

target_link_libraries(MiscTests catch)
add_test(Misc MiscTests)


add_executable(Issue978
	Issue978.cpp
)
set_target_properties(Issue978
	PROPERTIES
    	EXCLUDE_FROM_ALL TRUE
        EXCLUDE_FROM_DEFAULT_BUILD TRUE
)
add_test(
	NAME
		Issue978
    COMMAND
    	${CMAKE_COMMAND} --build . --target Issue978 --config $<CONFIGURATION>
    WORKING_DIRECTORY
    	${CMAKE_BINARY_DIR}
)
set_tests_properties(Issue978 
	PROPERTIES
		WILL_FAIL TRUE)
