#pragma once

#include <memory>
#include <string>
#include <vector>

// To destroy the shader program only when every copy is destroyed
class ProgramId;

enum class ShaderType
{
	Vertex, Fragment
};

class ShaderProgram
{
public:
	bool addShaderFromMemory(ShaderType type, const std::string& shader);
	bool addShaderFromFile(ShaderType type, const std::string& path);
	bool link();
	void removeShaders();

	unsigned int id() const;
	void use() const;

	unsigned int uniformLocation(const std::string& name) const;

protected:
	void addShader(ShaderType type, unsigned int id);

	using ShaderPair = std::pair<ShaderType, unsigned int>;
	std::vector<ShaderPair> m_shaders;
	std::shared_ptr<ProgramId> m_programId;
};
