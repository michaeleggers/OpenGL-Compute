#include "r_main.h"

#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

static RenderState g_RenderState;

void GLFW_ErrorCallback(int error, const char* description) {
	fprintf(stderr, "GLFW ERROR (%d): %s\n", error, description);
}

void r_Init()
{
	if (!glfwInit()) {
		printf("Could not init GLFW.\n");
		exit(-1);
	}

	glfwSetErrorCallback(GLFW_ErrorCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	g_RenderState.glfwWindow = glfwCreateWindow(1024, 768, "OpenGL Compute Shaders", NULL, NULL);
	if (!g_RenderState.glfwWindow) {
		printf("Could not create GLFW window.\n");
		exit(-1);
	}

	glfwMakeContextCurrent(g_RenderState.glfwWindow);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("Failed to load OpenGL Extensions using GLAD.\n");
		exit(-1);
	}

	g_RenderState.vendor = glGetString(GL_VENDOR);
	g_RenderState.renderer = glGetString(GL_RENDERER);
	printf("VENDOR: %s, DEVICE: %s\n", (char*)g_RenderState.vendor, (char*)g_RenderState.renderer);

	printf("Renderer initialized.\n");
}

void r_Shutdown()
{
	glfwDestroyWindow(g_RenderState.glfwWindow);
	glfwTerminate();

	printf("Renderer shut down.\n");
}

GLFWwindow* r_GetWindow()
{
	return g_RenderState.glfwWindow;
}
