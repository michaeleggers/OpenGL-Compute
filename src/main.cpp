#include <stdio.h>
#include <stdlib.h>

#include <string>

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "platform.h"
#include "r_main.h"
#include "r_shader.h"

struct ViewProjectionMatrices {
	glm::mat4 view;
	glm::mat4 proj;
};


// Render Globals

static GLuint g_vertexVAOs[2];
static GLuint g_vertexVBO;
static GLuint g_angleBuffers[2];

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

	r_Init();

	// Load shaders

	Shader vertFragShaders{};
	if (!vertFragShaders.Load("vertex_shader.glsl", "fragment_shader.glsl")) {
		printf("Not able to load standard shader.\n");
		exit(-1);
	}

	// Create Buffers

	glGenVertexArrays(2, g_vertexVAOs);

	// Buffer for geometry

	static const Vertex vertexAttributes[] = {
		{ glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ glm::vec3(0.0f, 0.5f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ glm::vec3(0.5f, -0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) }
	};

	glGenBuffers(1, &g_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 1024 * 1024 * 1024, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(Vertex), vertexAttributes);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create Shader Storage Buffers (SSBOs) that we read/write from/to

	glGenBuffers(2, g_angleBuffers);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_angleBuffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 100 * sizeof(float), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_angleBuffers[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 100 * sizeof(float), NULL, GL_DYNAMIC_COPY);

	for (int i = 0; i < 2; i++) {
		glBindVertexArray(g_vertexVAOs[i]);	

		glBindBuffer(GL_ARRAY_BUFFER, g_vertexVBO);

		// Vertex Layout standard vertex geometry

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
		glEnableVertexAttribArray(1);

		// Vertex Layout SSBOs

		glBindBuffer(GL_ARRAY_BUFFER, g_angleBuffers[i]);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), NULL);		
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	
	// View Projection Data
	ViewProjectionMatrices viewProjUniform{};

	while (!glfwWindowShouldClose( r_GetWindow() )) {

		glfwPollEvents();

		viewProjUniform.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		viewProjUniform.proj = glm::perspective(glm::radians(70.0f), (float)r_WindowWidth() / (float)r_WindowHeight(), 1.0f, 100.0f);

		glClearColor(0.3f, 0.2f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vertFragShaders.Activate();
		vertFragShaders.SetViewProjMatrices(viewProjUniform.view, viewProjUniform.proj);
		glBindVertexArray(g_vertexVAOs[0]);		
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers( r_GetWindow() );

	}
	
	r_Shutdown();

}

