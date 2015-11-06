#pragma once

#include <render/Mesh.h>
#include <render/Material.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/detail/type_mat4x4.hpp>

namespace simplerender
{

struct ModelInstance
{
	using SPtr = std::shared_ptr<ModelInstance>;
	glm::mat4 transformation;
	Mesh::SPtr mesh;
	Material::SPtr material;
};

class Scene
{
public:
	using Meshes = std::vector<Mesh::SPtr>;
	using Materials = std::vector<Material::SPtr>;
	using ModelInstances = std::vector<ModelInstance::SPtr>;

	void initOpenGL();
	void resize(int width, int height);
	void render();

	void addMesh(const Mesh::SPtr& mesh);
	Meshes& meshes();
	const Meshes& meshes() const;

	void addMaterial(const Material::SPtr& material);
	Materials& materials();
	const Materials& materials() const;

	void addInstance(const ModelInstance::SPtr& instance);
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
	ProgramStruct& selectProgram(const Mesh::SPtr mesh);

	Meshes m_meshes;
	Materials m_materials;
	ModelInstances m_instances; // Only this list is used during rendering, the meshes and materials lists are here for convenience

	glm::mat4 m_modelview, m_projection;
	glm::vec3 m_min, m_max, m_center, m_size;

	glm::quat m_rotation;
	glm::vec3 m_translation = { 0.f, 0.f, 0.f };

	ProgramStruct m_trianglesProg, m_linesProg;
	Material defaultMaterial;
};

std::pair<glm::vec3, glm::vec3> boundingBox(const Scene& scene);

//****************************************************************************//

inline void Scene::addMesh(const Mesh::SPtr& mesh)
{ m_meshes.push_back(mesh); }

inline Scene::Meshes& Scene::meshes()
{ return m_meshes; }

inline const Scene::Meshes& Scene::meshes() const
{ return m_meshes; }

inline void Scene::addMaterial(const Material::SPtr& material)
{ m_materials.push_back(material); }

inline Scene::Materials& Scene::materials()
{ return m_materials; }

inline const Scene::Materials& Scene::materials() const
{ return m_materials; }

inline void Scene::addInstance(const ModelInstance::SPtr& instance)
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
