#include "Document.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/SimpleGUI.h>
#include <core/VectorWrapper.h>

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
		auto node = ModelNode::create();
		if (mesh->mName.length)
			node->name = mesh->mName.C_Str();
		else
			node->name = "mesh " + std::to_string(i);
		node->type = "Mesh";
		node->parent = m_rootNode.get();
		m_rootNode->objects.push_back(node);
	}
}

Document::ObjectPropertiesPtr Document::objectProperties(GraphNode* baseItem)
{
	auto item = dynamic_cast<ModelNode*>(baseItem);
	if(!item)
		return nullptr;
	
	return nullptr;
}
