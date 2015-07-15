#pragma once

#include <core/Model.h>

#include <glm/gtc/quaternion.hpp>

#include <memory>

class Scene : public QOpenGLFunctions_3_3_Core
{
public:
	using ModelPtr = std::shared_ptr<Model>;

	void addModel(ModelPtr model);
	void initOpenGL();
	void resize(int width, int height);
	void render();

	const std::vector<ModelPtr>& models();

	glm::quat& rotation();
	glm::quat rotation() const;
	glm::vec3& translation();
	glm::vec3 translation() const;

	glm::vec3 center() const;
	glm::vec3 size() const;

protected:
	std::pair<glm::vec3, glm::vec3> boundingBox();

	std::vector<ModelPtr> m_models;
	glm::mat4 m_modelview, m_projection;
	glm::vec3 m_min, m_max, m_center, m_size;

	glm::quat m_rotation;
	glm::vec3 m_translation = { 0.f, 0.f, 0.f };

	QOpenGLShaderProgram m_program;
	GLint m_mvLoc = 0, m_mvpLoc = 0, m_colLoc = 0, m_texLoc = 0;
};

inline const std::vector<Scene::ModelPtr>& Scene::models()
{ return m_models; }

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
