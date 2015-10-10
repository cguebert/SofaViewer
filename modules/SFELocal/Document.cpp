#include "Document.h"
#include "SofaProperties.h"

#include <core/DocumentFactory.h>
#include <core/ObjectProperties.h>
#include <core/PropertiesUtils.h>
#include <core/SimpleGUI.h>

#include <sfe/sofaFrontEndLocal.h>
#include <sfe/Server.h>
#include <sfe/Helpers.h>
#include <sfe/DataTypeTrait.h>

#include <iostream>
#include <future>

int SFELocalDoc = RegisterDocument<Document>("SFELocalDoc").setDescription("Run Sofa scenes using Sofa Front End Local")
	.addLoadFile("Sofa scenes (*.scn)");
ModuleHandle SFELocalModule = RegisterModule("SFELocal").addDocument(SFELocalDoc);

// Register the types used in the SimpleRender lib so that SFE can directly copy to them
namespace sfe
{
template<> struct DataTypeTrait<glm::vec2> : public ArrayTypeTrait<glm::vec2, 2>{};
template<> struct DataTypeTrait<glm::vec3> : public ArrayTypeTrait<glm::vec3, 3>{};
}

Document::Document(ui::SimpleGUI& gui)
	: BaseDocument(gui)
	, m_gui(gui)
	, m_mouseManipulator(m_scene)
	, m_graphImages(m_graph)
	, m_simulation(sfe::getLocalSimulation())
{
}

std::string Document::documentType()
{
	return "SFELocalDoc";
}

bool Document::loadFile(const std::string& path)
{
	// Read settings
	std::vector<std::string> sofaPaths;
	if(m_gui.settings().get("dataRepositoryPaths", sofaPaths))
		m_simulation.getHelper()->setDataRepositoryPaths(sofaPaths);

	m_simulation.setAnimate(false, true);
	if (!m_simulation.loadFile(path))
		return false;
	else
	{
	//	simulation.root().createObject("RequiredPlugin", { { "pluginName", "DtExtensions" } });
	//	mouseInteractor = simulation.root().createObject("DtMouseInteractor");

		// Initializes the scene
		m_simulation.init();

		// Create the models for rendering
		parseScene();

		// Setup the callbacks
		m_simulation.setCallback(sfe::Simulation::CallbackType::Step, [this](){ postStep(); });
		m_simulation.setCallback(sfe::Simulation::CallbackType::Reset, [this](){ postStep(); });

		return true;
	}
}

void Document::initUI()
{
	// Update the scene graph
	createGraph();

	// Buttons box
	m_gui.buttonsPanel().addButton("Animate", "Pause or play the simulation", [this](){
		bool animating = m_simulation.isAnimating();
		m_simulation.setAnimate(!animating);
	});
	m_gui.buttonsPanel().addButton("Step", "Do a single step of the simulation", [this](){ singleStep(); }, 0, 1);

	m_gui.buttonsPanel().addButton("Reset", "Reset the simulation", [this](){ resetSimulation(); }, 1, 0);

	auto prop = property::createCopyProperty("Dt", 0.02f);
	m_gui.buttonsPanel().addProperty(prop, 1, 1);

	m_gui.buttonsPanel().addButton("Update graph", "Update the graph based on the current state of the simulation", [this](){ createGraph(); }, 2, 0, 1, 2);

	// Status bar
	m_statusFPS = m_gui.addStatusBarZone("FPS: 9999.9"); // Reasonable width for the fps counter
	m_gui.setStatusBarText(m_statusFPS, ""); // Set it to empty because we do not have the fps information yet

	// Menu actions
	m_gui.addMenuItem(ui::SimpleGUI::Menu::Tools, "Sofa paths", "", [this](){ modifyDataRepository(); } );
	m_gui.addMenuItem(ui::SimpleGUI::Menu::Tools, "Launch Server", "", [this](){ launchServer(); } );
	m_gui.addMenuItem(ui::SimpleGUI::Menu::Tools, "Stop Server", "", [this](){ m_communication.closeCommunication(); m_serverRunning = false; });
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
	if(m_updateObjects)
	{
		for(auto model : m_scene.models())
			model->updatePositions();
	}

	m_scene.render();
}

