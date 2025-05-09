#define TAIWAN_SHIELD_IMPLEMENTATION
#include "taiwan.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "g_tree.h"
#include "platform.h"
#include "r_main.h"
#include "r_shader.h"

struct ViewProjectionMatrices
{
    glm::mat4 view;
    glm::mat4 proj;
};

struct ComputeShaderData
{
    float     deltaTime;
    float     totalTime;
    int       numBranches;
    int       maxDepth;
    glm::vec3 rotationAxis;
    float     padding;
    glm::vec3 strength; // x = strength, y, z unused for now
};

// Render Globals

static GLuint g_vertexVAOs[ 2 ];
static GLuint g_branchBuffers[ 2 ];
static GLuint g_vertexBuffers[ 2 ];
static GLuint g_computeShaderUBO;

static void SetupDirectories(const char* assets, const char* shaders)
{
    std::string assets_dir  = "../../assets";
    std::string shaders_dir = "../../src/shaders";
    if ( assets )
    {
        assets_dir = std::string(assets);
    }
    if ( shaders )
    {
        shaders_dir = std::string(shaders);
    }

    init_directories(assets_dir.c_str(), shaders_dir.c_str());
}

void CleanupBuffers()
{
    // Delete previously created buffers
    glDeleteVertexArrays(2, g_vertexVAOs);
    glDeleteBuffers(2, g_branchBuffers);
    glDeleteBuffers(2, g_vertexBuffers);
    glDeleteBuffers(1, &g_computeShaderUBO);

    // Make sure they are uninitialized. So OpenGL will tell you
    // if you try to bind them.

    g_vertexVAOs[ 0 ] = 0;
    g_vertexVAOs[ 1 ] = 0;

    g_branchBuffers[ 0 ] = 0;
    g_branchBuffers[ 1 ] = 0;

    g_vertexBuffers[ 0 ] = 0;
    g_vertexBuffers[ 1 ] = 0;

    g_computeShaderUBO = 0;
}

void GenerateGlBuffers()
{
    // Create the VAO

    glGenVertexArrays(2, g_vertexVAOs);

    // SSBOs

    glGenBuffers(2, g_branchBuffers);
    glGenBuffers(2, g_vertexBuffers);

    // Uniform Buffer

    glGenBuffers(1, &g_computeShaderUBO);
}

void InitBuffers(std::vector<Branch>& branches)
{

    // Separate vertices from branches

    std::vector<Vertex> vertices{};
    for ( Branch& branch : branches )
    {
        vertices.push_back(branch.start);
        vertices.push_back(branch.end);
    }

    // Fill Shader Storage Buffers (SSBOs) that we read/write from/to

    std::vector<BranchComputeData> computeData{};
    for ( Branch& branch : branches )
    {
        computeData.push_back(branch.computeData);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_branchBuffers[ 0 ]);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, computeData.size() * sizeof(BranchComputeData), &computeData[ 0 ], GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_branchBuffers[ 1 ]);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, computeData.size() * sizeof(BranchComputeData), &computeData[ 0 ], GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_vertexBuffers[ 0 ]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(Vertex), &vertices[ 0 ], GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_vertexBuffers[ 1 ]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(Vertex), &vertices[ 0 ], GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Create Uniform Buffer for Compute Shader

    glBindBuffer(GL_UNIFORM_BUFFER, g_computeShaderUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ComputeShaderData), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for ( int i = 0; i < 2; i++ )
    {
        glBindVertexArray(g_vertexVAOs[ i ]);

        //Layout SSBOs

        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBuffers[ i ]);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec4));
        glEnableVertexAttribArray(1);

        glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (GLvoid*)(sizeof(glm::vec4) + sizeof(glm::vec4)));
        glEnableVertexAttribArray(2);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

std::vector<Branch> RecreateTree(int depth)
{
    std::vector<Branch> tree = CreateTree(glm::vec3(0.0f), glm::vec3(0.0f, 20.0f, 0.0f), 20.0f, depth);
    printf("# Branches: %ld\n# Vertices: %ld\n", tree.size(), tree.size() * 2);

    return tree;
}

