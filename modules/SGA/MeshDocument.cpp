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
	static std::vector<std::string> typesNames = { "Root", "Node", "Mesh", "Material", "Instance", "MeshesGroup", "MaterialsGroup" };
	return typesNames;
}

const std::string& getTypeName(MeshNode::Type type)
{
	return meshNodeTypeNames()[static_cast<int>(type)];
}

graph::GraphNodes getNodes(GraphNode* root, MeshNode::Type type)
{
	return graph::getNodes(root, [type](GraphNode* baseNode){
		auto node = dynamic_cast<MeshNode*>(baseNode);
		return node && node->nodeType == type;
	});
}

std::vector<MeshNode*> convertToMeshNodes(const graph::GraphNodes& nodes)
{
	std::vector<MeshNode*> meshNodes;
	for (const auto& node : nodes)
	{
		auto meshNode = dynamic_cast<MeshNode*>(node);
		if (meshNode)
			meshNodes.push_back(meshNode);
	}
	return meshNodes;
}

std::vector<std::string> getNames(GraphNode* root, MeshNode::Type type)
{
	auto nodes = getNodes(root, type);

	std::vector<std::string> names;
	for (auto& node : nodes)
		names.push_back(node->name);
	return names;
}

std::string createNewName(GraphNode* root, MeshNode::Type type, const std::string& prefix)
{
	auto nodes = getNodes(root, type);
	std::vector<std::string> names;
	for (const auto& node : nodes)
		names.push_back(node->name);

	int i = nodes.size() + 1;
	std::string name = prefix + std::to_string(i);
	while (names.end() != std::find(names.begin(), names.end(), name))
	{
		++i;
		name = prefix + std::to_string(i);
	}

	return name;
}

template <class C, class T>
void removeValue(C& container, const T& val)
{
	auto it = std::find(container.begin(), container.end(), val);
	if (it != container.end())
		container.erase(it);
}

template <class C, class T>
int indexOf(C& container, const T& val)
{
	auto it = std::find(container.begin(), container.end(), val);
	if (it == container.end())
		return -1;
	else
		return std::distance(container.begin(), it);
}

}

