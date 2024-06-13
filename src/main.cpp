#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

void GLFW_ErrorCallback(int error, const char* description) {
	fprintf(stderr, "GLFW ERROR (%d): %s\n", error, description);
}

GLFWwindow* g_glfwWindow;

int main(int argc, char** argv) {

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


	while (!glfwWindowShouldClose(g_glfwWindow)) {

		glfwPollEvents();

		glClearColor(0.3f, 0.2f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(g_glfwWindow);

	}

	glfwDestroyWindow(g_glfwWindow);
	glfwTerminate();
}

