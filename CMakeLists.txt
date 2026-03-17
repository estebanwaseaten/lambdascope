cmake_minimum_required(VERSION 3.0.0)

#PROJECT_SOURCE_DIR	--> dir of most recent project() call
#CMAKE_SOURCE_DIR --> top level CMakeLists.txt
#CMAKE_CURRENT_SOURCE_DIR --> current CMakeLists.txt (add_subdirectory() call) should be similar to PROJECT_SOURCE_DIR

project(lambdascope)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-pg ) #-Wall -Wextra

set(SOURCE_LIST
	"src/lambdascope.cpp"
	"src/controller.cpp"
	#"src/channel.cpp"
	#"src/scope.cpp"
	#"src/adc.cpp"
	#"src/adc_MCP3201.cpp"
	#"src/adc_dummy.cpp"
	"src/utilities.cpp"
	"src/custom_elements.cpp"
	#"src/utilities.cpp"
)

set(LIBRARIES_LIST
	#${PROJECT_SOURCE_DIR}/libs/threader/					#first build threader. but terminalio should also build threader - to be safe		---> we should not do that... or put the binaries somewhere else`?
	${PROJECT_SOURCE_DIR}/libs/terminalio/
	${PROJECT_SOURCE_DIR}/libs/canvas/
	${PROJECT_SOURCE_DIR}/libs/raspi/
	#${PROJECT_SOURCE_DIR}/libs/devicetree/
)

# if libs have to be compiled --> calls CMakeLists.txt in those dirs: ???
foreach(LIBRARY ${LIBRARIES_LIST} )
	add_subdirectory(${LIBRARY} ${LIBRARY}/build)				#  # first ${LIBRARY} for source dir, second for binary dir (i.e. the dir where all the output files are put, but not the library binary)
endforeach()




#defines the target:
add_executable(${PROJECT_NAME} ${SOURCE_LIST})

set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/src
		${PROJECT_SOURCE_DIR}/libs/threader/include
		${PROJECT_SOURCE_DIR}/libs/terminalio/include
		${PROJECT_SOURCE_DIR}/libs/canvas/include
		${PROJECT_SOURCE_DIR}/libs/raspi/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC threader)				# link against threader
target_link_libraries(${PROJECT_NAME} PUBLIC terminalio)			# link against terminalio
target_link_libraries(${PROJECT_NAME} PUBLIC canvas)				# link against canvas
target_link_libraries(${PROJECT_NAME} PUBLIC devicetree)			# link against devicetree
target_link_libraries(${PROJECT_NAME} PUBLIC raspi)					# link against raspi

#target_link_libraries(${PROJECT_NAME} PUBLIC bcm_host)
