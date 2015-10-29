#include "MeshImport.h"
#include "Document.h"

#include <core/ObjectProperties.h>
#include <core/PropertiesUtils.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

namespace property
{
	namespace details
	{
		template <>
		struct ArrayTraits<glm::vec3>
		{
			static const bool isArray = true;
			static const bool fixed = true;
			static const int size = 3;
			using value_type = glm::vec3::value_type;
		};

		template <>
		struct ArrayTraits<glm::vec4>
		{
			static const bool isArray = true;
			static const bool fixed = true;
			static const int size = 4;
			using value_type = glm::vec4::value_type;
		};

		template <>
		struct ArrayTraits<glm::mat4>
		{
			static const bool isArray = true;
			static const bool fixed = true;
			static const int size = 4;
			using value_type = glm::mat4::col_type;
		};
	}
}

namespace std
{

inline glm::vec3::value_type* begin(glm::vec3& v) { return &v.x; }
inline glm::vec3::value_type* end(glm::vec3& v) { return &v.x + 3; }

}

namespace
{

inline glm::mat4 convert(const aiMatrix4x4& in)
{
	glm::mat4 out(glm::uninitialize);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			out[i][j] = in[i][j];
	return out;
}

inline glm::vec3 convert(aiVector3D v)
{
	return glm::vec3{ v.x, v.y, v.z };
}

std::shared_ptr<simplerender::Model> createModel(const aiMesh* mesh)
{
	auto model = std::make_shared<simplerender::Model>();
	model->m_vertices.reserve(mesh->mNumVertices);
	for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
		model->m_vertices.push_back(convert(mesh->mVertices[j]));

	if (mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
	{
		model->m_triangles.reserve(mesh->mNumFaces);
		for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
			model->m_triangles.push_back({ mesh->mFaces[j].mIndices[0], mesh->mFaces[j].mIndices[1], mesh->mFaces[j].mIndices[2] });
	}
	else if (mesh->mPrimitiveTypes == aiPrimitiveType_LINE)
	{
		model->m_edges.reserve(mesh->mNumFaces);
		for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
			model->m_edges.push_back({ mesh->mFaces[j].mIndices[0], mesh->mFaces[j].mIndices[1] });
	}

	model->m_normals.reserve(mesh->mNumVertices);
	for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
		model->m_normals.push_back(convert(mesh->mNormals[j]));

	return model;
}

}

MeshImport::MeshImport(Document* doc, simplerender::Scene& scene, Graph& graph)
	: m_document(doc)
	, m_scene(scene)
	, m_graph(graph)
{
}

std::vector<simplerender::Model*> MeshImport::importMeshes(const std::string& filePath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (scene)
		parseScene(scene);

	return m_newModels;
}

void MeshImport::parseNode(const aiScene* scene, const aiNode* aNode, const glm::mat4& transformation, SGANode* parent)
{
	auto n = m_document->createNode(aNode->mName.C_Str(), SGANode::Type::Node, parent);
	auto nodeTransformation = convert(aNode->mTransformation);
	n->transformation = nodeTransformation;
	auto accTrans = nodeTransformation * transformation;

	for (unsigned int i = 0; i < aNode->mNumMeshes; ++i)
		parseMeshInstance(scene, aNode->mMeshes[i], accTrans, n.get());

	for (unsigned int i = 0; i < aNode->mNumChildren; ++i)
		parseNode(scene, aNode->mChildren[i], accTrans, n.get());
}

void MeshImport::parseMeshInstance(const aiScene* scene, unsigned int id, const glm::mat4& transformation, SGANode* parent)
{
	const auto mesh = scene->mMeshes[id];
	const auto modelId = modelIndex(id);
	if (modelId < 0)
		return;

	auto n = m_document->createNode(mesh->mName.C_Str(), SGANode::Type::Instance, parent);
	n->meshId = modelId;
	n->model = m_scene.models()[modelId];
	n->transformation = transformation;
	m_scene.addInstance({ glm::transpose(transformation), n->model });
}

void MeshImport::parseScene(const aiScene* scene)
{
	// Adding meshes
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const auto& mesh = scene->mMeshes[i];
		if (!mesh->HasPositions() || !mesh->HasFaces() || !mesh->HasNormals() || 
			(mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE && mesh->mPrimitiveTypes != aiPrimitiveType_LINE))
			continue;

		auto node = m_document->createNode(mesh->mName.length ? mesh->mName.C_Str() : "mesh " + std::to_string(i), SGANode::Type::Mesh, m_graph.root());

		auto model = createModel(mesh);
		node->model = model;
		int index = m_scene.models().size();
		m_scene.addModel(model);
		m_modelsIndices.emplace_back(i, index);
		m_newModels.push_back(model.get());
	}

	// Adding graph
	glm::mat4 transformation;
	auto root = dynamic_cast<SGANode*>(m_graph.root());
	parseNode(scene, scene->mRootNode, transformation, root);
}

int MeshImport::modelIndex(int meshId)
{
	for (const auto& model : m_modelsIndices)
	{
		if (model.first == meshId)
			return model.second;
	}

	return -1;
}

void populateProperties(SGANode* item, const simplerender::Scene& scene, ObjectProperties* properties)
{
	switch (item->nodeType)
	{
	case SGANode::Type::Root:
	{
		properties->createPropertyAndWrapper("transformation", item->transformation);
		auto bb = simplerender::boundingBox(scene);
		auto sceneSize = bb.second - bb.first;
		auto sizeProp = property::createCopyProperty("Scene size", sceneSize);
		sizeProp->setReadOnly(true);
		break;
	}

	case SGANode::Type::Node:
	{
		properties->createPropertyAndWrapper("transformation", item->transformation);
		break;
	}

	case SGANode::Type::Mesh:
	{
		auto model = item->model;
		if (!model)
			return;

		properties->createPropertyAndWrapper("vertices", model->m_vertices);
		properties->createPropertyAndWrapper("edges", model->m_edges);
		properties->createPropertyAndWrapper("triangles", model->m_triangles);
		properties->createPropertyAndWrapper("normals", model->m_normals);
		break;
	}

	case SGANode::Type::Instance:
	{
		properties->addProperty(property::createRefProperty("mesh id", item->meshId));
		properties->createPropertyAndWrapper("transformation", item->transformation).first->setReadOnly(true);
		break;
	}
	}
}
