#include "Document.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/PropertiesUtils.h>
#include <core/SimpleGUI.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>

int SofaGraphAbstractionDoc = RegisterDocument<Document>("Sofa Graph Abstraction").setDescription("Create Sofa simulations using higher level objects.").canCreateNew(true);
ModuleHandle SofaGraphAbstractionModule = RegisterModule("SofaGraphAbstraction").addDocument(SofaGraphAbstractionDoc);

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

Document::Document(const std::string& type)
	: BaseDocument(type)
	, m_mouseManipulator(m_scene)
	, m_sgaFactory(modulePath() + "/definitions")
{
}

bool Document::loadFile(const std::string& path)
{
	return false;
}

void Document::initUI(simplegui::SimpleGUI& gui)
{
	m_gui = &gui;
	m_gui->getMenu(simplegui::SimpleGUI::MenuType::Tools).addItem("Import mesh", "Import a scene or a mesh", [this](){ importMesh(); });

	auto root = createNode("SGA scene", "Root", ModelNode::Type::Root, nullptr);
	m_rootNode = root;
	m_graph.setRoot(m_rootNode);
}

void Document::initOpenGL()
{
	m_scene.initOpenGL();
}

void Document::resize(int width, int height)
{
	m_scene.resize(width, height);
}

void Document::render()
{
	if (m_reinitScene)
	{
		m_reinitScene = false;
		for (auto model : m_scene.models())
			model->init();
	}

	m_scene.render();
}

bool Document::mouseEvent(const MouseEvent& event)
{
	return m_mouseManipulator.mouseEvent(event);
}

ModelNode::SPtr Document::createNode(const std::string& name, const std::string& type, ModelNode::Type nodeType, GraphNode::SPtr parent)
{
	auto node = ModelNode::create();
	node->name = name;
	node->type = type;
	node->nodeType = nodeType;
	node->parent = parent.get();
	node->uniqueId = m_nextNodeId++;
	if (parent)
		parent->children.push_back(node);

	return node;
}

inline glm::mat4 convert(const aiMatrix4x4& in)
{
	glm::mat4 out(glm::uninitialize);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			out[i][j] = in[i][j];
	return out;
}

void Document::parseNode(const aiScene* scene, const aiNode* aNode, const glm::mat4& transformation, GraphNode::SPtr parent)
{
	auto n = createNode(aNode->mName.C_Str(), "Node", ModelNode::Type::Node, parent);
	auto nodeTransformation = convert(aNode->mTransformation);
	n->transformation = nodeTransformation;
	auto accTrans = nodeTransformation * transformation;

	for (unsigned int i = 0; i < aNode->mNumChildren; ++i)
		parseNode(scene, aNode->mChildren[i], accTrans, n);

	for (unsigned int i = 0; i < aNode->mNumMeshes; ++i)
		parseMeshInstance(scene, aNode->mMeshes[i], accTrans, n);
}

void Document::parseMeshInstance(const aiScene* scene, unsigned int id, const glm::mat4& transformation, GraphNode::SPtr parent)
{
	const auto mesh = scene->mMeshes[id];
	const auto modelId = modelIndex(id);
	if (modelId < 0)
		return;

	auto n = createNode(mesh->mName.C_Str(), "Instance", ModelNode::Type::Instance, parent);
	n->meshId = modelId;
	n->transformation = transformation;
	m_scene.addInstance({ glm::transpose(transformation), m_scene.models()[modelId] });
}

void Document::parseScene(const aiScene* scene)
{
	// Adding meshes
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const auto& mesh = scene->mMeshes[i];
		if (!mesh->HasPositions() || !mesh->HasFaces() || !mesh->HasNormals() || mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
			continue;

		auto node = createNode(mesh->mName.length ? mesh->mName.C_Str() : "mesh " + std::to_string(i), "Mesh", ModelNode::Type::Mesh, m_rootNode);

		auto model = createModel(mesh);
		node->model = model;
		m_scene.addModel(model);
		m_modelsIndices.push_back(i);
	}

	// Adding graph
	glm::mat4 transformation;
	parseNode(scene, scene->mRootNode, transformation, m_rootNode);
}

inline glm::vec3 convert(aiVector3D v)
{
	return glm::vec3{ v.x, v.y, v.z };
}

std::shared_ptr<simplerender::Model> Document::createModel(const aiMesh* mesh)
{
	auto model = std::make_shared<simplerender::Model>();
	model->m_vertices.reserve(mesh->mNumVertices);
	for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
		model->m_vertices.push_back(convert(mesh->mVertices[j]));

	model->m_triangles.reserve(mesh->mNumFaces);
	for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
		model->m_triangles.push_back({ mesh->mFaces[j].mIndices[0], mesh->mFaces[j].mIndices[1], mesh->mFaces[j].mIndices[2] });

	model->m_normals.reserve(mesh->mNumVertices);
	for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
		model->m_normals.push_back(convert(mesh->mNormals[j]));

	return model;
}

Document::ObjectPropertiesPtr Document::objectProperties(GraphNode* baseItem)
{
	auto item = dynamic_cast<ModelNode*>(baseItem);
	if(!item)
		return nullptr;

	switch (item->nodeType)
	{
	case ModelNode::Type::Root:
	{
		auto properties = std::make_shared<ObjectProperties>(item->name);
		properties->createPropertyAndWrapper("transformation", item->transformation);
		auto bb = simplerender::boundingBox(m_scene);
		auto sceneSize = bb.second - bb.first;
		properties->addProperty(property::createCopyProperty("Scene size", sceneSize));
		return properties;
	}

	case ModelNode::Type::Node:
	{
		auto properties = std::make_shared<ObjectProperties>(item->name);
		properties->createPropertyAndWrapper("transformation", item->transformation);
		return properties;
	}

	case ModelNode::Type::Mesh:
	{
		auto model = item->model;
		if (!model)
			return nullptr;

		auto properties = std::make_shared<ObjectProperties>(item->name);
		properties->createPropertyAndWrapper("vertices", model->m_vertices);
		properties->createPropertyAndWrapper("triangles", model->m_triangles);
		properties->createPropertyAndWrapper("normals", model->m_normals);
		return properties;
	}

	case ModelNode::Type::Instance:
	{
		auto properties = std::make_shared<ObjectProperties>(item->name);
		properties->addProperty(property::createRefProperty("mesh id", item->meshId));
		properties->createPropertyAndWrapper("transformation", item->transformation);
		return properties;
	}
	}

	return nullptr;
}

int Document::modelIndex(int meshId)
{
	for (int i = 0, nb = m_modelsIndices.size(); i < nb; ++i)
	{
		if (m_modelsIndices[i] == meshId)
			return i;
	}

	return -1;
}

void Document::importMesh()
{
	auto path = m_gui->getOpenFileName("Import mesh", "", "Supported files (*.3ds *.ac *.ase *.blend *.dae *.ifc *.lwo *.lws *.lxo *.ms3d *.obj *.ply *.stl *.x *.xgl *.zgl");
	if (path.empty())
		return;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (scene)
	{
		parseScene(scene);
		m_graph.setRoot(m_rootNode); // Update the whole graph (TODO: update only the new nodes)
		m_reinitScene = true;
	}
}
