#include <render/Shader.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

namespace simplerender
{

using namespace std;

unsigned int m_program;

class ProgramId
{
public:
	ProgramId(unsigned int id = 0) : m_id(id) { }
	~ProgramId() { glDeleteProgram(m_id); }

	unsigned int id() const { return m_id; }

protected:
	unsigned int m_id;
};

//****************************************************************************//

bool ShaderProgram::addShaderFromMemory(ShaderType type, const std::string& content)
{
	GLuint glType = 0;
	switch (type)
	{
	case ShaderType::Vertex:	glType = GL_VERTEX_SHADER;		break;
	case ShaderType::Fragment:	glType = GL_FRAGMENT_SHADER;	break;
	}

	GLuint shader = glCreateShader(glType);
	const GLchar* code = content.c_str();
	glShaderSource(shader, 1, &code, nullptr);
	glCompileShader(shader);

	// Print compile errors if any
	GLint success = 0;
	GLchar infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cout << "Error : Compilation of shader failed\n" << infoLog << std::endl;
		glDeleteShader(shader);
		return false;
	};

	addShader(type, shader);

	return true;
}

bool ShaderProgram::addShaderFromFile(ShaderType type, const std::string& path)
{
	string content;
	ifstream file;

	file.exceptions(ifstream::failbit | ifstream::badbit);
	try
	{
		// Open files
		file.open(path);
		std::stringstream stream;
		stream << file.rdbuf();
		file.close();
		content = stream.str();
	}
	catch (ifstream::failure e)
	{
		cout << "Error : Cannot load shader file " << path << endl;
		return false;
	}
	
	return addShaderFromMemory(type, content);
}

bool ShaderProgram::link()
{
	m_programId.reset();

	unsigned int program = glCreateProgram();
	for (const auto& shader : m_shaders)
		glAttachShader(program, shader.second);
	glLinkProgram(program);

	// Print linking errors if any
	GLint success = 0;
	GLchar infoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cout << "Error : Shader program link failed\n" << infoLog << std::endl;
		glDeleteProgram(program);
		return false;
	}

	m_programId = std::make_shared<ProgramId>(program);

	removeShaders();

	return true;
}

void ShaderProgram::addShader(ShaderType type, unsigned int id)
{
	auto it = std::find_if(m_shaders.begin(), m_shaders.end(), [type](const ShaderPair& s){
		return s.first == type;
	});

	if (it != m_shaders.end())
	{
		glDeleteShader(it->second);
		it->second = id;
	}
	else
		m_shaders.emplace_back(type, id);
}

void ShaderProgram::removeShaders()
{
	for (const auto& shader : m_shaders)
		glDeleteShader(shader.second);
	m_shaders.clear();
}

unsigned int ShaderProgram::id() const
{
	if (m_programId)
		return m_programId->id();
	return 0;
}

void ShaderProgram::use() const
{
	glUseProgram(id());
}

unsigned int ShaderProgram::uniformLocation(const std::string& name) const
{
	return glGetUniformLocation(id(), name.c_str());
}

} // namespace simplerender
