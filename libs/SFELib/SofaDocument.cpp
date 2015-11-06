#include "SofaDocument.h"
#include "SofaProperties.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/SimpleGUI.h>

#include <sfe/sofaFrontEndLocal.h>
#include <sfe/Server.h>
#include <sfe/Helpers.h>
#include <sfe/DataTypeTrait.h>

#include <cctype>
#include <iostream>
#include <future>

// Register the types used in the SimpleRender lib so that SFE can directly copy to them
namespace sfe
{
template<> struct DataTypeTrait<glm::vec2> : public ArrayTypeTrait<glm::vec2, 2>{};
template<> struct DataTypeTrait<glm::vec3> : public ArrayTypeTrait<glm::vec3, 3>{};
}

SofaDocument::SofaDocument(const std::string& type, sfe::Simulation simulation)
	: BaseDocument(type)
	, m_mouseManipulator(m_scene)
	, m_graphImages(m_graph)
	, m_simulation(simulation)
{
}

void SofaDocument::initUI(simplegui::SimpleGUI& gui)
{
	m_gui = &gui;

	// Buttons box
	m_gui->buttonsPanel().addButton("Animate", "Pause or play the simulation", [this](){
		bool animating = m_simulation.isAnimating();
		m_simulation.setAnimate(!animating);
	});
	m_gui->buttonsPanel().addButton("Step", "Do a single step of the simulation", [this](){ singleStep(); }, 0, 1);

	m_gui->buttonsPanel().addButton("Reset", "Reset the simulation", [this](){ resetSimulation(); }, 1, 0);

	auto prop = property::createCopyProperty("Dt", 0.02f);
	m_gui->buttonsPanel().addProperty(prop, 1, 1);

	m_gui->buttonsPanel().addButton("Update graph", "Update the graph based on the current state of the simulation", [this](){ createGraph(); }, 2, 0, 1, 2);

	// Status bar
	m_statusFPS = m_gui->addStatusBarZone("FPS: 9999.9"); // Reasonable width for the fps counter
	m_gui->setStatusBarText(m_statusFPS, ""); // Set it to empty because we do not have the fps information yet
}

void SofaDocument::initOpenGL()
{
	m_scene.initOpenGL();
}

void SofaDocument::resize(int width, int height)
{
	m_scene.resize(width, height);
}

void SofaDocument::render()
{
	for (auto mesh : m_newMeshes)
		mesh->init();
	m_newMeshes.clear();

	if(m_updateObjects)
	{
		for(auto mesh : m_scene.meshes())
			mesh->updatePositions();
		m_updateObjects = false;
	}

	m_scene.render();
}

bool SofaDocument::mouseEvent(const MouseEvent& event)
{
	return m_mouseManipulator.mouseEvent(event);
}

void parseColor(std::istream& input, glm::vec4& color)
{
	for (int i = 0; i < 4; ++i)
		input >> color[i];
}

// Helper function to parse the material (a string) and extract the diffuse color
// The syntax of the material string is: MaterialName, Diffuse, DiffuseActive, DifuseColor(R,G,B,A), ...
simplerender::Material::SPtr parseMaterial(const std::string& materialText)
{
	auto material = std::make_shared<simplerender::Material>();
	std::istringstream in(materialText);
	std::string dummy, element;
	int active;
	in >> dummy;

	for (unsigned int i=0; i<7; ++i)
    {
        in  >>  element;
		std::transform(element.begin(), element.end(), element.begin(), std::tolower);

        if      (element == "diffuse")   { in  >> active; parseColor(in, material->diffuse);   }
        else if (element == "ambient")   { in  >> active; parseColor(in, material->ambient);   }
        else if (element == "specular")  { in  >> active; parseColor(in, material->specular);  }
        else if (element == "emissive")  { in  >> active; parseColor(in, material->emissive);  }
        else if (element == "shininess") { in  >> active; in >> material->shininess; }
		else if (element == "texture")   { std::getline(in, material->textureFilename); }
		else if (element == "bump")      { std::getline(in, dummy); }
    }
	parseColor(in, material->diffuse);

	return material;
}

std::string trim(const std::string& str)
{
	auto first = str.find_first_not_of(" \t\n\r");
	if (first == std::string::npos) // only whitespace
		return "";
	auto last = str.find_last_not_of(" \t\n\r");
	return str.substr(first, last - first + 1);
}

SofaDocument::SofaModel SofaDocument::createSofaModel(sfe::Object& visualModel)
{
	SofaModel sofaModel;
	sofaModel.m_sofaObject = visualModel; // Store the proxy to this object

	// Store every Data proxy we will use in the updateObjects function
	auto posData = visualModel.data("position");
	auto vertData = visualModel.data("vertices");

	// Determine if we use the position Data or the vertices Data
	std::vector<float> tmp;
	vertData.get(tmp);
	if(tmp.empty())
		sofaModel.d_vertices = posData;
	else
		sofaModel.d_vertices = posData;
	sofaModel.d_normals = visualModel.data("normal");

	if (!sofaModel.d_vertices || !sofaModel.d_normals)
		return sofaModel;

	auto mesh = std::make_shared<simplerender::Mesh>();
	sofaModel.mesh = mesh;
	m_newMeshes.push_back(mesh);

	sofaModel.d_vertices.get(mesh->m_vertices);
	sofaModel.d_normals.get(mesh->m_normals);

	// Get the constant information (topology and color)
	// Triangles
	auto trianglesData = visualModel.data("triangles");
	if (trianglesData)
		trianglesData.get(mesh->m_triangles);

	// Quads
	auto quadsData = visualModel.data("quads");
	if (quadsData)
		quadsData.get(mesh->m_quads);

	if (mesh->m_triangles.empty() && mesh->m_quads.empty())
		return sofaModel;

	// The material
	auto matData = visualModel.data("material");
	std::string materialText;
	matData.get(materialText);
	auto material = parseMaterial(materialText);
	sofaModel.material = material;
	m_scene.addMaterial(material);

	// Create an instance to put together the mesh and its material
	auto instance = std::make_shared<simplerender::ModelInstance>();
	instance->mesh = mesh;
	instance->material = material;
	m_scene.addInstance(instance);
	
	return sofaModel;
}

