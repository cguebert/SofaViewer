#include "SGADocument.h"
#include "SGAExecution.h"
#include "SGAProperties.h"
#include "MeshImport.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/PropertiesUtils.h>
#include <core/SimpleGUI.h>

#include <serialization/DocXML.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

int registerSGADocument()
{
	return RegisterDocument<SGADocument>("Sofa Graph Abstraction")
		.setDescription("Create Sofa simulations using higher level objects.")
		.canCreateNew(true)
		.addLoadFile("SGA document (*.sga)")
		.addSaveFile("SGA document (*.sga)");
}

ModuleHandle SofaGraphAbstractionModule = RegisterModule("SofaGraphAbstraction")
	.addDocument(registerSGADocument())
	.addDocument(registerMeshDocument());

namespace
{

SGANode* getChild(GraphNode* parent, SGANode::Type type)
{
	for (auto child : parent->children)
	{
		auto sgaNode = dynamic_cast<SGANode*>(child.get());
		if (sgaNode->nodeType == type)
			return sgaNode;
	}

	return nullptr;
}

MeshNode* getChild(GraphNode* parent, MeshNode::Type type)
{
	for (auto child : parent->children)
	{
		auto meshNode = dynamic_cast<MeshNode*>(child.get());
		if (meshNode->nodeType == type)
			return meshNode;
	}

	return nullptr;
}

const std::vector<std::string>& sgaNodeTypeNames()
{
	static std::vector<std::string> typesNames = { "SGA root", "SGA physics", "SGA collision", "SGA visual", "SGA modifier" };
	return typesNames;
}

const std::string& getTypeName(SGANode::Type type)
{
	return sgaNodeTypeNames()[static_cast<int>(type)];
}

SGANode::Type SGAToNodeType(sga::ObjectDefinition::ObjectType type)
{
	static std::vector<SGANode::Type> sgaToNodeTypes = { SGANode::Type::SGA_Root, SGANode::Type::SGA_Physics, SGANode::Type::SGA_Collision, SGANode::Type::SGA_Visual, SGANode::Type::SGA_Modifier };
	return sgaToNodeTypes[static_cast<int>(type)];
}

const std::string& SGATypeName(sga::ObjectDefinition::ObjectType type)
{
	static std::vector<std::string> sgaTypesNames = { "root", "physics", "collision", "visual", "modifier" };
	return sgaTypesNames[static_cast<int>(type)];
}

// Returns the position of the new node, and the node to be removed or null
// Default order: Physics, Collision, Visual, Modifier, Nodes
std::pair<int, SGANode*> indexOfNewNode(GraphNode* parent, sga::ObjectDefinition::ObjectType type)
{
	using ObjectType = sga::ObjectDefinition::ObjectType;
	switch (type)
	{
	case ObjectType::RootObject:
	{
		auto prev = getChild(parent, SGANode::Type::SGA_Root);
		return std::make_pair(0, prev);
	}

	case ObjectType::PhysicsObject:
	{
		auto prev = getChild(parent, SGANode::Type::SGA_Physics);
		return std::make_pair(0, prev);
	}

	case ObjectType::CollisionObject:
	{
		bool hasPhysics = (nullptr != getChild(parent, SGANode::Type::SGA_Physics));
		auto prev = getChild(parent, SGANode::Type::SGA_Collision);
		return std::make_pair(hasPhysics ? 1 : 0, prev);
	}

	case ObjectType::VisualObject:
	{
		int index = 0;
		if (getChild(parent, SGANode::Type::SGA_Physics)) ++index;
		if (getChild(parent, SGANode::Type::SGA_Collision)) ++index;
		auto prev = getChild(parent, SGANode::Type::SGA_Visual);
		return std::make_pair(index, prev);
	}

	case ObjectType::ModifierObject:
	{
		auto firstNode = getChild(parent, MeshNode::Type::Node);
		if (firstNode)
			return std::make_pair(graph::indexOfChild(parent, firstNode), nullptr); // Add before the first node
		return std::make_pair(-1, nullptr); // Add at the end
	}
	}

	return std::make_pair(-1, nullptr);
}

}

SGADocument::SGADocument(const std::string& type)
	: MeshDocument(type)
	, m_sgaFactory(modulePath() + "/definitions")
{
	m_rootNode->name = "SGA scene";

	prepareSGAObjectsLists();
	createSGAGraphImages();
}

bool SGADocument::loadFile(const std::string& path)
{
	auto createNodeFunc = [this](const std::string& typeName, const std::string& id) 
	{ return createNode(typeName, id); };

	auto getPropertiesFunc = [this](GraphNode* item) 
	{ return objectProperties(item); };

	auto node = importXMLFile(path, createNodeFunc, getPropertiesFunc);
	if (!node)
		return false;

	m_rootNode = node;
	m_graph.setRoot(m_rootNode);
	updateInstances();
	return true;
}

