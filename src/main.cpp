#include <stdio.h>
#include <stdlib.h>

#include <string>

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#include "platform.h"
#include "r_shader.h"

// Renderer Globals
GLFWwindow* g_glfwWindow;
const GLubyte* g_glVendor;
const GLubyte* g_glRenderer;


void GLFW_ErrorCallback(int error, const char* description) {
	fprintf(stderr, "GLFW ERROR (%d): %s\n", error, description);
}

static void SetupDirectories(int argc, char** argv) {

	std::string assets_dir = "../../assets";
	std::string shaders_dir = "../../src/shaders";
	if (argc > 1) {
		assets_dir = argv[1];
	}
	if (argc > 2) {
		shaders_dir = argv[2];
	}

	init_directories(assets_dir.c_str(), shaders_dir.c_str());
}

int main(int argc, char** argv) {

	SetupDirectories(argc, argv);

	if (!glfwInit()) {
		printf("Could not init GLFW.\n");
		exit(-1);
	}

	glfwSetErrorCallback(GLFW_ErrorCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	g_glfwWindow = glfwCreateWindow(1024, 768, "OpenGL Compute Shaders", NULL, NULL);
	if (!g_glfwWindow) {
		printf("Could not create GLFW window.\n");
		exit(-1);
	}

	glfwMakeContextCurrent(g_glfwWindow);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("Failed to load OpenGL Extensions using GLAD.\n");
		exit(-1);
	}

	g_glVendor = glGetString(GL_VENDOR);
	g_glRenderer = glGetString(GL_RENDERER);
	printf("VENDOR: %s, DEVICE: %s\n", (char*)g_glVendor, (char*)g_glRenderer);

	// Load shaders

	Shader shaders{};
	shaders.Load("vertex_shader.glsl", "fragment_shader.glsl");

	while (!glfwWindowShouldClose(g_glfwWindow)) {

		glfwPollEvents();

		glClearColor(0.3f, 0.2f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(g_glfwWindow);

	}

	glfwDestroyWindow(g_glfwWindow);
	glfwTerminate();
}