void SofaDocument::parseScene()
{
	auto rootNode = m_simulation.root(); // Get the root node of the simulation
	if (!rootNode) // Test if the root node is valid
		return;

	// We look for every Sofa object inheriting from the class VisualModelImpl in the graph
	auto visualModels = rootNode.findObjects("VisualModelImpl", {}, sfe::Node::SearchDirection::Down);
	for (auto& visualModel : visualModels)
	{
		auto sofaModel = createSofaModel(visualModel);

		if (sofaModel.mesh)
			m_scene.addMesh(sofaModel.mesh);
		m_sofaModels.push_back(sofaModel);
	}
}

void SofaDocument::setupCallbacks()
{
	sfe::SetAsynchronousCallbacks(true);
	m_sfeCallbacks.push_back(m_simulation.addCallback(sfe::Simulation::CallbackType::Step, [this](){ postStep(); }));
	m_sfeCallbacks.push_back(m_simulation.addCallback(sfe::Simulation::CallbackType::Reset, [this](){ postStep(); }));
}

void SofaDocument::updateObjects()
{
	for(auto sofaModel : m_sofaModels)
	{
		sofaModel.d_vertices.get(sofaModel.mesh->m_vertices);
		sofaModel.d_normals.get(sofaModel.mesh->m_normals);
	}
}

void SofaDocument::parseNode(GraphNode::SPtr parent, sfe::Node node)
{
	for(auto& object : node.objects())
		createNode(object, parent);

	for(auto& child : node.children())
	{
		auto n = createNode(child, parent);
		parseNode(n, child);
	}
}

GraphNode::SPtr SofaDocument::createNode(sfe::Object object, GraphNode::SPtr parent)
{
	auto n = SofaNode::create();
	n->name = object.name();
	n->type = object.className();
	n->uniqueId = object.uniqueId();
	n->parent = parent.get();
	n->isObject = true;
	n->object = object;
	n->expanded = false;
	m_graphImages.setImage(*n.get());
	if (parent)
		parent->children.push_back(n);

	// Parse slaves
	for(auto& slave : object.slaves())
	{
		auto s = createNode(slave, n);
		n->children.push_back(s);
	}

	return n;
}

GraphNode::SPtr SofaDocument::createNode(sfe::Node node, GraphNode::SPtr parent)
{
	auto n = SofaNode::create();
	n->name = node.name();
	n->uniqueId = node.uniqueId();
	n->parent = parent.get();
	n->isObject = false;
	n->node = node;
	m_graphImages.setImage(*n.get());
	if (parent)
		parent->children.push_back(n);

	return n;
}

void SofaDocument::createGraph()
{
	auto root = m_simulation.root();
	auto rootNode = createNode(root, nullptr);
	parseNode(rootNode, root);
	m_graph.setRoot(rootNode);
}

void SofaDocument::postStep()
{
	if(!m_singleStep)
	{
		const double fpsDuration = 0.5;
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> dur = now - m_fpsStart;
		double currentDuration = dur.count();
		if(currentDuration > fpsDuration)
		{
			double nbFPS = 0;
			if(m_fpsCount)
				nbFPS = m_fpsCount / currentDuration;

			m_fpsCount = 0;
			m_fpsStart = now;

			std::stringstream ss;
			ss << "FPS: ";
			ss.precision(1);
			ss << std::fixed << nbFPS;
			m_gui->setStatusBarText(m_statusFPS, ss.str());
		}
		++m_fpsCount;
	}

	// Udpate properties in opened dialogs
	std::async(&SofaDocument::updateProperties, this);

	updateObjects();
	m_updateObjects = true; // We have to modify the buffers in the correct thread
	m_gui->updateView();
}

ObjectProperties::SPtr SofaDocument::objectProperties(GraphNode* baseItem)
{
	auto item = dynamic_cast<SofaNode*>(baseItem);
	if(!item)
		return nullptr;

	ObjectProperties::SPtr properties;
	if(item->isObject)
		return createSofaObjectProperties(item->object);
	else
		return createSofaObjectProperties(item->node);
}

void SofaDocument::singleStep()
{
	auto start = std::chrono::high_resolution_clock::now();
	m_singleStep = true;
	m_simulation.step();
	m_singleStep = false;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> dur = end - start;
	auto fps = 1.0 / dur.count();
	std::stringstream ss;
	ss << "FPS: ";
	ss.precision(1);
	ss << std::fixed << fps;
	m_gui->setStatusBarText(m_statusFPS, ss.str());
}

void SofaDocument::resetSimulation()
{
	bool animating = m_simulation.isAnimating();
	if(animating)
		m_simulation.setAnimate(false, true); // We want to wait until the current step has finished
	m_gui->closeAllPropertiesDialogs();
	m_simulation.reset();
	createGraph();
	m_fpsCount = 1;
	m_fpsStart = std::chrono::high_resolution_clock::now();
	if(animating)
		m_simulation.setAnimate(true);
}


void SofaDocument::updateProperties()
{
	auto dialogs = m_gui->getOpenedPropertiesDialogs();
	for(auto& dlg : dialogs)
	{
		dlg.second->updateProperties();
		dlg.second->modified();
	}
}