bool SGADocument::saveFile(const std::string& path)
{
	auto getPropertiesFunc = [this](GraphNode* item) { return objectProperties(item); };
	return exportToXMLFile(path, m_rootNode.get(), getPropertiesFunc);
}

void SGADocument::initUI(simplegui::SimpleGUI& gui)
{
	m_gui = &gui;
	auto& toolsMenu = m_gui->getMenu(simplegui::SimpleGUI::MenuType::Tools);
	toolsMenu.addItem("Import mesh", "Import a scene or a mesh", [this](){ importMesh(); });
	toolsMenu.addSeparator();

	auto& panel = m_gui->buttonsPanel();
	panel.addButton("Run", "Convert to a Sofa simulation and run it", [this](){ 
		if (!m_execution)
			convertAndRun();
	});

	panel.addButton("Stop", "Stop the Sofa simulation", [this]() { stopExecution(); });

	MeshDocument::initUI(gui);
}

void SGADocument::initOpenGL()
{
	MeshDocument::initOpenGL();
}

void SGADocument::resize(int width, int height)
{
	MeshDocument::resize(width, height);
}

void SGADocument::render()
{
	if (m_execution)
	{
		m_execution->render();
		return;
	}

	MeshDocument::render();
}

bool SGADocument::mouseEvent(const MouseEvent& event)
{
	return MeshDocument::mouseEvent(event);
}

SGANode::SPtr SGADocument::createNode(const std::string& name, SGANode::Type nodeType, GraphNode* parent, int position)
{
	auto node = SGANode::create();
	node->name = name;
	node->type = getTypeName(nodeType);
	node->nodeType = nodeType;
	node->parent = parent;
	node->uniqueId = m_nextNodeId++;
	node->imageId = m_graphSGAImages[static_cast<int>(nodeType)];
	if (parent)
		m_graph.insertChild(parent, node, position);

	return node;
}

GraphNode::SPtr SGADocument::createNode(const std::string& typeName, const std::string& id)
{
	auto& names = sgaNodeTypeNames();
	auto it = std::find(names.begin(), names.end(), typeName);
	if (it == names.end()) // Not a SGA node
		return MeshDocument::createNode(typeName, id);

	int index = std::distance(names.begin(), it);
	auto sgaType = static_cast<SGANode::Type>(index);

	auto node = createNode(id, sgaType, nullptr);
	node->sgaDefinition = m_sgaFactory.definition(id);
	return node;
}

SGADocument::ObjectPropertiesPtr SGADocument::objectProperties(GraphNode* baseItem)
{
	stopExecution();

	auto sgaNode = dynamic_cast<SGANode*>(baseItem);
	if (sgaNode)
		return createSGAObjectProperties(sgaNode->sgaDefinition);

	auto properties = MeshDocument::objectProperties(baseItem);
	auto meshNode = dynamic_cast<MeshNode*>(baseItem);
	if (meshNode && meshNode->nodeType == MeshNode::Type::Root)
	{
		properties->createPropertyAndWrapper("gravity", m_simulationProperties.gravity).first->setGroup("SGA");
		properties->createPropertyAndWrapper("timestep", m_simulationProperties.timestep).first->setGroup("SGA");
	}

	return properties;
}

void SGADocument::closeObjectProperties(GraphNode* baseItem, ObjectPropertiesPtr ptr, bool accepted)
{
	MeshDocument::closeObjectProperties(baseItem, ptr, accepted);
}

