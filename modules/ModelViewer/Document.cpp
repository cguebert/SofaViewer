#include "Document.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/PropertiesUtils.h>
#include <core/SimpleGUI.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>

int ModelViewerDoc = RegisterDocument<Document>("ModelViewerDoc").setDescription("Open 3d models or scenes.")
	.addLoadFile("Collada (*.dae)")
	.addLoadFile("Blender 3D (*.blend)")
	.addLoadFile("3ds Max 3DS (*.3ds)")
	.addLoadFile("3ds Max ASE (*.ase)")
	.addLoadFile("Wavefront Object (*.obj)")
	.addLoadFile("Industry Foundation Classes (*.ifc)")
	.addLoadFile("XGL (*.xgl *.zgl)")
	.addLoadFile("Stanford Polygon Library (*.ply)")
	.addLoadFile("LightWave (*.lwo)")
	.addLoadFile("LightWave Scene (*.lws)")
	.addLoadFile("Modo (*.lxo)")
	.addLoadFile("Stereolithography (*.stl)")
	.addLoadFile("DirectX X (*.x)")
	.addLoadFile("AC3D (*.ac)")
	.addLoadFile("Milkshape 3D (*.ms3d)");
ModuleHandle ModelViewerModule = RegisterModule("ModelViewer").addDocument(ModelViewerDoc);

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
	}
}

Document::Document(ui::SimpleGUI& gui)
	: BaseDocument(gui)
	, m_gui(gui)
	, m_mouseManipulator(m_scene)
{
}

std::string Document::documentType()
{
	return "ModelViewerDoc";
}

bool Document::loadFile(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (!scene)
	{
	//	DoTheErrorLogging(importer.GetErrorString());
		return false;
	}

	parseScene(scene);
	return true;
}

void Document::initUI()
{
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
	m_scene.render();
}

bool Document::mouseEvent(const MouseEvent& event)
{
	return m_mouseManipulator.mouseEvent(event);
}

inline glm::vec3 convert(aiVector3D v)
{
	return glm::vec3{ v.x, v.y, v.z };
}

void Document::parseScene(const aiScene* scene)
{
	// Root
	m_rootNode = ModelNode::create();
	m_rootNode->name = "Model";
	m_rootNode->type = "Root";
	m_rootNode->parent = nullptr;

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const auto& mesh = scene->mMeshes[i];
		if (!mesh->HasPositions() || !mesh->HasFaces() || !mesh->HasNormals() || mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
			continue;

		auto node = ModelNode::create();
		if (mesh->mName.length)
			node->name = mesh->mName.C_Str();
		else
			node->name = "mesh " + std::to_string(i);
		node->type = "Mesh";
		node->parent = m_rootNode.get();
		m_rootNode->objects.push_back(node);

		auto model = std::make_shared<Model>();
		node->model = model;
		
		model->m_vertices.reserve(mesh->mNumVertices);
		for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
			model->m_vertices.push_back(convert(mesh->mVertices[j]));

		model->m_triangles.reserve(mesh->mNumFaces);
		for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
			model->m_triangles.push_back({ mesh->mFaces[j].mIndices[0], mesh->mFaces[j].mIndices[1], mesh->mFaces[j].mIndices[2] });

		model->m_normals.reserve(mesh->mNumVertices);
		for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
			model->m_normals.push_back(convert(mesh->mNormals[j]));

		m_scene.addModel(model);
	}
}

Document::ObjectPropertiesPtr Document::objectProperties(GraphNode* baseItem)
{
	auto item = dynamic_cast<ModelNode*>(baseItem);
	if(!item)
		return nullptr;

	auto model = item->model;
	if (!model)
		return nullptr;

	ObjectPropertiesPtr ptr = std::make_shared<ObjectProperties>(item->name);
	ptr->addProperty(property::createCopyProperty("vertices", model->m_vertices));
	ptr->addProperty(property::createCopyProperty("triangles", model->m_triangles));
	ptr->addProperty(property::createCopyProperty("normals", model->m_normals));
	
	return ptr;
}