TransformationComponents toTransformationComponents(const glm::mat4& matrix)
{
	TransformationComponents transformation;
	auto modelTrans = glm::transpose(matrix);
	glm::vec3 scale = { 1, 1, 1 }, translation, skew;
	glm::vec4 perspective;
	glm::quat orientation;
	if (glm::decompose(modelTrans, scale, orientation, translation, skew, perspective))
	{
		// I have wrong values for the orientation, so I recompute them in an other way
		// First removing the scaling
		for (int i = 0; i < 3; ++i)
		{
			if (scale[i])
			{
				for (int j = 0; j < 3; ++j)
					modelTrans[i][j] /= scale[i];
			}
				
		}
		// Then using the conversion inside the quaternion constructor
		orientation = glm::quat(modelTrans);
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
	m_meshesGroup = createNode("Meshes", MeshNode::Type::MeshesGroup, m_rootNode.get()).get();
	m_materialsGroup = createNode("Materials", MeshNode::Type::MaterialsGroup, m_rootNode.get()).get();
}

bool MeshDocument::loadFile(const std::string& path)
{
	MeshImport importer(this, m_scene, m_graph);
	std::tie(m_newMeshes, m_newMaterials) = importer.importMeshes(path);
	return true;
}

bool MeshDocument::saveFile(const std::string& path)
{
	return false;
}

void MeshDocument::initUI(simplegui::SimpleGUI& gui)
{
	m_gui = &gui;
	m_graph.setRoot(m_rootNode);

	auto& toolsMenu = gui.getMenu(simplegui::MenuType::Tools);
	toolsMenu.addItem("Remove duplicate meshes", "Remove meshes that are identical to each other", [this](){ removeDuplicateMeshes(); });
	toolsMenu.addItem("Remove unused meshes", "Remove meshes that have no instance", [this](){ removeUnusedMeshes(); });
	toolsMenu.addItem("Remove unused materials", "Remove materials that have no instance", [this](){ removeUnusedMaterials(); });
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
	for (auto mesh : m_newMeshes)
		mesh->init();
	m_newMeshes.clear();

	for (auto material : m_newMaterials)
		material->init();
	m_newMaterials.clear();

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
		auto mesh = std::make_shared<simplerender::Mesh>();
		m_scene.addMesh(mesh);
		node->mesh = mesh;
		m_newMeshes.push_back(mesh.get());
	}
	else if (meshType == MeshNode::Type::Material)
	{
		auto material = std::make_shared<simplerender::Material>();
		m_scene.addMaterial(material);
		node->material = material;
		m_newMaterials.push_back(material.get());
	}
	else if (meshType == MeshNode::Type::Instance)
	{
		auto instance = std::make_shared<simplerender::ModelInstance>();
		m_scene.addInstance(instance);
		node->instance = instance;
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
		properties->createRefProperty("translation", item->transformationComponents.translation);
		properties->createRefProperty("rotation", item->transformationComponents.rotation);
		properties->createRefProperty("scale", item->transformationComponents.scale);

		auto bb = simplerender::boundingBox(m_scene);
		auto sceneSize = bb.second - bb.first;
		properties->createCopyProperty("Scene size", sceneSize)->setReadOnly(true);
		break;
	}

	case MeshNode::Type::Node:
	{
		properties->createRefProperty("name", item->name);
		properties->createRefProperty("translation", item->transformationComponents.translation);
		properties->createRefProperty("rotation", item->transformationComponents.rotation);
		properties->createRefProperty("scale", item->transformationComponents.scale);
		break;
	}

	case MeshNode::Type::Mesh:
	{
		properties->createRefProperty("name", item->name);
		auto mesh = item->mesh;
		if (mesh)
		{
			properties->createRefProperty("vertices", mesh->m_vertices);
			properties->createRefProperty("edges", mesh->m_edges);
			properties->createRefProperty("triangles", mesh->m_triangles);
			properties->createRefProperty("normals", mesh->m_normals);
			properties->createRefProperty("UV", mesh->m_texCoords);
		}
		break;
	}

	case MeshNode::Type::Material:
	{
		properties->createRefProperty("name", item->name);
		auto material = item->material;
		if (material)
		{
			properties->createRefProperty("diffuse", material->diffuse, meta::Color());
			properties->createRefProperty("ambient", material->ambient, meta::Color());
			properties->createRefProperty("specular", material->specular, meta::Color());
			properties->createRefProperty("emissive", material->emissive, meta::Color());
			properties->createRefProperty("shininess", material->shininess);
			properties->createRefProperty("texture", material->textureFilename, meta::File());
		}
		break;
	}

	case MeshNode::Type::Instance:
	{
		properties->createRefProperty("name", item->name);
		properties->createRefProperty("mesh id", item->meshId, meta::Enum(getNames(m_rootNode.get(), MeshNode::Type::Mesh)));
		properties->createRefProperty("material id", item->materialId, meta::Enum(getNames(m_rootNode.get(), MeshNode::Type::Material)));
		properties->createRefProperty("transformation", item->transformationMatrix)->setReadOnly(true);
		break;
	}
	}

	return properties;
}

void MeshDocument::closeObjectProperties(GraphNode* baseItem, ObjectPropertiesPtr ptr, bool accepted)
{
	if (!accepted)
		return;

	auto item = dynamic_cast<MeshNode*>(baseItem);
	if (!item)
		return;

	if (item->nodeType == MeshNode::Type::Root || item->nodeType == MeshNode::Type::Node || item->nodeType == MeshNode::Type::Instance)
		updateNodes(item);
	else if (item->nodeType == MeshNode::Type::Mesh)
		m_newMeshes.push_back(item->mesh.get());
	else if (item->nodeType == MeshNode::Type::Material)
		m_newMaterials.push_back(item->material.get());
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
		menu.addItem("Add node", "Add a new child node", [item, this]() { addNode(item); });
		menu.addItem("Remove node", "Remove this node", [item, this]() { removeNode(item); });
		menu.addSeparator();
		menu.addItem("Add mesh instance", "Add a new mesh instance", [item, this]() { addInstance(item); });
		break;

	case MeshNode::Type::Instance:
		menu.addItem("Remove instance", "Remove this mesh instance", [item, this]() { removeInstance(item); });
		break;

	case MeshNode::Type::MeshesGroup:
		menu.addItem("Add mesh", "Add a new mesh", [this]() { addMesh(); });
		break;

	case MeshNode::Type::MaterialsGroup:
		menu.addItem("Add material", "Add a new material", [this]() { addMaterial(); });
		break;
	}
}

void MeshDocument::createGraphImages()
{
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createDiskImage(0xffdedede))); // Root
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createDiskImage(0xffdedede))); // Node
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xff00daff }))); // Mesh
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xffffa4a4 }))); // Material
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createSquaresImage({ 0xff80b1d3 }))); // Instance
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createDiskImage({ 0xff00daff }))); // Meshes group
	m_graphMeshImages.push_back(m_graph.addImage(GraphImage::createDiskImage({ 0xffffa4a4 }))); // Materials group
}

