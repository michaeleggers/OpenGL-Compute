#include "r_shader.h"

#include <glad/gl.h>

#include <stdio.h>

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "platform.h"


// GLOBAL SHADER BUFFERS (USED BY ALL SHADERS)

#define BIND_POINT_VIEW_PROJECTION    0
#define BIND_POINT_SETTINGS			  1
#define BIND_POINT_SSBO               2


bool Shader::Load(const std::string& vertName, const std::string& fragName, uint32_t shaderFeatureBits)
{
	if (!CompileShader(vertName, GL_VERTEX_SHADER, m_VertexShader)
		|| !CompileShader(fragName, GL_FRAGMENT_SHADER, m_FragmentShader)) {

		return false;
	}

	m_ShaderProgram = glCreateProgram();
	glAttachShader(m_ShaderProgram, m_VertexShader);
	glAttachShader(m_ShaderProgram, m_FragmentShader);
	glLinkProgram(m_ShaderProgram);

	if (!IsValidProgram()) {
		return false;
	}

	// Uniforms

	glGenBuffers(1, &m_ViewProjUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ViewProjUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// render settings (such as wireframe)

	glGenBuffers(1, &m_SettingsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_SettingsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	printf("Check Uniforms for Shaders: %s, %s\n", vertName.c_str(), fragName.c_str());

	bool uniformsFound = true;
	std::vector<std::string> uniformProblems{};

	// Per frame matrices
	// 
	// NOTE: Since we use the binding qualifier in the shader we don't really have to
	// call glUniformBlockBinding anymore. Also glGetUniformBlockIndex is not
	// neccessary sind we know the binding a priori. Useful for Uniforms that are global
	// to all shaders.
	// 
	//glBindBuffer(GL_UNIFORM_BUFFER, g_ViewProjUBO);
	GLuint bindingPoint = BIND_POINT_VIEW_PROJECTION;
	//m_ViewProjUniformIndex = glGetUniformBlockIndex(m_ShaderProgram, "ViewProjMatrices");
	//if (m_ViewProjUniformIndex == GL_INVALID_INDEX) {		
	//	uniformProblems.push_back({ "Not able to find uniform location for 'ViewProjMatrices'" });
	//	uniformsFound = false;
	//}
	//glUniformBlockBinding(m_ShaderProgram, m_ViewProjUniformIndex, bindingPoint);
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_ViewProjUBO, 0, 2*sizeof(glm::mat4));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Per frame settings
	//glBindBuffer(GL_UNIFORM_BUFFER, g_SettingsUBO);
	GLuint settingsBindingPoint = BIND_POINT_SETTINGS;
	//m_SettingsUniformIndex = glGetUniformBlockIndex(m_ShaderProgram, "Settings");
	//if (m_SettingsUniformIndex == GL_INVALID_INDEX) {
	//	uniformProblems.push_back({ "Not able to find uniform location for 'Setting'" });
	//	uniformsFound = false;
	//}
	//glUniformBlockBinding(m_ShaderProgram, m_SettingsUniformIndex, settingsBindingPoint);
	glBindBufferRange(GL_UNIFORM_BUFFER, settingsBindingPoint, m_SettingsUBO, 0, sizeof(uint32_t));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	if (!uniformsFound) {
		for (std::string& uniformMsg : uniformProblems) {
			printf("%s\n", uniformMsg.c_str());
		}
	}
	else {
		printf("All uniforms found\n");
	}

	return true;
}

bool Shader::Load(const std::string& computeName)
{
	if (!CompileShader(computeName, GL_COMPUTE_SHADER, m_ComputeShader)) {
		return false;
	}

	m_ShaderProgram = glCreateProgram();
	glAttachShader(m_ShaderProgram, m_ComputeShader);	
	glLinkProgram(m_ShaderProgram);

	if (!IsValidProgram()) {
		return false;
	}
	
	return true;
}

void Shader::Unload()
{
	glDeleteProgram(m_ShaderProgram);
	glDeleteShader(m_VertexShader);
	glDeleteShader(m_FragmentShader);
}

void Shader::Activate()
{
	glUseProgram(m_ShaderProgram);
}

GLuint Shader::Program() const
{
	return m_ShaderProgram;
}

void Shader::SetViewProjMatrices(glm::mat4 view, glm::mat4 proj)
{	
	glBindBuffer(GL_UNIFORM_BUFFER, m_ViewProjUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(proj));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::SetMatrixPalette(glm::mat4* palette, uint32_t numMatrices)
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_PaletteUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, numMatrices*sizeof(glm::mat4), palette);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::SetMat4(std::string uniformName, glm::mat4 mat4)
{
	GLuint loc = glGetUniformLocation(m_ShaderProgram, uniformName.c_str());
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&mat4);
}

void Shader::SetVec3(std::string uniformName, glm::vec3 vec3)
{
	GLuint loc = glGetUniformLocation(m_ShaderProgram, uniformName.c_str());
	glUniform3fv(loc, 1, (float*)&vec3);
}

void Shader::SetShaderSettingBits(uint32_t bits)
{
	m_SettingsBits |= bits;
	glBindBuffer(GL_UNIFORM_BUFFER, m_SettingsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uint32_t), (void*)&m_SettingsBits);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::ResetShaderSettingBits(uint32_t bits)
{
	m_SettingsBits = (m_SettingsBits & (~bits));
	glBindBuffer(GL_UNIFORM_BUFFER, m_SettingsUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uint32_t), (void*)&m_SettingsBits);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool Shader::CompileShader(const std::string& fileName, GLenum shaderType, GLuint& outShader)
{	
	uint8_t* shaderCode; 
	int codeSize = 0;
	read_shader_file(fileName.c_str(), &shaderCode, &codeSize);

	outShader = glCreateShader(shaderType);
	glShaderSource(outShader, 1, (GLchar**)(&shaderCode), nullptr);
	glCompileShader(outShader);

	if (!IsCompiled(outShader)) {
		printf("Failed to compile shader: %s\n", fileName.c_str());
		return false;
	}

	return true;
}

bool Shader::IsCompiled(GLuint shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		memset(buffer, 0, 512);
		glGetShaderInfoLog(shader, 511, nullptr, buffer);
		printf("GLSL compile error:\n%s\n", buffer);

		return false;
	}

	return true;
}

bool Shader::IsValidProgram()
{
	GLint status;
	glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		char buffer[512];
		memset(buffer, 0, 512);
		glGetProgramInfoLog(m_ShaderProgram, 511, nullptr, buffer);
		printf("GLSL compile error:\n%s\n", buffer);

		return false;
	}

	return true;
}
