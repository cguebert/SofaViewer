#include <render/Scene.h>
#include <render/shaders.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace simplerender
{

void Scene::initOpenGL()
{
	// Get OpenGL functions
	glewExperimental = GL_TRUE;
	glewInit();

	for(auto& model : m_models)
		model->init();

	auto bb = boundingBox(*this);
	m_min = bb.first; m_max = bb.second;
	m_center = (m_min + m_max) / 2.f;
	m_size = m_max - m_min;

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	prepareProgram(m_trianglesProg, trianglesVertexShader, trianglesFragmentShader);
	prepareProgram(m_linesProg, linesVertexShader, linesFragmentShader);
}

void Scene::resize(int width, int height)
{
	// Height / width ratio
	GLfloat ratio = height ? (GLfloat)width / (GLfloat)height : 1.f;

	// Setup our viewport
	glViewport(0, 0, width, height);

	// Compute zFar
	const float zFar = std::max( {m_size[0], m_size[1], m_size[2]} ) * 10.f;

	// Change to the projection matrix and set our viewing volume
	m_projection = glm::perspective( 45.f, ratio, 0.1f, zFar );
}

void Scene::render()
{
	// Modelview matrix
	m_modelview = glm::mat4(1.f);
	auto sizeMax = std::max( {m_size[0], m_size[1], m_size[2]} );
	m_modelview = glm::translate(m_modelview, glm::vec3(0.f, 0.f, -(sizeMax * 1.5f)));	// move the scene to see it
	m_modelview = glm::translate(m_modelview, m_translation );
	m_modelview = m_modelview * glm::mat4_cast(m_rotation);
	m_modelview = glm::rotate(m_modelview, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f) );					// correct rotation for the example scene
	m_modelview = glm::rotate(m_modelview, glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f) );
	m_modelview = glm::translate(m_modelview, -m_center);		// move scene to origin to apply rotation
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_instances.empty())
	{
		glm::mat4 modelviewProjection = m_projection * m_modelview;
		
		for (const auto& object : m_models)
		{
			const auto& prog = selectProgram(object);
			prog.program.use();

			if (prog.mvLoc != -1) 
				glUniformMatrix4fv(prog.mvLoc, 1, GL_FALSE, glm::value_ptr(m_modelview));
			glUniformMatrix4fv(prog.mvpLoc, 1, GL_FALSE, glm::value_ptr(modelviewProjection));

			glUniform4fv(prog.colLoc, 1, &object->m_color[0]);

			object->render();
		}
	}
	else
	{
		for (const auto& instance : m_instances)
		{
			const auto& object = instance.second;
			const auto& prog = selectProgram(object);
			prog.program.use();

			glm::mat4 modelview = m_modelview * instance.first;
			glm::mat4 modelviewProjection = m_projection * modelview;
			if (prog.mvLoc != -1)
				glUniformMatrix4fv(prog.mvLoc, 1, GL_FALSE, glm::value_ptr(modelview));
			glUniformMatrix4fv(prog.mvpLoc, 1, GL_FALSE, glm::value_ptr(modelviewProjection));

			glUniform4fv(prog.colLoc, 1, &object->m_color[0]);

			object->render();
		}
	}
}

void Scene::prepareProgram(ProgramStruct& ps, const char* vertexShader, const char* fragmentShader)
{
	auto& prog = ps.program;
	prog.removeShaders();
	prog.addShaderFromMemory(ShaderType::Vertex, vertexShader);
	prog.addShaderFromMemory(ShaderType::Fragment, fragmentShader);
	prog.link();

	ps.mvLoc = prog.uniformLocation("MV");
	ps.mvpLoc = prog.uniformLocation("MVP");
	ps.colLoc = prog.uniformLocation("diffuseColor");
	ps.texLoc = prog.uniformLocation("texture");
}

Scene::ProgramStruct& Scene::selectProgram(const ModelPtr model)
{
	if (!model->m_mergedTriangles.empty())
		return m_trianglesProg;
	return m_linesProg;
}

std::pair<glm::vec3, glm::vec3> boundingBox(const Scene& scene)
{
	if (scene.models().empty())
		return std::make_pair(glm::vec3(-5, -5, -5), glm::vec3(5, 5, 5));

	glm::vec3 vMin, vMax;
	for (int i = 0; i < 3; ++i)
	{
		vMin[i] = std::numeric_limits<float>::max();
		vMax[i] = -std::numeric_limits<float>::max();
	}

	if (scene.instances().empty())
	{
		for (const auto& model : scene.models())
		{
			auto bb = boundingBox(*model.get());
			for (int i = 0; i < 3; ++i)
			{
				vMin[i] = std::min(vMin[i], bb.first[i]);
				vMax[i] = std::max(vMax[i], bb.second[i]);
			}
		}
	}
	else
	{
		for (const auto& instance : scene.instances())
		{
			auto bb = boundingBox(*instance.second.get(), instance.first);
			for (int i = 0; i < 3; ++i)
			{
				vMin[i] = std::min(vMin[i], bb.first[i]);
				vMax[i] = std::max(vMax[i], bb.second[i]);
			}
		}
	}

	return std::make_pair(vMin, vMax);
}

} // namespace simplerender
