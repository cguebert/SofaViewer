#pragma once

#include <render/Model.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/detail/type_mat4x4.hpp>

#include <memory>

class Scene
{
public:
	using ModelPtr = std::shared_ptr<Model>;
	using Models = std::vector<ModelPtr>;
	using ModelInstance = std::pair<glm::mat4, ModelPtr>;
	using ModelInstances = std::vector<ModelInstance>;

	void initOpenGL();
	void resize(int width, int height);
	void render();

	void addModel(ModelPtr model);
	Models& models();

	void addInstance(const ModelInstance& instance);
	ModelInstances& instances();

	glm::quat& rotation();
	glm::quat rotation() const;
	glm::vec3& translation();
	glm::vec3 translation() const;

	glm::vec3 center() const;
	glm::vec3 size() const;

protected:
	std::pair<glm::vec3, glm::vec3> boundingBox();

	Models m_models;
	ModelInstances m_instances;

	glm::mat4 m_modelview, m_projection;
	glm::vec3 m_min, m_max, m_center, m_size;

	glm::quat m_rotation;
	glm::vec3 m_translation = { 0.f, 0.f, 0.f };

	ShaderProgram m_program;
	int m_mvLoc = 0, m_mvpLoc = 0, m_colLoc = 0, m_texLoc = 0;
};

inline void Scene::addModel(ModelPtr model)
{ m_models.push_back(model); }

inline Scene::Models& Scene::models()
{ return m_models; }

inline void Scene::addInstance(const ModelInstance& instance)
{ m_instances.push_back(instance); }

inline Scene::ModelInstances& Scene::instances()
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
