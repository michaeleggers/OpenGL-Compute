cmake_minimum_required(VERSION 3.10)
project(ComputeShader VERSION 1.0)

ADD_SRC_HEADER_FILES()

add_executable(ComputeShader
	main.cpp
	platform.h
	platform.cpp
	r_shader.h
	r_shader.cpp
	r_main.cpp
	r_main.h
	g_tree.h
	g_tree.cpp
	utils.h
	utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/src/gl.c
	# ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/KHR/khrplatform.h
	# ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/glad/glad.h
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb/stb_image.h
)
	
target_include_directories(ComputeShader
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm
)

target_link_libraries(ComputeShader
	PUBLIC glfw
)
