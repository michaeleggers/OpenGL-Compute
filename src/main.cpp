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

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

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
	int   maxDepth;
};

// Render Globals

static GLuint g_vertexVAOs[2];
static GLuint g_branchBuffers[2];
static GLuint g_vertexBuffers[2];
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
	//uint32_t branchIndex = 0;
	for (Branch& branch : branches) {		
		//branch.end.branchIndex = branchIndex;
		vertices.push_back(branch.start);
		vertices.push_back(branch.end);
		//branchIndex++;
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

	glGenBuffers(2, g_branchBuffers);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_branchBuffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, computeData.size() * sizeof(BranchComputeData), &computeData[0], GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_branchBuffers[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, computeData.size() * sizeof(BranchComputeData), &computeData[0], GL_DYNAMIC_COPY);

	glGenBuffers(2, g_vertexBuffers);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_vertexBuffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_vertexBuffers[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_COPY);

	// Create Uniform Buffer for Compute Shader

	glGenBuffers(1, &g_computeShaderUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, g_computeShaderUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ComputeShaderData), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	for (int i = 0; i < 2; i++) {
		glBindVertexArray(g_vertexVAOs[i]);

		//glBindBuffer(GL_ARRAY_BUFFER, g_vertexVBO);

		//// Vertex Layout standard vertex geometry

		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		//glEnableVertexAttribArray(0);

		//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
		//glEnableVertexAttribArray(1);

		//glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec3) + sizeof(glm::vec4)));
		//glEnableVertexAttribArray(2);

		//Layout SSBOs

		glBindBuffer(GL_ARRAY_BUFFER, g_vertexBuffers[i]);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec4));
		glEnableVertexAttribArray(1);

		glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec4) + sizeof(glm::vec4)));
		glEnableVertexAttribArray(2);
	}
	glBindVertexArray(0);
}

int main(int argc, char** argv) {

	SetupDirectories(argc, argv);

	// Renderer Init

	r_Init();

	// ImGUI Init

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(r_GetWindow(), true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

	// Load shaders

	Shader vertFragShaders{};
	if (!vertFragShaders.Load("vertex_shader.glsl", "fragment_shader.glsl")) {
		printf("Not able to load standard shader.\n");
		exit(-1);
	}

	Shader orientBranchesComputeShader{};
	if (!orientBranchesComputeShader.Load("orient_branches_compute_shader.glsl")) {
		printf("Not able to load compute shader.\n");
		exit(-1);
	}

	Shader buildTreeComputeShader{};
	if (!buildTreeComputeShader.Load("build_tree_compute_shader.glsl")) {
		printf("Not able to load build tree compute shader\n");
		exit(-1);
	}

	// Create geometry and upload to GPU

	int treeDepth = 20;
	std::vector<Branch> tree = CreateTree(glm::vec3(0.0f), glm::vec3(0.0f, 20.0f, 0.0f), 20.0f, treeDepth);	
	InitBuffers(tree);
	printf("# Branches: %d\n# Vertices: %d\n", tree.size(), tree.size() * 2);

	// View Projection Data
	ViewProjectionMatrices viewProjUniform{};

	// Compute Shader Data
	ComputeShaderData computeShaderData{};
	computeShaderData.deltaTime = 0.001f;
	computeShaderData.totalTime = 0.0f;
	computeShaderData.numBranches = tree.size();
	computeShaderData.maxDepth = treeDepth;

	uint64_t frameIndex = 0;


	// Toggle VSYNC
	glfwSwapInterval(0);

	double deltaTimeMs = 0.0;
	double totalTimeMs = 0.0;
	double updateFreqMs = 1000.0;
	while (!glfwWindowShouldClose( r_GetWindow() )) {

		double startTime = glfwGetTime();

		glfwPollEvents();


		// Compute Stage: Orient branches

		computeShaderData.deltaTime = (float)deltaTimeMs;
		computeShaderData.totalTime += (float)deltaTimeMs;

		orientBranchesComputeShader.Activate();
		
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_branchBuffers[frameIndex]);     // read
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_branchBuffers[frameIndex ^ 1]); // write
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, g_computeShaderUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ComputeShaderData), &computeShaderData);		
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glDispatchCompute( (tree.size() + 255) / 256, 1, 1);		

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);		

		// Second Compute Stage: Build new tree
		
		buildTreeComputeShader.Activate();


		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_branchBuffers[frameIndex]);     // read
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_vertexBuffers[frameIndex]);		// read
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_vertexBuffers[frameIndex ^ 1]); // write
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, g_computeShaderUBO);

		glDispatchCompute( (tree.size() + 255) / 256, 1, 1);

		glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);		

		// Grtaphics Stage

		// Start new ImGUI window

		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::ShowDemoWindow(); // Show demo window! :)
		}
	
		//glViewport(0, 0, r_WindowWidth(), r_WindowWidth());
		glClearColor(0.3f, 0.2f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		viewProjUniform.view = glm::lookAt(glm::vec3(0.0f, 60.0f, 70.0f), glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		viewProjUniform.proj = glm::perspective(glm::radians(70.0f), (float)r_WindowWidth() / (float)r_WindowHeight(), 1.0f, 1000.0f);

		vertFragShaders.Activate();
		vertFragShaders.SetViewProjMatrices(viewProjUniform.view, viewProjUniform.proj);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_branchBuffers[frameIndex]);
		glBindVertexArray(g_vertexVAOs[frameIndex]);		
		glLineWidth(1.0f);
		glPointSize(9.0f);
		glDrawArrays(GL_LINES, 0, 2*tree.size());

		// Render ImGUI window

		{
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());		
		}

		glfwSwapBuffers( r_GetWindow() );

		double endTime = glfwGetTime();
		deltaTimeMs = (endTime - startTime) * 1000.0;
		totalTimeMs += deltaTimeMs;

		if (totalTimeMs > updateFreqMs) {
			char windowTitleBuf[256] = { 0 };
			float fps = 1000.0f / deltaTimeMs;
			sprintf(windowTitleBuf, "Tree Test -- delta time: %f ms -- FPS: %f\n", deltaTimeMs, fps);
			glfwSetWindowTitle(r_GetWindow(), windowTitleBuf);
			totalTimeMs = 0.0;
		}

		frameIndex ^= 1; // Swap the frame index ( = swap the buffers used for reading/writing)
		
	}
	
	r_Shutdown();

}

