#include "Document.h"
#include "SGAExecution.h"
#include "SGAProperties.h"
#include "MeshImport.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/PropertiesUtils.h>
#include <core/SimpleGUI.h>

#include <serialization/DocXML.h>

#include <iostream>

int SofaGraphAbstractionDoc = RegisterDocument<Document>("Sofa Graph Abstraction")
	.setDescription("Create Sofa simulations using higher level objects.")
	.canCreateNew(true)
	.addLoadFile("SGA document (*.sga)")
	.addSaveFile("SGA document (*.sga)");
ModuleHandle SofaGraphAbstractionModule = RegisterModule("SofaGraphAbstraction").addDocument(SofaGraphAbstractionDoc);

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

const std::vector<std::string>& nodeTypeNames()
{
	static std::vector<std::string> typesNames = { "Root", "Node", "Mesh", "Instance", "SGA root", "SGA physics", "SGA collision", "SGA visual", "SGA modifier" };
	return typesNames;
}

const std::string& getTypeName(SGANode::Type type)
{
	return nodeTypeNames()[static_cast<int>(type)];
}

SGANode::Type getNodeType(const std::string& type)
{
	auto& names = nodeTypeNames();
	auto it = std::find(names.begin(), names.end(), type);
	if (it == names.end())
		return SGANode::Type::Node;
	int index = std::distance(names.begin(), it);
	return static_cast<SGANode::Type>(index);
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

bool isSGANode(SGANode::Type type)
{
	if (type == SGANode::Type::SGA_Root ||
		type == SGANode::Type::SGA_Physics ||
		type == SGANode::Type::SGA_Collision ||
		type == SGANode::Type::SGA_Visual ||
		type == SGANode::Type::SGA_Modifier)
		return true;
	return false;
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
		auto firstNode = getChild(parent, SGANode::Type::Node);
		if (firstNode)
			return std::make_pair(indexOfChild(parent, firstNode), nullptr); // Add before the first node
		return std::make_pair(-1, nullptr); // Add at the end
	}
	}

	return std::make_pair(-1, nullptr);
}

}

Document::Document(const std::string& type)
	: BaseDocument(type)
	, m_mouseManipulator(m_scene)
	, m_sgaFactory(modulePath() + "/definitions")
{
	prepareSGAObjectsLists();
	createGraphImages();

	m_simulationProperties.gravity = { 0, -9.81, 0 };
}

bool Document::loadFile(const std::string& path)
{
	auto createNodeFunc = [this](const std::string& typeName, const std::string& id) 
	{
		auto type = getNodeType(typeName);
		auto node = createNode(id, type, nullptr);
		if (isSGANode(type))
			node->sgaDefinition = m_sgaFactory.definition(id);
		else if (type == SGANode::Type::Mesh)
		{
			auto model = std::make_shared<simplerender::Model>();
			m_scene.addModel(model);
			node->model = model;
			m_newModels.push_back(node->model.get());
		}
		return node;
	};

	auto getPropertiesFunc = [this](GraphNode* item) { return objectProperties(item); };

	auto node = importXMLFile(path, createNodeFunc, getPropertiesFunc);
	if (!node)
		return false;

	m_rootNode = std::dynamic_pointer_cast<SGANode>(node);
	m_graph.setRoot(m_rootNode);
	updateInstances();
	return true;
}

bool Document::saveFile(const std::string& path)
{
	auto getPropertiesFunc = [this](GraphNode* item) { return objectProperties(item); };
	return exportToXMLFile(path, m_rootNode.get(), getPropertiesFunc);
}