void SGADocument::graphContextMenu(GraphNode* baseItem, simplegui::Menu& menu)
{
	auto parent = baseItem->parent;
	auto meshNode = dynamic_cast<MeshNode*>(baseItem);
	if (meshNode)
	{

		switch (meshNode->nodeType)
		{
		case MeshNode::Type::Root:
		{
			menu.addItem("Set SGA root node", "Change the type of the Sofa simulation", [meshNode, this]() { addSGANode(meshNode, sga::ObjectDefinition::ObjectType::RootObject); });
			return;
		}

		case MeshNode::Type::Node:
		{
			menu.addItem("Add node", "Add a new child node", [meshNode, this]() { addNode(meshNode); });
			menu.addItem("Remove node", "Remove this node", [meshNode, this]() { removeNode(meshNode); });
			menu.addSeparator();
			menu.addItem("Add mesh instance", "Add a new mesh instance", [meshNode, this]() { addInstance(meshNode); });
			return;
		}

		case MeshNode::Type::Instance:
		{
			menu.addItem("Remove instance", "Remove this mesh instance", [meshNode, this]() { removeNode(meshNode); });
			menu.addSeparator();
			for (auto type : { sga::ObjectDefinition::ObjectType::PhysicsObject, sga::ObjectDefinition::ObjectType::CollisionObject, sga::ObjectDefinition::ObjectType::VisualObject })
			{
				bool present = (getChild(meshNode, SGAToNodeType(type)) != nullptr);
				auto label = (present ? "Modify " : "Add ") + SGATypeName(type);
				menu.addItem(label, label + (present ? " for" : " to") + " this object", [this, meshNode, type]() { addSGANode(meshNode, type); });
			}
			return;
		}
		}
	}

	auto sgaNode = dynamic_cast<SGANode*>(baseItem);
	if (!sgaNode)
		return;

	switch (sgaNode->nodeType)
	{
		case SGANode::Type::SGA_Root:
	{
		menu.addItem("Modify SGA root", "Change the type of the Sofa simulation", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::RootObject); });
		return;
	}

	case SGANode::Type::SGA_Physics:
	{
		menu.addItem("Modify physics", "Modify physics for this object", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::PhysicsObject); });
		menu.addItem("Remove physics", "Remove physics for this object", [sgaNode, this](){ m_graph.removeChild(sgaNode->parent, sgaNode); });
		return;
	}

	case SGANode::Type::SGA_Collision:
	{
		menu.addItem("Modify collision", "Modify collision for this object", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::CollisionObject); });
		menu.addItem("Remove collision", "Remove collision for this object", [sgaNode, this](){ m_graph.removeChild(sgaNode->parent, sgaNode); });
		return;
	}

	case SGANode::Type::SGA_Visual:
	{
		menu.addItem("Modify visual", "Modify visual for this object", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::VisualObject); });
		menu.addItem("Remove visual", "Remove visual for this object", [sgaNode, this](){ m_graph.removeChild(sgaNode->parent, sgaNode); });
		return;
	}

	case SGANode::Type::SGA_Modifier:
	{
		menu.addItem("Modify modifier", "Modify modifier for this object", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::ModifierObject); });
		menu.addItem("Remove modifier", "Remove modifier for this object", [sgaNode, this](){ m_graph.removeChild(sgaNode->parent, sgaNode); });
		return;
	}
	}
}

void SGADocument::importMesh()
{
	auto path = m_gui->getOpenFileName("Import mesh", "", "Supported files (*.3ds *.ac *.ase *.blend *.dae *.ifc *.lwo *.lws *.lxo *.ms3d *.obj *.ply *.stl *.x *.xgl *.zgl");
	if (path.empty())
		return;

	MeshImport importer(this, m_scene, m_graph);
	m_newMeshes = importer.importMeshes(path);
	m_graph.setRoot(m_rootNode); // Update the whole graph (TODO: update only the new nodes)
}

void SGADocument::addSGANode(GraphNode* parent, sga::ObjectDefinition::ObjectType type)
{
	auto dlg = m_gui->createDialog("Add SGA " + SGATypeName(type) + " object");
	auto& panel = dlg->content();

	int objectTypeId = 0;
	auto objectTypeProp = property::createRefProperty("Object type", objectTypeId, meta::Enum(SGAObjectsLabels(type)));
	panel.addProperty(objectTypeProp);
	
	if (dlg->exec())
	{
		int index;
		SGANode* prev;
		std::tie(index, prev) = indexOfNewNode(parent, type);

		auto objectType = SGAObjectId(type, objectTypeId);
		auto node = createNode(objectType, SGAToNodeType(type), parent, index);
		node->sgaDefinition = m_sgaFactory.definition(objectType);

		if (prev)
			m_graph.removeChild(parent, prev);
	}
}

void SGADocument::prepareSGAObjectsLists()
{
	const int nbSGATypes = 5;
	m_sgaObjectsLabels.resize(nbSGATypes);
	m_sgaObjectsIds.resize(nbSGATypes);
	for (int i = 0; i < nbSGATypes; ++i)
	{
		auto type = static_cast<sga::ObjectDefinition::ObjectType>(i);
		auto objects = m_sgaFactory.availableObjects(type);
		for (const auto& id : objects)
		{
			m_sgaObjectsIds[i].push_back(id);
			m_sgaObjectsLabels[i].push_back(m_sgaFactory.definition(id).label());
		}
	}
}

void SGADocument::createSGAGraphImages()
{
	m_graphSGAImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xffdedea4 }))); // SGA_Root
	m_graphSGAImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xffbebada }))); // SGA_Physics
	m_graphSGAImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xfffccde5 }))); // SGA_Collision
	m_graphSGAImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xffccebc5 }))); // SGA_Visual
	m_graphSGAImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xfffdb462 }))); // SGA_Modifier
}

void SGADocument::convertAndRun()
{
	m_gui->closeAllPropertiesDialogs();

	auto root = m_graph.root();
	if (!getChild(root, SGANode::Type::SGA_Root))
		addSGANode(root, sga::ObjectDefinition::ObjectType::RootObject);

	std::string dataPath = modulePath();
	m_execution = std::make_shared<SGAExecution>(m_scene, m_sgaFactory, dataPath);
	if (m_execution->convert(m_simulationProperties, m_rootNode.get()))
	{
		m_execution->run([this]() {
			m_gui->updateView();
		});
	}
	else
		m_execution.reset();
}

void SGADocument::stopExecution()
{
	if (m_execution) 
		m_execution->stop(); 
	m_execution.reset(); 
}
