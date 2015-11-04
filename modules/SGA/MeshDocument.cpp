#include "MeshDocument.h"
#include "MeshImport.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/PropertiesUtils.h>
#include <core/SimpleGUI.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

int registerMeshDocument()
{
	return RegisterDocument<MeshDocument>("Model Viewer").setDescription("Open 3d models or scenes.")
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
}

namespace
{

const std::vector<std::string>& meshNodeTypeNames()
{
	static std::vector<std::string> typesNames = { "Root", "Node", "Mesh", "Instance" };
	return typesNames;
}

const std::string& getTypeName(MeshNode::Type type)
{
	return meshNodeTypeNames()[static_cast<int>(type)];
}

}

TransformationComponents toTransformationComponents(const glm::mat4& matrix)
{
	TransformationComponents transformation;
	auto modelTrans = glm::transpose(matrix);
	glm::vec3 scale, translation, skew;
	glm::vec4 perspective;
	glm::quat orientation;
	if (glm::decompose(modelTrans, scale, orientation, translation, skew, perspective))
	{
		glm::vec3 rotation = glm::degrees(glm::eulerAngles(orientation));
		transformation = { translation, rotation, scale };
	}

	return transformation;
}

glm::mat4 toTransformationMatrix(const TransformationComponents& transformation)
{
	glm::quat orientation(glm::radians(transformation.rotation));
	glm::mat4 transMat;
	transMat = glm::translate(transMat, transformation.translation);
	transMat = transMat * glm::toMat4(orientation);
	transMat = glm::scale(transMat, transformation.scale);
	transMat = glm::transpose(transMat);

	return transMat;
}

MeshDocument::MeshDocument(const std::string& type)
	: BaseDocument(type)
	, m_mouseManipulator(m_scene)
{
	createGraphImages();
	m_rootNode = createNode("ModelViewer", MeshNode::Type::Root, nullptr);
}

bool MeshDocument::loadFile(const std::string& path)
{
	MeshImport importer(this, m_scene, m_graph);
	m_newModels = importer.importMeshes(path);
	m_graph.setRoot(m_rootNode); // Update the whole graph (TODO: update only the new nodes)
	return true;
}

bool MeshDocument::saveFile(const std::string& path)
{
	return false;
}

void MeshDocument::initUI(simplegui::SimpleGUI& gui)
{
	m_graph.setRoot(m_rootNode);
}

void MeshDocument::initOpenGL()
{
	m_scene.initOpenGL();
}

void MeshDocument::resize(int width, int height)
{
	m_scene.resize(width, height);
}

void MeshDocument::render()
{
	for (auto model : m_newModels)
		model->init();
	m_newModels.clear();

	m_scene.render();
}

bool MeshDocument::mouseEvent(const MouseEvent& event)
{
	return m_mouseManipulator.mouseEvent(event);
}

MeshNode::SPtr MeshDocument::createNode(const std::string& name, MeshNode::Type nodeType, GraphNode* parent, int position)
{
	auto node = MeshNode::create();
	node->name = name;
	node->type = getTypeName(nodeType);
	node->nodeType = nodeType;
	node->parent = parent;
	node->uniqueId = m_nextNodeId++;
	node->imageId = m_graphMeshImages[static_cast<int>(nodeType)];
	if (parent)
		m_graph.insertChild(parent, node, position);

	return node;
}

GraphNode::SPtr MeshDocument::createNode(const std::string& typeName, const std::string& id)
{
	auto& names = meshNodeTypeNames();
	auto it = std::find(names.begin(), names.end(), typeName);
	if (it == names.end()) // Unknown node
		return nullptr;

	int index = std::distance(names.begin(), it);
	auto meshType = static_cast<MeshNode::Type>(index);
	auto node = createNode(id, meshType, nullptr);

	if (meshType == MeshNode::Type::Mesh)
	{
		auto model = std::make_shared<simplerender::Model>();
		m_scene.addModel(model);
		node->model = model;
		m_newModels.push_back(node->model.get());
	}

	return node;
}

