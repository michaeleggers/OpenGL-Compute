#include <stdio.h>
#include <stdlib.h>

#include <string>

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#include "platform.h"
#include "r_shader.h"

// Renderer Globals
static GLFWwindow* g_glfwWindow;
static const GLubyte* g_glVendor;
static const GLubyte* g_glRenderer;
static GLuint g_vertexVBO;
static GLuint g_vertexVAO;

struct ViewProjectionMatrices {
	glm::mat4 view;
	glm::mat4 proj;
};

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
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
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

	Shader vertFragShaders{};
	vertFragShaders.Load("vertex_shader.glsl", "fragment_shader.glsl");

	// Create Buffers

	glGenVertexArrays(1, &g_vertexVAO);
	glBindVertexArray(g_vertexVAO);
	glBindVertexArray(0);

	glGenBuffers(1, &g_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 1024 * 1024 * 1024, NULL, GL_STATIC_DRAW);
	static const glm::vec3 vertexPositions[] = {
		glm::vec3(-0.5f, -0.5f, 0.0f),
		glm::vec3(0.0f, 0.5f, 0.0f),
		glm::vec3(0.5f, -0.5f, 0.0f)
	};
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(glm::vec3), vertexPositions);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Global Render Settings

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	while (!glfwWindowShouldClose(g_glfwWindow)) {

		glfwPollEvents();

		glClearColor(0.3f, 0.2f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(g_vertexVAO);
		glUseProgram(vertFragShaders.Program());
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(g_glfwWindow);

	}

	glfwDestroyWindow(g_glfwWindow);
	glfwTerminate();
}

