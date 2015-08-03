#include "Document.h"

#include <core/ObjectProperties.h>
#include <core/SimpleGUI.h>

#include <sfe/sofaFrontEndLocal.h>

Document::Document(ui::SimpleGUI& gui)
	: BaseDocument(gui)
	, m_gui(gui)
	, m_mouseManipulator(m_scene)
{
	m_simulation = sfe::getLocalSimulation();

	m_simulation.setCallback(sfe::Simulation::CallbackType::Step, [this](){ postStep(); });
	m_simulation.setCallback(sfe::Simulation::CallbackType::Reset, [this](){ postStep(); });

	m_gui.buttonsPanel().addButton("Animate", "Pause or play the simulation", [this](){
		bool animating = m_simulation.isAnimating();
		m_simulation.setAnimate(!animating);
	});
	m_gui.buttonsPanel().addButton("Step", "Do a single step of the simulation", [this](){ singleStep(); }, 0, 1);

	m_gui.buttonsPanel().addButton("Reset", "Reset the simulation", [this](){
		bool animating = m_simulation.isAnimating();
		if(animating)
			m_simulation.setAnimate(false, true); // We want to wait until the current step has finished
		m_simulation.reset();
		m_fpsCount = 1;
		m_fpsStart = std::chrono::high_resolution_clock::now();
		if(animating)
			m_simulation.setAnimate(true);
	});

	m_statusFPS = m_gui.addStatusBarZone("FPS: 999.9");
	m_gui.setStatusBarText(m_statusFPS, "");
}

bool Document::loadFile(const std::string& path)
{
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

		// Update the scene graph
		createGraph();

		return true;
	}
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

Scene::ModelPtr createSofaModel(sfe::Object& visualModel)
{
	Scene::ModelPtr model = std::make_shared<Model>();
	model->m_sofaObject = visualModel; // Store the proxy to this object

	// Store every Data proxy we will use in the updateObjects function
	auto posData = visualModel.data("position");
	auto vertData = visualModel.data("vertices");

	// Determine if we use the position Data or the vertices Data
	std::vector<float> tmp;
	vertData.get(tmp);
	if(tmp.empty())
		model->d_vertices = posData;
	else
		model->d_vertices = posData;
	model->d_normals = visualModel.data("normal");

	if (!model->d_vertices || !model->d_normals)
		return nullptr;

	model->d_vertices.get(model->m_vertices);
	model->d_normals.get(model->m_normals);

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
		return nullptr;

	model->mergeIndices();

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
	return model;
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
		auto model = createSofaModel(visualModel);

		if (model)
			m_scene.addModel(model);
	}
}

void Document::updateObjects()
{
	for(auto model : m_scene.models())
	{
		model->d_vertices.get(model->m_vertices);
		model->d_normals.get(model->m_normals);
	}
}

void Document::parseNode(Graph::NodePtr parent, sfe::Node node)
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

Graph::NodePtr Document::createNode(sfe::Object object, Graph::NodePtr parent)
{
	auto n = SofaNode::create();
	n->name = object.name();
	n->type = object.className();
	n->parent = parent.get();
	n->isObject = true;
	n->object = object;
	m_graphImages.setImage(*n.get());

	// Parse slaves
	for(auto& slave : object.slaves())
	{
		auto s = createNode(slave, n);
		n->objects.push_back(s);
	}

	return n;
}

Graph::NodePtr Document::createNode(sfe::Node node, Graph::NodePtr parent)
{
	auto n = SofaNode::create();
	n->name = node.name();
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
	m_graph.setImages(m_graphImages.images());
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

	updateObjects();
	m_updateObjects = true; // We have to modify the buffers in the correct thread
	ViewUpdater::get().update();
}

template <class T>
ObjectProperties::PropertyPtr createProp(sfe::Data data, const std::string& widget)
{
	T val;
	data.get(val);
	auto prop = std::make_shared<Property>(data.name(), widget, data.readOnly(), data.help(), data.group());
	prop->setValue(std::make_shared<PropertyValueCopy<T>>(std::move(val)));
	return prop;
}

void addData(Document::ObjectPropertiesPtr properties, sfe::Data data)
{
	if(!data || !data.displayed())
		return;

	auto storageType = data.supportedType();

	auto typeTrait = data.typeInfo();
	std::string valueType;
	int columnCount = 1;
	if(typeTrait && typeTrait->ValidInfo())
	{
		valueType = typeTrait->ValueType()->name();
		columnCount = typeTrait->size();
	}

	std::string widget;
	if(valueType == "bool")
		widget = "checkbox";

	ObjectProperties::PropertyPtr prop;
	switch(storageType)
	{
	case sfe::Data::DataType::Int:		prop = createProp<int>(data, widget);			break;
	case sfe::Data::DataType::Float:	prop = createProp<float>(data, widget);			break;
	case sfe::Data::DataType::Double:	prop = createProp<double>(data, widget);		break;
	case sfe::Data::DataType::String:	prop = createProp<std::string>(data, widget);	break;
	case sfe::Data::DataType::Vector_Int:		prop = createProp<std::vector<int>>(data, widget);			break;
	case sfe::Data::DataType::Vector_Float:		prop = createProp<std::vector<float>>(data, widget);		break;
	case sfe::Data::DataType::Vector_Double:	prop = createProp<std::vector<double>>(data, widget);		break;
	case sfe::Data::DataType::Vector_String:	prop = createProp<std::vector<std::string>>(data, widget);	break;
	}

	prop->setColumnCount(columnCount);
	properties->addProperty(prop);
}

Document::ObjectPropertiesPtr Document::objectProperties(Graph::Node* baseItem) const
{
	auto item = dynamic_cast<SofaNode*>(baseItem);
	if(!item)
		return nullptr;

	if(item->isObject)
	{
		const auto& object = item->object;
		auto prop = std::make_shared<ObjectProperties>(object.name(), object.className(), object.templateName());

		auto names = object.listData();
		for(const auto& name : names)
			addData(prop, object.data(name));

		return prop;
	}
	else
	{
		const auto& node = item->node;
		auto prop = std::make_shared<ObjectProperties>(node.name());

		auto names = node.listData();
		for(const auto& name : names)
			addData(prop, node.data(name));

		return prop;
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
