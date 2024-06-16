#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
#include "g_tree.h"

struct ViewProjectionMatrices {
	glm::mat4 view;
	glm::mat4 proj;
};


struct ComputeShaderData {
	float deltaTime;
	float totalTime;
	int   numBranches;
};

// Render Globals

static GLuint g_vertexVAOs[2];
static GLuint g_angleBuffers[2];
static GLuint g_vertexVBO;
static GLuint g_computeShaderUBO;

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

void InitBuffers(std::vector<Branch>& branches) {

	// Create the VAO

	glGenVertexArrays(2, g_vertexVAOs);

	// Geometry Buffer

	std::vector<Vertex> vertices{};
	uint32_t branchIndex = 0;
	for (Branch& branch : branches) {
		branch.start.branchIndex = branchIndex;
		branch.end.branchIndex = branchIndex;
		vertices.push_back(branch.start);
		vertices.push_back(branch.end);
		branchIndex++;
	}

	glGenBuffers(1, &g_vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 1.5 * 1024 * 1024 * 1024, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), &vertices[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create Shader Storage Buffers (SSBOs) that we read/write from/to

	std::vector<BranchComputeData> computeData{};
	for (Branch& branch : branches) {
		computeData.push_back(branch.computeData);		
	}

	glGenBuffers(2, g_angleBuffers);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_angleBuffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, computeData.size() * sizeof(BranchComputeData), &computeData[0], GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_angleBuffers[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, computeData.size() * sizeof(BranchComputeData), &computeData[0], GL_DYNAMIC_COPY);

	// Create Uniform Buffer for Compute Shader

	glGenBuffers(1, &g_computeShaderUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, g_computeShaderUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ComputeShaderData), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	for (int i = 0; i < 2; i++) {
		glBindVertexArray(g_vertexVAOs[i]);

		glBindBuffer(GL_ARRAY_BUFFER, g_vertexVBO);

		// Vertex Layout standard vertex geometry

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
		glEnableVertexAttribArray(1);

		glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec3) + sizeof(glm::vec4)));
		glEnableVertexAttribArray(2);

		// Layout SSBOs (for now disabled as we use the SSBO as such)

		//glBindBuffer(GL_ARRAY_BUFFER, g_angleBuffers[i]);
		//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BranchComputeData), (GLvoid*)0);
		//glEnableVertexAttribArray(2);
	}
	glBindVertexArray(0);
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

	Shader computeShader{};
	if (!computeShader.Load("compute_shader.glsl")) {
		printf("Not able to load compute shader.\n");
		exit(-1);
	}

	// Create geometry and upload to GPU

	std::vector<Branch> tree = CreateTree(glm::vec3(0.0f), glm::vec3(0.0f, 20.0f, 0.0f), 20.0f, 10);	
	InitBuffers(tree);

	
	// View Projection Data
	ViewProjectionMatrices viewProjUniform{};

	// Compute Shader Data
	ComputeShaderData computeShaderData{};
	computeShaderData.deltaTime = 0.001f;
	computeShaderData.totalTime = 0.0f;
	computeShaderData.numBranches = tree.size();

	uint64_t frameIndex = 0;


	// Toggle VSYNC
	glfwSwapInterval(1);

	double deltaTimeMs = 0.0;
	double totalTimeMs = 0.0;
	double updateFreqMs = 1000.0;
	while (!glfwWindowShouldClose( r_GetWindow() )) {

		double startTime = glfwGetTime();

		glfwPollEvents();


		// Compute Stage

		computeShaderData.deltaTime = (float)deltaTimeMs;
		computeShaderData.totalTime += (float)deltaTimeMs;

		computeShader.Activate();

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_angleBuffers[frameIndex]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_angleBuffers[frameIndex ^ 1]);
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, g_computeShaderUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ComputeShaderData), &computeShaderData);		
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glDispatchCompute( (tree.size() + 255) / 256, 1, 1);
		//glDispatchCompute(1, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		//glViewport(0, 0, r_WindowWidth(), r_WindowWidth());
		glClearColor(0.3f, 0.2f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Graphics Stage

		viewProjUniform.view = glm::lookAt(glm::vec3(0.0f, 60.0f, 70.0f), glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		viewProjUniform.proj = glm::perspective(glm::radians(70.0f), (float)r_WindowWidth() / (float)r_WindowHeight(), 1.0f, 1000.0f);

		vertFragShaders.Activate();
		vertFragShaders.SetViewProjMatrices(viewProjUniform.view, viewProjUniform.proj);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_angleBuffers[frameIndex]);
		glBindVertexArray(g_vertexVAOs[frameIndex]);		
		glLineWidth(1.0f);
		glPointSize(9.0f);
		glDrawArrays(GL_LINES, 0, 2*tree.size());

		glfwSwapBuffers( r_GetWindow() );

		double endTime = glfwGetTime();
		deltaTimeMs = (endTime - startTime) * 1000.0;
		totalTimeMs += deltaTimeMs;

		if (totalTimeMs > updateFreqMs) {
			printf("delta time: %f\n", deltaTimeMs);
			totalTimeMs = 0.0;
		}

		frameIndex ^= 1; // Swap the frame index ( = swap the buffers used for reading/writing)
		
	}
	
	r_Shutdown();

}

