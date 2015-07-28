#include <core/Document.h>
#include <core/ObjectProperties.h>

#include <sfe/sofaFrontEndLocal.h>

#include <QFileInfo>
#include <QDir>

class ChangeDir
{
public:
	ChangeDir(const QString& path)
	{
		prevDir = QDir::current();
		QDir::setCurrent(QFileInfo(path).absolutePath());
	}

	~ChangeDir() { QDir::setCurrent(prevDir.absolutePath()); }

protected:
	QDir prevDir;
};

/******************************************************************************/

ViewUpdater& ViewUpdater::get()
{
	static ViewUpdater instance;
	return instance;
}

void ViewUpdater::setSignal(UpdateFunc func)
{
	m_func = func;
}

void ViewUpdater::update()
{
	if(m_func)
		m_func();
}

/******************************************************************************/

Document::Document()
	: m_mouseManipulator(m_scene)
{
	m_simulation = sfe::getLocalSimulation();
}

bool Document::loadFile(const QString& path)
{
	ChangeDir cd(path);
	std::string cpath = path.toLocal8Bit().constData();
	if (!m_simulation.loadFile(cpath))
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
		model->updatePositions();
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

	return n;
}

void Document::createGraph()
{
	auto root = m_simulation.root();
	auto rootNode = createNode(root, nullptr);
	parseNode(rootNode, root);
	m_graph.setRoot(rootNode);
}

void Document::step()
{
	m_simulation.step();
	updateObjects();
	ViewUpdater::get().update();
}

template <class T>
void setValue(ObjectProperties::PropertyPtr& prop, sfe::Data data)
{
	T val;
	data.get(val);
	prop->setValue(std::make_shared<PropertyValue<T>>(std::move(val)));
}

void addData(Document::ObjectPropertiesPtr properties, sfe::Data data)
{
	if(!data || !data.displayed())
		return;

	auto storageType = static_cast<Property::Type>(data.supportedType());

	auto typeTrait = data.typeInfo();
	std::string valueType;
	int columnCount = 1;
	if(typeTrait && typeTrait->ValidInfo())
	{
		valueType = typeTrait->ValueType()->name();
		columnCount = typeTrait->size();
	}

	auto prop = std::make_shared<Property>(data.name(), data.help(), data.group(), data.readOnly(), storageType, valueType);
	prop->setColumnCount(columnCount);

	switch(storageType)
	{
	case sfe::Data::DataType::Int:		setValue<int32_t>(prop, data);		break;
	case sfe::Data::DataType::Float:	setValue<float>(prop, data);		break;
	case sfe::Data::DataType::Double:	setValue<double>(prop, data);		break;
	case sfe::Data::DataType::String:	setValue<std::string>(prop, data);	break;
	case sfe::Data::DataType::Vector_Int:		setValue<std::vector<int32_t>>(prop, data);		break;
	case sfe::Data::DataType::Vector_Float:		setValue<std::vector<float>>(prop, data);		break;
	case sfe::Data::DataType::Vector_Double:	setValue<std::vector<double>>(prop, data);		break;
	case sfe::Data::DataType::Vector_String:	setValue<std::vector<std::string>>(prop, data); break;
	}

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