bool Document::mouseEvent(const MouseEvent& event)
{
	return m_mouseManipulator.mouseEvent(event);
}

// Helper function to parse the material (a string) and extract the diffuse color
// The syntax of the material string is: MaterialName, Diffuse, DiffuseActive, DifuseColor(R,G,B,A), ...
glm::vec4 getColor(const std::string& material)
{
	std::string dummy;
	int active;
	std::istringstream ss(material);
	ss >> dummy >> dummy >> active;
	glm::vec4 color;
	for (int i = 0; i < 4; ++i)
		ss >> color[i];

	return color;
}

std::string trim(const std::string& str)
{
	auto first = str.find_first_not_of(" \t\n\r");
	if (first == std::string::npos) // only whitespace
		return "";
	auto last = str.find_last_not_of(" \t\n\r");
	return str.substr(first, last - first + 1);
}

Document::SofaModel Document::createSofaModel(sfe::Object& visualModel)
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

	auto model = std::make_shared<Model>();
	sofaModel.d_vertices.get(model->m_vertices);
	sofaModel.d_normals.get(model->m_normals);

	// Get the constant information (topology and color)
	// Triangles
	auto trianglesData = visualModel.data("triangles");
	if (trianglesData)
		trianglesData.get(model->m_triangles);

	// Quads
	auto quadsData = visualModel.data("quads");
	if (quadsData)
		quadsData.get(model->m_quads);

	if (model->m_triangles.empty() && model->m_quads.empty())
		return sofaModel;

	// The diffuse color
	auto matData = visualModel.data("material");
	std::string material;
	matData.get(material);
	model->m_color = getColor(material);

	// The texture
/*	model->texFileName = getTexture(visualModel, material);

	// Texture coordinates
	if (!model->texFileName.empty())
	{
		auto texCoords = visualModel.data("texcoords");
		if (texCoords)	texCoords.get(model->texCoords);
	}
*/
	sofaModel.model = model;
	return sofaModel;
}

void Document::parseScene()
{
	auto rootNode = m_simulation.root(); // Get the root node of the simulation
	if (!rootNode) // Test if the root node is valid
		return;

	// We look for every Sofa object inheriting from the class VisualModelImpl in the graph
	auto visualModels = rootNode.findObjects("VisualModelImpl", {}, sfe::Node::SearchDirection::Down);
	for (auto& visualModel : visualModels)
	{
		auto sofaModel = createSofaModel(visualModel);

		if (sofaModel.model)
			m_scene.addModel(sofaModel.model);
		m_sofaModels.push_back(sofaModel);
	}
}

void Document::updateObjects()
{
	for(auto sofaModel : m_sofaModels)
	{
		sofaModel.d_vertices.get(sofaModel.model->m_vertices);
		sofaModel.d_normals.get(sofaModel.model->m_normals);
	}
}

void Document::parseNode(GraphNode::Ptr parent, sfe::Node node)
{
	for(auto& object : node.objects())
	{
		auto n = createNode(object, parent);
		parent->objects.push_back(n);
	}

	for(auto& child : node.children())
	{
		auto n = createNode(child, parent);
		parseNode(n, child);
		parent->children.push_back(n);
	}
}

GraphNode::Ptr Document::createNode(sfe::Object object, GraphNode::Ptr parent)
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

	// Parse slaves
	for(auto& slave : object.slaves())
	{
		auto s = createNode(slave, n);
		n->objects.push_back(s);
	}

	return n;
}

GraphNode::Ptr Document::createNode(sfe::Node node, GraphNode::Ptr parent)
{
	auto n = SofaNode::create();
	n->name = node.name();
	n->uniqueId = node.uniqueId();
	n->parent = parent.get();
	n->isObject = false;
	n->node = node;
	m_graphImages.setImage(*n.get());

	return n;
}