void MeshDocument::updateNodes(MeshNode* item, const glm::mat4* transformation)
{
	glm::mat4 accTrans;
	if(transformation)
		accTrans = *transformation;

	if (item->nodeType == MeshNode::Type::Root)
	{
		accTrans = item->transformationMatrix * accTrans;
	}
	else if(item->nodeType == MeshNode::Type::Node)
	{
		glm::mat4 transMat = toTransformationMatrix(item->transformationComponents);
		accTrans = transMat * accTrans;
		item->transformationMatrix = transMat; // Local transformation
	}
	else if (item->nodeType == MeshNode::Type::Instance)
	{
		auto mesh = item->meshId != -1 ? m_scene.meshes()[item->meshId] : nullptr;
		auto material = item->materialId != -1 ? m_scene.materials()[item->materialId] : nullptr;
		item->mesh = mesh;
		item->material = material;
		if (transformation)
		{
			item->transformationMatrix = *transformation; // Global transformation
			item->instance->transformation = glm::transpose(*transformation);
		}
		item->instance->mesh = mesh;
		item->instance->material = material;
		
	}

	for (auto child : item->children)
	{
		auto node = dynamic_cast<MeshNode*>(child.get());
		if (node)
			updateNodes(node, &accTrans);
	}
}

void MeshDocument::addNode(MeshNode* parent)
{
	createNode(createNewName(m_rootNode.get(), MeshNode::Type::Node, "Node "), MeshNode::Type::Node, parent);
}

void MeshDocument::removeNode(MeshNode* item)
{
	if (item->parent)
	{
		// Remove the child instances
		auto instanceNodes = convertToMeshNodes(getNodes(item, MeshNode::Type::Instance));
		for(auto node : instanceNodes)
			removeValue(m_scene.instances(), node->instance);

		// Remove the node
		m_graph.removeChild(item->parent, item);

		// Update the view if any changes were made
		if(!instanceNodes.empty())
			m_gui->updateView();
	}
}

void MeshDocument::addInstance(MeshNode* parent)
{
	auto node = createNode(createNewName(m_rootNode.get(), MeshNode::Type::Instance, "Instance "), MeshNode::Type::Instance, parent);
	auto instance = std::make_shared<simplerender::ModelInstance>();
	node->instance = instance;
	m_scene.addInstance(instance);
	updateNodes(parent);
}

void MeshDocument::removeInstance(MeshNode* item)
{
	removeValue(m_scene.instances(), item->instance);
	removeNode(item);
	m_gui->updateView();
}

void MeshDocument::addMesh()
{
	auto root = m_rootNode.get();
	auto node = createNode(createNewName(root, MeshNode::Type::Mesh, "Mesh "), MeshNode::Type::Mesh, m_meshesGroup);
	auto mesh = std::make_shared<simplerender::Mesh>();
	node->mesh = mesh;
	m_scene.addMesh(mesh);
}

void MeshDocument::addMaterial()
{
	auto root = m_rootNode.get();
	auto node = createNode(createNewName(root, MeshNode::Type::Material, "Material "), MeshNode::Type::Material, m_materialsGroup);
	auto material = std::make_shared<simplerender::Material>();
	node->material = material;
	m_scene.addMaterial(material);
}

void MeshDocument::removeDuplicateMeshes()
{
	using MeshPtr = simplerender::Mesh::SPtr;
	std::vector<MeshPtr> usedMeshes, duplicatedMeshes;
	using Remplacement = std::pair<MeshPtr, MeshPtr>;
	std::vector<Remplacement> replacements;

	// Find duplicated meshes
	for (auto& mesh : m_scene.meshes())
	{
		bool ignore = false;
		for (auto usedMesh : usedMeshes)
		{
			if (*mesh == *usedMesh)
			{
				ignore = true;
				duplicatedMeshes.push_back(mesh);
				replacements.emplace_back(mesh, usedMesh);
			}
		}

		if (!ignore)
			usedMeshes.push_back(mesh);
	}

	// Test if we have something to modify
	if (duplicatedMeshes.empty())
		return;

	// Modify the scene
	m_scene.meshes() = usedMeshes;

	// Remove the duplicated meshes nodes present in the graph
	auto meshNodes = convertToMeshNodes(getNodes(m_rootNode.get(), MeshNode::Type::Mesh));
	for (auto& meshNode : meshNodes)
	{
		auto mesh = meshNode->mesh;
		auto itMesh = std::find(duplicatedMeshes.begin(), duplicatedMeshes.end(), mesh);

		if (itMesh != duplicatedMeshes.end())
			m_graph.removeChild(meshNode->parent, meshNode);
	}

	// Modify the instances
	auto instanceNodes = convertToMeshNodes(getNodes(m_rootNode.get(), MeshNode::Type::Instance));
	for (auto& instanceNode : instanceNodes)
	{
		auto instance = instanceNode->instance.get();
		if (!instance)
			continue;
		auto mesh = instance->mesh;
		auto it = std::find_if(replacements.begin(), replacements.end(), [mesh](const Remplacement& r) {
			return r.first == mesh;
		});

		if (it != replacements.end())
		{
			auto mesh = it->second;
			instance->mesh = mesh;
			instanceNode->mesh = mesh;
			instanceNode->meshId = indexOf(m_scene.meshes(), mesh);
		}
	}
}