void Document::initUI(simplegui::SimpleGUI& gui)
{
	m_gui = &gui;
	m_gui->getMenu(simplegui::SimpleGUI::MenuType::Tools).addItem("Import mesh", "Import a scene or a mesh", [this](){ importMesh(); });

	auto& panel = m_gui->buttonsPanel();
	panel.addButton("Run", "Convert to a Sofa simulation and run it", [this](){ 
		if (!m_execution)
			convertAndRun();
	});

	panel.addButton("Stop", "Stop the Sofa simulation", [this](){ 
		if (m_execution) 
			m_execution->stop(); 
		m_execution.reset(); 
	});

	m_rootNode = createNode("SGA scene", SGANode::Type::Root, nullptr);
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
	if (m_execution)
	{
		m_execution->render();
		return;
	}

	for (auto model : m_newModels)
		model->init();
	m_newModels.clear();

	m_scene.render();
}

bool Document::mouseEvent(const MouseEvent& event)
{
	return m_mouseManipulator.mouseEvent(event);
}

SGANode::SPtr Document::createNode(const std::string& name, SGANode::Type nodeType, GraphNode* parent, int position)
{
	auto node = SGANode::create();
	node->name = name;
	node->type = getTypeName(nodeType);
	node->nodeType = nodeType;
	node->parent = parent;
	node->uniqueId = m_nextNodeId++;
	node->imageId = m_graphImages[static_cast<int>(nodeType)];
	if (parent)
		m_graph.insertChild(parent, node, position);

	return node;
}

Document::ObjectPropertiesPtr Document::objectProperties(GraphNode* baseItem)
{
	auto item = dynamic_cast<SGANode*>(baseItem);
	if(!item)
		return nullptr;

	switch (item->nodeType)
	{
	case SGANode::Type::Root:
	{
		auto properties = std::make_shared<ObjectProperties>(item->name);
		properties->createPropertyAndWrapper("gravity", m_simulationProperties.gravity);
		properties->createPropertyAndWrapper("timestep", m_simulationProperties.timestep);
		populateProperties(item, m_scene, properties.get());
		return properties;
	}

	case SGANode::Type::Node:
	case SGANode::Type::Mesh:
	case SGANode::Type::Instance:
	{
		auto properties = std::make_shared<ObjectProperties>(item->name);
		populateProperties(item, m_scene, properties.get());
		return properties;
	}

	case SGANode::Type::SGA_Root:
	case SGANode::Type::SGA_Physics:
	case SGANode::Type::SGA_Collision:
	case SGANode::Type::SGA_Visual:
	case SGANode::Type::SGA_Modifier:
	{
		return createSGAObjectProperties(item->sgaDefinition);
	}
	}

	return nullptr;
}

void Document::closeObjectProperties(GraphNode* baseItem, ObjectPropertiesPtr ptr, bool accepted)
{
	auto item = dynamic_cast<SGANode*>(baseItem);
	if (!item)
		return;

	if (item->nodeType == SGANode::Type::Root || item->nodeType == SGANode::Type::Node)
		updateInstances();
}

void Document::graphContextMenu(GraphNode* baseItem, simplegui::Menu& menu)
{
	auto item = dynamic_cast<SGANode*>(baseItem);
	if (!item)
		return;

	SGANode* parent = nullptr;
	if (item->parent)
		parent = dynamic_cast<SGANode*>(item->parent);

	switch (item->nodeType)
	{
	case SGANode::Type::Root:
	{
		menu.addItem("Set SGA root node", "Change the type of the Sofa simulation", [item, this](){ addSGANode(item, sga::ObjectDefinition::ObjectType::RootObject); });
		return;
	}

	case SGANode::Type::Instance:
	{
		for (auto type : { sga::ObjectDefinition::ObjectType::PhysicsObject, sga::ObjectDefinition::ObjectType::CollisionObject, sga::ObjectDefinition::ObjectType::VisualObject })
		{
			bool present = (getChild(item, SGAToNodeType(type)) != nullptr);
			auto label = (present ? "Modify " : "Add " ) + SGATypeName(type);
			menu.addItem(label, label + (present ? " for" : " to" ) + " this object", [this, item, type](){ addSGANode(item, type); });
		}
		return;
	}

	case SGANode::Type::SGA_Root:
	{
		menu.addItem("Modify SGA root", "Change the type of the Sofa simulation", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::RootObject); });
		return;
	}

	case SGANode::Type::SGA_Physics:
	{
		menu.addItem("Modify physics", "Modify physics for this object", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::PhysicsObject); });
		menu.addItem("Remove physics", "Remove physics for this object", [item, this](){ m_graph.removeChild(item->parent, item); });
		return;
	}

	case SGANode::Type::SGA_Collision:
	{
		menu.addItem("Modify collision", "Modify collision for this object", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::CollisionObject); });
		menu.addItem("Remove collision", "Remove collision for this object", [item, this](){ m_graph.removeChild(item->parent, item); });
		return;
	}

	case SGANode::Type::SGA_Visual:
	{
		menu.addItem("Modify visual", "Modify visual for this object", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::VisualObject); });
		menu.addItem("Remove visual", "Remove visual for this object", [item, this](){ m_graph.removeChild(item->parent, item); });
		return;
	}

	case SGANode::Type::SGA_Modifier:
	{
		menu.addItem("Modify modifier", "Modify modifier for this object", [parent, this](){ addSGANode(parent, sga::ObjectDefinition::ObjectType::ModifierObject); });
		menu.addItem("Remove modifier", "Remove modifier for this object", [item, this](){ m_graph.removeChild(item->parent, item); });
		return;
	}
	}
}

