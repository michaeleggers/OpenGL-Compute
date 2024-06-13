cmake_minimum_required(VERSION 3.10)
project(ComputeShader VERSION 1.0)

ADD_SRC_HEADER_FILES()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm
)

add_executable(ComputeShader
	${SRC_FILES}
	${HEADER_FILES}
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/src/gl.c
	# ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/KHR/khrplatform.h
	# ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/glad/glad.h
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb/stb_image.h
)
	
target_include_directories(ComputeShader
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include
)

target_link_libraries(ComputeShader
	PUBLIC glfw
)