void MeshDocument::removeUnusedMeshes()
{
	// Get the list of the instances
	auto instanceNodes = convertToMeshNodes(getNodes(m_rootNode.get(), MeshNode::Type::Instance));
	std::vector<simplerender::Mesh*> instancedMeshes;
	for (auto& instanceNode : instanceNodes)
		instancedMeshes.push_back(instanceNode->mesh.get());

	// Create a unique list of used meshes
	std::sort(instancedMeshes.begin(), instancedMeshes.end());
	auto lastInstanced = std::unique(instancedMeshes.begin(), instancedMeshes.end());
	if (lastInstanced != instancedMeshes.end())
		instancedMeshes.erase(lastInstanced, instancedMeshes.end());

	// Test if a mesh is used
	auto isUnused = [&instancedMeshes](const simplerender::Mesh::SPtr& mesh){
		auto meshPtr = mesh.get();
		return instancedMeshes.end() == std::find(instancedMeshes.begin(), instancedMeshes.end(), meshPtr);
	};

	// Copy the unused meshes
	auto& meshes = m_scene.meshes();
	simplerender::Scene::Meshes unusedMeshes;
	std::copy_if(meshes.begin(), meshes.end(), std::back_inserter(unusedMeshes), isUnused);

	// Iterate over unused meshes
	if (!unusedMeshes.empty())
	{
		auto meshNodes = convertToMeshNodes(getNodes(m_rootNode.get(), MeshNode::Type::Mesh));
		for (const auto& mesh : unusedMeshes)
		{
			// If there is a mesh node in the graph, remove it
			auto itMesh = std::find_if(meshNodes.begin(), meshNodes.end(), [&mesh](MeshNode* node){
				return node->mesh == mesh;
			});

			if (itMesh != meshNodes.end())
			{
				auto meshNode = *itMesh;
				m_graph.removeChild(meshNode->parent, meshNode);
			}
		}

		// Modifiy the scene's meshes list
		auto last = std::remove_if(meshes.begin(), meshes.end(), isUnused);
		meshes.erase(last, meshes.end());
	}
}

void MeshDocument::removeUnusedMaterials()
{
	// Get the list of the instances
	auto instanceNodes = convertToMeshNodes(getNodes(m_rootNode.get(), MeshNode::Type::Instance));
	std::vector<simplerender::Material*> instancedMaterials;
	for (auto& instanceNode : instanceNodes)
	{
		if (instanceNode->material)
			instancedMaterials.push_back(instanceNode->material.get());
	}

	// Create a unique list of used materials
	std::sort(instancedMaterials.begin(), instancedMaterials.end());
	auto lastInstanced = std::unique(instancedMaterials.begin(), instancedMaterials.end());
	if (lastInstanced != instancedMaterials.end())
		instancedMaterials.erase(lastInstanced, instancedMaterials.end());

	// Test if a material is used
	auto isUnused = [&instancedMaterials](const simplerender::Material::SPtr& material){
		auto materialPtr = material.get();
		return instancedMaterials.end() == std::find(instancedMaterials.begin(), instancedMaterials.end(), materialPtr);
	};

	// Copy the unused materials
	auto& materials = m_scene.materials();
	simplerender::Scene::Materials unusedMaterials;
	std::copy_if(materials.begin(), materials.end(), std::back_inserter(unusedMaterials), isUnused);

	// Iterate over unused materials
	if (!unusedMaterials.empty())
	{
		auto materialNodes = convertToMeshNodes(getNodes(m_rootNode.get(), MeshNode::Type::Material));
		for (const auto& material : unusedMaterials)
		{
			// If there is a material node in the graph, remove it
			auto itMaterial = std::find_if(materialNodes.begin(), materialNodes.end(), [&material](MeshNode* node){
				return node->material == material;
			});

			if (itMaterial != materialNodes.end())
			{
				auto materialNode = *itMaterial;
				m_graph.removeChild(materialNode->parent, materialNode);
			}
		}

		// Modifiy the scene's materials list
		auto last = std::remove_if(materials.begin(), materials.end(), isUnused);
		materials.erase(last, materials.end());
	}
}