int main(int argc, char** argv)
{

    if ( argc < 2 )
    {
        printf("Usage:\n");
        printf("ComputeShader <rel-shaders-dir> <rel-assets-dir> <tree-depth>(optional, default: 10)\n\n");
        tw_PrintShield();
        exit(-1);
    }

    SetupDirectories(argv[ 1 ], argv[ 2 ]);

    int treeDepth = 10;
    if ( argc > 3 )
    {
        treeDepth = atoi(argv[ 3 ]);
    }
    if ( treeDepth < 0 )
    {
        printf("tree-depth must be at least 0.\n\n");
        exit(-1);
    }

    // Renderer Init

    r_Init();

    // ImGUI Init

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(
        r_GetWindow(),
        true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    // Load shaders

    Shader vertFragShaders{};
    if ( !vertFragShaders.Load("vertex_shader.glsl", "fragment_shader.glsl") )
    {
        printf("Not able to load standard shader.\n");
        exit(-1);
    }

    Shader orientBranchesComputeShader{};
    if ( !orientBranchesComputeShader.Load("orient_branches_compute_shader.glsl") )
    {
        printf("Not able to load compute shader.\n");
        exit(-1);
    }

    Shader buildTreeComputeShader{};
    if ( !buildTreeComputeShader.Load("build_tree_compute_shader.glsl") )
    {
        printf("Not able to load build tree compute shader\n");
        exit(-1);
    }

    // Create OpenGL VAOs, SSBOs and UBO

    GenerateGlBuffers();

    // Create geometry and upload to GPU

    std::vector<Branch> tree = RecreateTree(treeDepth);
    InitBuffers(tree);

    // View Projection Data
    ViewProjectionMatrices viewProjUniform{};

    // Compute Shader Data
    ComputeShaderData computeShaderData{};
    computeShaderData.deltaTime    = 0.001f;
    computeShaderData.totalTime    = 0.0f;
    computeShaderData.numBranches  = tree.size();
    computeShaderData.maxDepth     = treeDepth;
    computeShaderData.rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    computeShaderData.strength     = glm::vec3(10.0f, 0.0f, 0.0f);

    uint64_t frameIndex = 0;

    // Toggle VSYNC
    glfwSwapInterval(0);

    double    deltaTimeMs   = 0.0;
    double    totalTimeMs   = 0.0;
    double    updateFreqMs  = 1000.0;
    glm::vec3 windDirection = glm::vec3(0.0f, 0.0f, 1.0f);
    while ( !glfwWindowShouldClose(r_GetWindow()) )
    {

        double startTime = glfwGetTime();

        glfwPollEvents();

        // Compute Stage: Orient branches

        computeShaderData.deltaTime = (float)deltaTimeMs;
        computeShaderData.totalTime += (float)deltaTimeMs;

        orientBranchesComputeShader.Activate();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_branchBuffers[ 0 ]); // read
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_branchBuffers[ 1 ]); // write
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, g_computeShaderUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ComputeShaderData), &computeShaderData);
        //glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glDispatchCompute((tree.size() + 255) / 256, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Second Compute Stage: Build new tree

        buildTreeComputeShader.Activate();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_branchBuffers[ 1 ]);              // read
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_vertexBuffers[ frameIndex ]);     // read/write
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_vertexBuffers[ frameIndex ^ 1 ]); // write/read
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, g_computeShaderUBO);

        glDispatchCompute((tree.size() + 255) / 256, 1, 1);

        // Graphics Stage

        // Start new ImGUI window

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            //ImGui::ShowDemoWindow(); // Show demo window! :)

            int numVertices = computeShaderData.numBranches * 2;
            ImGui::Begin("Tree Test");
            ImGui::Text("Create new tree");
            ImGui::DragInt("Recursion Depth", &treeDepth, 0.05F, 0, 25);
            if ( ImGui::Button("RECREATE TREE") )
            {
                CleanupBuffers();
                GenerateGlBuffers();
                tree = RecreateTree(treeDepth);
                InitBuffers(tree);
                computeShaderData.maxDepth    = treeDepth;
                computeShaderData.numBranches = tree.size();
            }
            ImGui::Separator();
            ImGui::Text("# Branches: %d", computeShaderData.numBranches);
            ImGui::Text("# Vertices: %d (%f Mio)", numVertices, numVertices / 1000000.0f);
            ImGui::Text("Frametime (ms): %f", deltaTimeMs);
            ImGui::DragFloat3("wind direction", &windDirection[ 0 ], 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("Wind strength", &computeShaderData.strength.x, 1.0f, 0.0f, 2000.0f);
            // We need to pass the vector that is perpendicular to the wind direction because, that is the rotation axis
            glm::quat upOrientation        = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            computeShaderData.rotationAxis = glm::rotate(upOrientation, windDirection);

            ImGui::End();
        }

        glClearColor(0.3f, 0.2f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewProjUniform.view
            = glm::lookAt(glm::vec3(0.0f, 70.0f, 70.0f), glm::vec3(0.0f, 30.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        viewProjUniform.proj
            = glm::perspective(glm::radians(70.0f), (float)r_WindowWidth() / (float)r_WindowHeight(), 1.0f, 1000.0f);

        vertFragShaders.Activate();
        vertFragShaders.SetViewProjMatrices(viewProjUniform.view, viewProjUniform.proj);
        //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_branchBuffers[frameIndex]);
        glBindVertexArray(g_vertexVAOs[ frameIndex ]);
        glLineWidth(3.0f);
        glPointSize(1.0f);
        glDrawArrays(GL_LINES, 0, 2 * tree.size());

        // Render ImGUI window

        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        glfwSwapBuffers(r_GetWindow());

        double endTime = glfwGetTime();
        deltaTimeMs    = (endTime - startTime) * 1000.0;
        totalTimeMs += deltaTimeMs;

        if ( totalTimeMs > updateFreqMs )
        {
            char  windowTitleBuf[ 256 ] = { 0 };
            float fps                   = 1000.0f / deltaTimeMs;
            sprintf(windowTitleBuf, "Tree Test -- delta time: %f ms -- FPS: %f\n", deltaTimeMs, fps);
            glfwSetWindowTitle(r_GetWindow(), windowTitleBuf);
            totalTimeMs = 0.0;
        }

        frameIndex ^= 1; // Swap the frame index ( = swap the buffers used for reading/writing)
    }

    r_Shutdown();
}
