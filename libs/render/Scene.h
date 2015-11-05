#pragma once

#include <render/Mesh.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/detail/type_mat4x4.hpp>

namespace simplerender
{

class Scene
{
public:
	using Models = std::vector<Mesh::SPtr>;
	using ModelInstance = std::pair<glm::mat4, Mesh::SPtr>;
	using ModelInstances = std::vector<ModelInstance>;

	void initOpenGL();
	void resize(int width, int height);
	void render();

	void addModel(Mesh::SPtr model);
	Models& models();
	const Models& models() const;

	void addInstance(const ModelInstance& instance);
	ModelInstances& instances();
	const ModelInstances& instances() const;

	glm::quat& rotation();
	glm::quat rotation() const;
	glm::vec3& translation();
	glm::vec3 translation() const;

	glm::vec3 center() const;
	glm::vec3 size() const;

protected:
	struct ProgramStruct
	{
		ShaderProgram program;
		int mvLoc = 0, mvpLoc = 0, colLoc = 0, texLoc = 0;
	};

	void prepareProgram(ProgramStruct& ps, const char* vertexShader, const char* fragmentShader);
	ProgramStruct& selectProgram(const Mesh::SPtr model);

	Models m_models;
	ModelInstances m_instances;

	glm::mat4 m_modelview, m_projection;
	glm::vec3 m_min, m_max, m_center, m_size;

	glm::quat m_rotation;
	glm::vec3 m_translation = { 0.f, 0.f, 0.f };

	ProgramStruct m_trianglesProg, m_linesProg;
};

std::pair<glm::vec3, glm::vec3> boundingBox(const Scene& scene);

//****************************************************************************//

inline void Scene::addModel(Mesh::SPtr model)
{ m_models.push_back(model); }

inline Scene::Models& Scene::models()
{ return m_models; }

inline const Scene::Models& Scene::models() const
{ return m_models; }

inline void Scene::addInstance(const ModelInstance& instance)
{ m_instances.push_back(instance); }

inline Scene::ModelInstances& Scene::instances()
{ return m_instances; }

inline const Scene::ModelInstances& Scene::instances() const
{ return m_instances; }

inline glm::quat& Scene::rotation()
{ return m_rotation; }

inline glm::quat Scene::rotation() const
{ return m_rotation; }

inline glm::vec3& Scene::translation()
{ return m_translation; }

inline glm::vec3 Scene::translation() const
{ return m_translation; }

inline glm::vec3 Scene::center() const
{ return m_center; }

inline glm::vec3 Scene::size() const
{ return m_size; }

} // namespace simplerender
