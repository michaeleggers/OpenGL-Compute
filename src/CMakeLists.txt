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

	# dependencies/imgui/imgui.h
	# dependencies/imgui/imconfig.h
	# dependencies/imgui/imgui_internal.h
	# dependencies/imgui/imstb_rectpack.h
	# dependencies/imgui/imstb_textedit.h
	# dependencies/imgui/imstb_truetype.h

	# dependencies/imgui/misc/cpp/imgui_stdlib.h
	dependencies/imgui/misc/cpp/imgui_stdlib.cpp

	dependencies/imgui/imgui.cpp
	dependencies/imgui/imgui_draw.cpp
	dependencies/imgui/imgui_tables.cpp
	dependencies/imgui/imgui_widgets.cpp
	dependencies/imgui/imgui_demo.cpp

	dependencies/imgui/backends/imgui_impl_glfw.cpp
	dependencies/imgui/backends/imgui_impl_opengl3.cpp


	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/src/gl.c
	# ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/KHR/khrplatform.h
	# ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/glad/glad.h
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb/stb_image.h
)
	
if(MSVC)
    target_compile_options(ComputeShader PRIVATE /W4)
else()
    target_compile_options(ComputeShader PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(ComputeShader
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui

)

target_link_libraries(ComputeShader
	PUBLIC glfw
)
