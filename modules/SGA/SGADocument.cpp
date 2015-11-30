#include "SGADocument.h"
#include "SGAExecution.h"
#include "SGAProperties.h"
#include "MeshImport.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/PropertiesUtils.h>

#include <serialization/DocXML.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <fstream>

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
		if (sgaNode && sgaNode->nodeType == type)
			return sgaNode;
	}

	return nullptr;
}

MeshNode* getChild(GraphNode* parent, MeshNode::Type type)
{
	for (auto child : parent->children)
	{
		auto meshNode = dynamic_cast<MeshNode*>(child.get());
		if (meshNode && meshNode->nodeType == type)
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

void copyFile(const std::string& from, const std::string& to)
{
	std::ifstream src(from, std::ios::binary);
	std::ofstream dst(to, std::ios::binary);
	dst << src.rdbuf();
}

std::string getDir(const std::string& filePath)
{
	auto pos = filePath.find_last_of("/\\");
	if (pos == std::string::npos)
		return "";
	return filePath.substr(0, pos);
}

std::string getFilename(const std::string& filePath)
{
	auto pos = filePath.find_last_of("/\\");
	if (pos == std::string::npos || pos + 1 == filePath.size())
		return "";
	return filePath.substr(pos + 1);
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

	auto root = importXMLFile(path, createNodeFunc, getPropertiesFunc);
	if (!root)
		return false;

	auto meshRoot = dynamic_cast<MeshNode*>(root.get());
	if (!meshRoot)
		return false;

	m_rootNode = root;
	m_graph.setRoot(m_rootNode);
	updateNodes(meshRoot);

	MeshImport::findTextures(m_newMaterials, path);

	// Get the meshes and materials groups nodes
	m_meshesGroup = getChild(meshRoot, MeshNode::Type::MeshesGroup);
	m_materialsGroup = getChild(meshRoot, MeshNode::Type::MaterialsGroup);

	return true;
}

bool SGADocument::saveFile(const std::string& path)
{
	auto texturePaths = modifyTexturesForSaving(path);
	auto getPropertiesFunc = [this](GraphNode* item) { return objectProperties(item); };
	auto result = exportToXMLFile(path, m_rootNode.get(), getPropertiesFunc);
	restoreTexturesPaths(texturePaths);
	return result;
}

void SGADocument::initUI(simplegui::SimpleGUI& gui)
{
	auto& toolsMenu = gui.getMenu(simplegui::MenuType::Tools);
	toolsMenu.addItem("Import mesh", "Import a scene or a mesh", [this](){ importMesh(); });
	toolsMenu.addItem("Export Sofa scene", "Convert to a Sofa simulation and export it", [this](){ convertAndExport(); });
	toolsMenu.addSeparator();

	auto& panel = gui.buttonsPanel();
	m_runButton = panel.addButton("Run", "Convert to a Sofa simulation and run it", [this](){ runClicked(); });
	m_runButton->setCheckable(true);

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
		properties->createRefProperty("gravity", m_simulationProperties.gravity)->setGroup("SGA");
		properties->createRefProperty("timestep", m_simulationProperties.timestep)->setGroup("SGA");
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
		MeshDocument::graphContextMenu(baseItem, menu);
		switch (meshNode->nodeType)
		{
		case MeshNode::Type::Root:
		{
			menu.addItem("Set SGA root node", "Change the type of the Sofa simulation", [meshNode, this]() { addSGANode(meshNode, sga::ObjectDefinition::ObjectType::RootObject); });
			return;
		}

		case MeshNode::Type::Instance:
		{
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
	std::tie(m_newMeshes, m_newMaterials) = importer.importMeshes(path);
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

void SGADocument::convertAndExport()
{
	m_gui->closeAllPropertiesDialogs();

	auto root = m_graph.root();
	if (!getChild(root, SGANode::Type::SGA_Root))
		addSGANode(root, sga::ObjectDefinition::ObjectType::RootObject);

	auto path = m_gui->getSaveFileName("Sofa scene exportation", "ExportSofaScene.scn", "Sofa scenes (*.scn)");
	if (path.empty())
		return;

	std::string dataPath = modulePath();
	SGAExecution execution(m_scene, m_sgaFactory, dataPath);
	if (execution.convert(m_simulationProperties, m_rootNode.get()))
		execution.exportScene(path);
}

void SGADocument::stopExecution()
{
	if (m_execution) 
		m_execution->stop(); 
	m_execution.reset(); 
}

void SGADocument::runClicked()
{
	if (!m_execution)
	{
		convertAndRun();
		if (m_execution) // Will be null if the conversion failed
			m_runButton->setChecked(true);
	}
	else
	{
		stopExecution();
		m_runButton->setChecked(false);
	}
}

std::vector<std::string> SGADocument::modifyTexturesForSaving(const std::string& savePath)
{
	std::vector<std::string> texPaths;

	// Find the repertory of the saved file
	std::string dir = getDir(savePath);
	if (dir.empty()) 
		return texPaths;

	// TODO: ask the user how we want to treat textures (copy or keep where they are)
//	enum class TexAction { Unknown, Copy, Keep };
//	TexAction action = TexAction::Unknown;

	for (auto& material : m_scene.materials())
	{
		auto& texPath = material->textureFilename;
		if (!texPath.empty())
		{
			texPaths.push_back(texPath);
			if (texPath.find(dir) != std::string::npos) // The texture can stay there, we can compute the relative path
			{
				texPath = texPath.substr(dir.size() + 1); // Removing the directory path, including the '/'
			}
			else // We must move the texture to be able to create a relative path
			{
				auto texName = getFilename(texPath);
				copyFile(texPath, dir + '/' + texName);
				texPath = texName;
			}
		}
	}

	return texPaths;
}

void SGADocument::restoreTexturesPaths(const std::vector<std::string>& paths)
{
	auto it = paths.begin();
	for (auto& material : m_scene.materials())
	{
		auto& texPath = material->textureFilename;
		if (!texPath.empty())
			texPath = *it++;
	}
}