void Document::createGraph()
{
	auto root = m_simulation.root();
	auto rootNode = createNode(root, nullptr);
	parseNode(rootNode, root);
	m_graph.setRoot(rootNode);
}

void Document::postStep()
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
			m_gui.setStatusBarText(m_statusFPS, ss.str());
		}
		++m_fpsCount;
	}

	// Udpate properties in opened dialogs
	std::async(&Document::updateProperties, this);

	updateObjects();
	m_updateObjects = true; // We have to modify the buffers in the correct thread
	m_gui.updateView();
}

ObjectProperties::SPtr Document::objectProperties(GraphNode* baseItem)
{
	auto item = dynamic_cast<SofaNode*>(baseItem);
	if(!item)
		return nullptr;

	ObjectProperties::SPtr properties;
	if(item->isObject)
		properties = createSofaObjectProperties(item->object);
	else
		properties = createSofaObjectProperties(item->node);

	{
		std::lock_guard<std::mutex> lock(m_openedObjectsPropertiesMutex);
		m_openedObjectProperties.push_back(properties);
	}
	return properties;
}

void Document::closeObjectProperties(ObjectPropertiesPtr properties)
{
	if(!m_openedObjectProperties.empty())
	{
		std::lock_guard<std::mutex> lock(m_openedObjectsPropertiesMutex);
		m_openedObjectProperties.erase(std::remove(m_openedObjectProperties.begin(), m_openedObjectProperties.end(), properties));
	}
}

void Document::singleStep()
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
	m_gui.setStatusBarText(m_statusFPS, ss.str());
}

void Document::resetSimulation()
{
	bool animating = m_simulation.isAnimating();
	if(animating)
		m_simulation.setAnimate(false, true); // We want to wait until the current step has finished
	auto objProps = m_openedObjectProperties;
	for(auto objProp : objProps)
		m_gui.closePropertiesDialog(objProp.get());
	m_simulation.reset();
	createGraph();
	m_fpsCount = 1;
	m_fpsStart = std::chrono::high_resolution_clock::now();
	if(animating)
		m_simulation.setAnimate(true);
}

void Document::modifyDataRepository()
{
	auto dialog = m_gui.createDialog("Sofa Data Repository");
	auto& panel = dialog->content();

	auto helper = m_simulation.getHelper();
	std::vector<std::string> paths = helper->dataRepositoryPaths();
	panel.addProperty(property::createRefProperty("Paths", paths));

	if(dialog->exec())
	{
		helper->setDataRepositoryPaths(paths);
		m_gui.settings().set("dataRepositoryPaths", paths);
	}
}

void Document::updateProperties()
{
	std::vector<ObjectPropertiesPtr> opened;
	{
		std::lock_guard<std::mutex> lock(m_openedObjectsPropertiesMutex);
		opened = m_openedObjectProperties;
	}

	for(auto& op : opened)
	{
		op->updateProperties();
		op->modified();
	}
}

void connectionFunc(int socket, bool readOnly)
{
	auto com = std::make_shared<sfecom::Communication>(socket);

	{
		sfes::Server server(com);
		server.setReadOnly(readOnly);
		server.defaultMessageLoop();
	}
}

void acceptFunc(sfecom::Communication* com, bool readOnly)
{
	while (true)
	{
		int socket = com->acceptConnection();
		if (socket < 0)
			return;
		std::thread(connectionFunc, socket, readOnly).detach();
	}
}

void Document::launchServer()
{
	if (m_serverRunning)
		return;

	auto dialog = m_gui.createDialog("Launch SFE Server");
	auto& panel = dialog->content();

	int port = 5074;
	panel.addProperty(property::createRefProperty("Server port", port));

	int readOnly = 0;
	auto readOnlyProp = property::createRefProperty("Read only", readOnly);
	readOnlyProp->setWidget("checkbox");
	panel.addProperty(readOnlyProp);

	if (dialog->exec())
	{
		m_serverRunning = m_communication.createServer(port);

		std::thread(acceptFunc, &m_communication, readOnly != 0).detach();
	}
}
