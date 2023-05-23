set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)

function(WestDependency_Declare west_project_name)
	list(APPEND west_dependencies ${west_project_name})
	list(REMOVE_DUPLICATES west_dependencies)
	set(west_dependencies ${west_dependencies} CACHE STRING "List of west dependencies")
	message(DEBUG "west_dependencies: ${west_dependencies}")
endfunction()

# Both FIND_PACKAGE and FETCHCONTENT_MAKEAVAILABLE_SERIAL methods provide
# the package or dependency name as the first method-specific argument.
macro(west_provide_dependency method packet)
	list(FIND west_dependencies ${packet} IS_PACKET_WEST_DEPENDENCY)
	if(NOT ${IS_PACKET_WEST_DEPENDENCY} EQUAL -1)
		message(STATUS "Getting ${packet} using west")

		# Zephyr project sets up WEST variable and performs a search for a west tool in the system
		if(NOT WEST)
			message(FATAL_ERROR "West tool is not installed, please install.")
		endif()

		execute_process(COMMAND west update ${packet}
		RESULT_VARIABLE west_update_result
		OUTPUT_VARIABLE west_update_stdout
		ERROR_VARIABLE west_update_strerr)

		message(STATUS "${west_update_stdout}")
		message(STATUS "${west_update_strerr}")

		# Tell the caller we fulfilled the request
		if("${method}" STREQUAL "FIND_PACKAGE")
			# We need to set this if we got here from a find_package() call
			# since we used a different method to fulfill the request.
			set(${packet}_FOUND TRUE)
			message(STATUS "Selected FIND_PACKAGE")
		elseif("${method}" STREQUAL "FETCHCONTENT_MAKEAVAILABLE_SERIAL")
			# We used the same method, but were given a different name to the
			# one we populated with. Tell the caller about the name it used.


			# This will forward to the built-in FetchContent implementation,
			# which detects a recursive call for the same thing and avoids calling
			# the provider again if dep_name is the same as the current call.
			FetchContent_GetProperties(${packet})
			if(NOT ${packet}_POPULATED)
				FetchContent_Populate(${packet})
			endif()
			set(PACKET_SOURCE_DIR ${${packet}_SOURCE_DIR})
			set(PACKET_BINARY_DIR ${${packet}_BINARY_DIR})
			add_subdirectory(${PACKET_SOURCE_DIR} ${PACKET_BINARY_DIR})

			message(STATUS "Selected FETCHCONTENT_MAKEAVAILABLE_SERIAL")
			message(STATUS "PACKET_SOURCE_DIR ${PACKET_SOURCE_DIR}")
			message(STATUS "PACKET_BINARY_DIR ${PACKET_BINARY_DIR}")

			FetchContent_SetPopulated(${packet}
				SOURCE_DIR "${PACKET_SOURCE_DIR}"
				BINARY_DIR "${PACKET_BINARY_DIR}"
			)

			unset(PACKET_SOURCE_DIR)
			unset(PACKET_BINARY_DIR)
		endif()
	endif()
endmacro()

cmake_language(
	SET_DEPENDENCY_PROVIDER west_provide_dependency
	SUPPORTED_METHODS
		FIND_PACKAGE
		FETCHCONTENT_MAKEAVAILABLE_SERIAL
)