MeshDocument::ObjectPropertiesPtr MeshDocument::objectProperties(GraphNode* baseItem)
{
	auto item = dynamic_cast<MeshNode*>(baseItem);
	if(!item)
		return nullptr;

	auto properties = std::make_shared<ObjectProperties>(item->name);
	switch (item->nodeType)
	{
	case MeshNode::Type::Root:
	{
		properties->createPropertyAndWrapper("translation", item->transformationComponents.translation);
		properties->createPropertyAndWrapper("rotation", item->transformationComponents.rotation);
		properties->createPropertyAndWrapper("scale", item->transformationComponents.scale);

		auto bb = simplerender::boundingBox(m_scene);
		auto sceneSize = bb.second - bb.first;
		auto sizeProp = property::createCopyProperty("Scene size", sceneSize);
		sizeProp->setReadOnly(true);
		properties->addProperty(sizeProp);
		break;
	}

	case MeshNode::Type::Node:
	{
		properties->createPropertyAndWrapper("translation", item->transformationComponents.translation);
		properties->createPropertyAndWrapper("rotation", item->transformationComponents.rotation);
		properties->createPropertyAndWrapper("scale", item->transformationComponents.scale);
		break;
	}

	case MeshNode::Type::Mesh:
	{
		auto model = item->model;
		if (model)
		{
			properties->createPropertyAndWrapper("vertices", model->m_vertices);
			properties->createPropertyAndWrapper("edges", model->m_edges);
			properties->createPropertyAndWrapper("triangles", model->m_triangles);
			properties->createPropertyAndWrapper("normals", model->m_normals);
			properties->createPropertyAndWrapper("color", model->m_color);
		}
		break;
	}

	case MeshNode::Type::Instance:
	{
		properties->addProperty(property::createRefProperty("mesh id", item->meshId));
		properties->createPropertyAndWrapper("transformation", item->transformationMatrix).first->setReadOnly(true);
		break;
	}
	}

	return properties;
}

void MeshDocument::closeObjectProperties(GraphNode* baseItem, ObjectPropertiesPtr ptr, bool accepted)
{
	auto item = dynamic_cast<MeshNode*>(baseItem);
	if (!item)
		return;

	if (item->nodeType == MeshNode::Type::Root || item->nodeType == MeshNode::Type::Node)
		updateInstances();
}

void MeshDocument::graphContextMenu(GraphNode* baseItem, simplegui::Menu& menu)
{
	auto item = dynamic_cast<MeshNode*>(baseItem);
	if (!item)
		return;

	MeshNode* parent = nullptr;
	if (item->parent)
		parent = dynamic_cast<MeshNode*>(item->parent);

	switch (item->nodeType)
	{
	case MeshNode::Type::Node:
	{
		menu.addItem("Add node", "Add a new child node", [item, this]() { addNode(item); });
		menu.addItem("Remove node", "Remove this node", [item, this]() { removeNode(item); });
		menu.addSeparator();
		menu.addItem("Add mesh instance", "Add a new mesh instance", [item, this]() { addInstance(item); });
		return;
	}

	case MeshNode::Type::Instance:
	{
		menu.addItem("Remove instance", "Remove this mesh instance", [item, this]() { removeNode(item); });
		return;
	}
	}
}

void MeshDocument::createGraphImages()
{
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createDiskImage(0xffdedede))); // Root
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createDiskImage(0xffdedede))); // Node
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xff00daff }))); // Mesh
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xff80b1d3 }))); // Instance
}

void MeshDocument::updateInstances()
{
	m_scene.instances().clear();
	glm::mat4 transformation;
	updateInstances(dynamic_cast<MeshNode*>(m_graph.root()), transformation);
}

void MeshDocument::updateInstances(MeshNode* item, const glm::mat4& transformation)
{
	auto accTrans = transformation;
	if (item->nodeType == MeshNode::Type::Root)
	{
		accTrans = item->transformationMatrix * transformation;
	}
	else if(item->nodeType == MeshNode::Type::Node)
	{
		glm::mat4 transMat = toTransformationMatrix(item->transformationComponents);
		accTrans = transMat * transformation;
		item->transformationMatrix = transMat;
	}
	else if (item->nodeType == MeshNode::Type::Instance)
	{
		item->transformationMatrix = transformation;
		auto model = m_scene.models()[item->meshId];
		item->model = model;
		m_scene.addInstance({ glm::transpose(transformation), model });
	}

	for (auto child : item->children)
	{
		auto node = dynamic_cast<MeshNode*>(child.get());
		if (node)
			updateInstances(node, accTrans);
	}
}

void MeshDocument::addNode(MeshNode* parent)
{

}

void MeshDocument::removeNode(MeshNode* item)
{
	if(item && item->parent)
		m_graph.removeChild(item->parent, item);
}

void MeshDocument::addInstance(MeshNode* parent)
{

}
