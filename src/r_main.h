#ifndef _R_MAIN_H_
#define _R_MAIN_H_

#include <stdint.h>

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct RenderState {
	GLFWwindow* glfwWindow;
	const GLubyte* vendor;
	const GLubyte* renderer;
	int windowWidth;
	int windowHeight;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec4 color;
	uint32_t  branchIndex;
};

void GLFW_ErrorCallback(int error, const char* description);
void GLFW_FramebufferCallback(GLFWwindow* window, int width, int height);
void r_Init();
void r_Shutdown();
int  r_WindowWidth();
int  r_WindowHeight();
GLFWwindow* r_GetWindow();

#endif