void Document::importMesh()
{
	auto path = m_gui->getOpenFileName("Import mesh", "", "Supported files (*.3ds *.ac *.ase *.blend *.dae *.ifc *.lwo *.lws *.lxo *.ms3d *.obj *.ply *.stl *.x *.xgl *.zgl");
	if (path.empty())
		return;

	MeshImport importer(this, m_scene, m_graph);
	m_newModels = importer.importMeshes(path);
	m_graph.setRoot(m_rootNode); // Update the whole graph (TODO: update only the new nodes)
}

void Document::addSGANode(GraphNode* parent, sga::ObjectDefinition::ObjectType type)
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

void Document::prepareSGAObjectsLists()
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

void Document::createGraphImages()
{
	m_graphImages.push_back(m_graph.addImage(GraphImage::createDiskImage(0xffdedede))); // Root
	m_graphImages.push_back(m_graph.addImage(GraphImage::createDiskImage(0xffdedede))); // Node
	m_graphImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xff00daff }))); // Mesh
	m_graphImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xff80b1d3 }))); // Instance
	m_graphImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xffdedea4 }))); // SGA_Root
	m_graphImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xffbebada }))); // SGA_Physics
	m_graphImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xfffccde5 }))); // SGA_Collision
	m_graphImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xffccebc5 }))); // SGA_Visual
	m_graphImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xfffdb462 }))); // SGA_Modifier
}

void Document::convertAndRun()
{
	auto root = m_graph.root();
	if (!getChild(root, SGANode::Type::SGA_Root))
		addSGANode(root, sga::ObjectDefinition::ObjectType::RootObject);

	std::string dataPath = modulePath();
	m_execution = std::make_shared<SGAExecution>(m_scene, m_sgaFactory, dataPath);
	m_execution->convert(m_simulationProperties,  m_rootNode.get());
	m_execution->run([this](){
		m_gui->updateView();
	});
}

void Document::updateInstances()
{
	m_scene.instances().clear();
	glm::mat4 transformation;
	updateInstances(dynamic_cast<SGANode*>(m_rootNode.get()), transformation);
}

void Document::updateInstances(SGANode* item, const glm::mat4& transformation)
{
	auto accTrans = transformation;
	if (item->nodeType == SGANode::Type::Root || item->nodeType == SGANode::Type::Node)
	{
		accTrans = item->transformation * transformation;
	}
	else if (item->nodeType == SGANode::Type::Instance)
	{
		item->transformation = transformation;
		auto model = m_scene.models()[item->meshId];
		item->model = model;
		m_scene.addInstance({ glm::transpose(transformation), model });
	}

	for (auto child : item->children)
		updateInstances(dynamic_cast<SGANode*>(child.get()), accTrans);
}
