#pragma once

#include <core/BaseDocument.h>
#include <core/Graph.h>
#include <core/MouseManipulator.h>

#include <render/Scene.h>
#include <sfe/Simulation.h>

struct TransformationComponents
{
	TransformationComponents(glm::vec3 t, glm::vec3 r, glm::vec3 s) : translation(t), rotation(r), scale(s) {}
	TransformationComponents() : scale(1, 1, 1) {}

	glm::vec3 translation, rotation, scale;
};

TransformationComponents toTransformationComponents(const glm::mat4& matrix);
glm::mat4 toTransformationMatrix(const TransformationComponents& transformation);

class MeshNode : public GraphNode
{
public:
	using SPtr = std::shared_ptr<MeshNode>;
	enum class Type { Root, Node, Mesh, Material, Instance };

	static SPtr create() { return std::make_shared<MeshNode>(); }

	Type nodeType;

	glm::mat4 transformationMatrix; // Root & Node & Instance (Read only). This is for visualization only, row major.
	TransformationComponents transformationComponents; // Root & Node
	simplerender::Mesh::SPtr mesh; // Mesh & Instance
	simplerender::Material::SPtr material; // Material & Instance
	simplerender::ModelInstance::SPtr instance; // Instance
	int meshId = -1, materialId = -1; // Instance
};

//****************************************************************************//

int registerMeshDocument();

class MeshDocument : public BaseDocument
{
public:
	MeshDocument(const std::string& type);

	bool loadFile(const std::string& path) override;
	bool saveFile(const std::string& path) override;
	void initUI(simplegui::SimpleGUI& gui) override;

	void initOpenGL() override;
	void resize(int width, int height) override;
	void render() override;

	bool mouseEvent(const MouseEvent& event) override;

	Graph& graph() override;

	ObjectPropertiesPtr objectProperties(GraphNode* item) override;
	void closeObjectProperties(GraphNode* item, ObjectPropertiesPtr ptr, bool accepted) override;
	void graphContextMenu(GraphNode* item, simplegui::Menu& menu) override;

	MeshNode::SPtr createNode(const std::string& name, MeshNode::Type nodeType, GraphNode* parent, int position = -1);
	GraphNode::SPtr createNode(const std::string& typeName, const std::string& id); // For the loading of a document

	void removeDuplicateMeshes();
	void removeUnusedMeshes();

protected:
	void createGraphImages();

	void updateNodes(MeshNode* item, const glm::mat4* transformation = nullptr);

	void addNode(MeshNode* parent);
	void removeNode(MeshNode* item);
	void addInstance(MeshNode* parent);
	void removeInstance(MeshNode* item);

	simplegui::SimpleGUI* m_gui = nullptr;
	simplerender::Scene m_scene;
	Graph m_graph;
	SofaMouseManipulator m_mouseManipulator;

	GraphNode::SPtr m_rootNode;
	size_t m_nextNodeId = 1;
	std::vector<int> m_graphMeshImages;
	std::vector<simplerender::Mesh*> m_newMeshes;
	std::vector<simplerender::Material*> m_newMaterials;
};

inline Graph& MeshDocument::graph()
{ return m_graph; }
