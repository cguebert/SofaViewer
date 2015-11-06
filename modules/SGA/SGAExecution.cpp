#include "SGAExecution.h"
#include "SGADocument.h"

#include <sfe/sofaFrontEndLocal.h>
#include <sga/types.h>

#include <fstream>

namespace sfe
{
template<> struct DataTypeTrait<glm::vec3> : public ArrayTypeTrait<glm::vec3, 3>{};
}

namespace
{

SGANode* getChild(GraphNode* parent, sga::ObjectDefinition::ObjectType type)
{
	for (auto child : parent->children)
	{
		auto sgaNode = dynamic_cast<SGANode*>(child.get());
		if (sgaNode->sgaDefinition && sgaNode->sgaDefinition.type() == type)
			return sgaNode;
	}

	return nullptr;
}

std::vector<SGANode*> getModifiers(GraphNode* parent)
{
	std::vector<SGANode*> list;
	for (auto child : parent->children)
	{
		auto sgaNode = dynamic_cast<SGANode*>(child.get());
		if (sgaNode->sgaDefinition && sgaNode->sgaDefinition.type() == sga::ObjectDefinition::ObjectType::ModifierObject)
			list.push_back(sgaNode);
	}

	return list;
}

sga::Transformation convertTransformation(const glm::mat4& matrix)
{
	auto input = toTransformationComponents(matrix);
	sga::Transformation output;
	for (int i = 0; i < 3; ++i)
	{
		output.translation[i] = input.translation[i];
		output.rotation[i] = input.rotation[i];
		output.scale[i] = input.scale[i];
	}

	return output;
}

}

namespace sgaExec
{

struct CreationContext
{
	std::string name;
	sga::ObjectWrapper parent;
	bool hasSGAParent = false;
	simplerender::Mesh::SPtr mesh;
	simplerender::Material::SPtr material;
	sga::Transformation transformation;
	sga::Vec3d boundingBox[2]; // min & max
	int modifierIndex = 0;
	int objectExportIndex = -1;

	glm::mat4 localTransformationMatrix, globalTransformationMatrix;
	sga::Transformation localTransformation, globalTransformation, parentTransformation;

	void setParent();
	void updateTransformation();
};

}

void sgaExec::CreationContext::setParent()
{
	hasSGAParent = true;
	parentTransformation = globalTransformation;
	localTransformationMatrix = glm::mat4();
	localTransformation.translation = { 0, 0, 0 };
	localTransformation.rotation = { 0, 0, 0 };
	localTransformation.scale = { 1, 1, 1 };
}

void sgaExec::CreationContext::updateTransformation()
{
	bool local = false;
	if (parent.constant("template") == "Rigid")
		local = true; // For rigid objects, the transformation of the parent is applied via the mapping

	transformation = local ? localTransformation : globalTransformation;
	if (!parent.constant("childIgnoresRotation").empty()) // Grid topologies are an exception concerning rotation
	{
		sga::Vec3d rot = sga::Vec3d{ 0, 0, 0 };
		if (!local)
		{
			rot = localTransformation.rotation;

			if (hasSGAParent)
			{
				for (int i = 0; i < 3; ++i)
					transformation.translation[i] = localTransformation.translation[i] + parentTransformation.translation[i];
			}
		}
		transformation.rotation = rot;
	}
}

//****************************************************************************//

SGAExecution::SGAExecution(simplerender::Scene& scene, sga::ObjectFactory factory, const std::string& dataPath)
	: m_scene(scene)
	, m_objectFactory(factory)
	, m_dataPath(dataPath)
	, m_sofaSimulation(sfe::getLocalSimulation())
{
	m_stepCallbackHandle = m_sofaSimulation.addCallback(sfe::Simulation::CallbackType::Step, [this](){ postStep(); });
}

SGAExecution::~SGAExecution()
{
	m_sofaSimulation.clear(); // Free the simulation
}

bool SGAExecution::convert(const SimulationProperties& simuProp, GraphNode* root)
{
	sgaExec::CreationContext context;
	auto rootSGANode = getChild(root, sga::ObjectDefinition::ObjectType::RootObject);
	if (!rootSGANode)
		return false;

	createSofaRoot(rootSGANode, context);
	m_sofaSimulation.setGravity(simuProp.gravity.x, simuProp.gravity.y, simuProp.gravity.z);
	m_sofaSimulation.setDt(simuProp.timestep);

	parseNode(root, context);
	postObjectsCreation();

	return true;
}

void SGAExecution::exportScene(const std::string& path)
{
	std::ofstream out(path);
	out << "<?xml version=\"1.0\"?>\n";
	m_sofaSimulation.root().exportXML(out);
}

void SGAExecution::parseNode(GraphNode* baseNode, sgaExec::CreationContext& context)
{
	auto meshNode = dynamic_cast<MeshNode*>(baseNode);
	if (!meshNode)
		return;

	if (meshNode->instance) // Instance
		convertObject(meshNode, context);
	else if (meshNode->nodeType == MeshNode::Type::Node || meshNode->nodeType == MeshNode::Type::Root)
	{
		context.globalTransformationMatrix = meshNode->transformationMatrix * context.localTransformationMatrix;
		context.globalTransformation = convertTransformation(context.globalTransformationMatrix);
		context.localTransformationMatrix = meshNode->transformationMatrix;
		context.localTransformation = convertTransformation(context.localTransformationMatrix);

		sgaExec::CreationContext tmpContext = context;
		for (auto child : meshNode->children)
		{
			parseNode(child.get(), context);
			context = tmpContext; // Reset the context each time we come back
		}
	}
}

void SGAExecution::convertObject(MeshNode* item, sgaExec::CreationContext& context)
{
	auto physicsNode = getChild(item, sga::ObjectDefinition::ObjectType::PhysicsObject);
	auto collisionNode = getChild(item, sga::ObjectDefinition::ObjectType::CollisionObject);
	auto visualNode = getChild(item, sga::ObjectDefinition::ObjectType::VisualObject);

	if (!physicsNode && !collisionNode && !visualNode)
		return;

	convertMesh(item, context);

	if (physicsNode)
	{
		context.parent = createSofaObject(physicsNode, context);
		context.setParent();
	}

	if (collisionNode)
	{
		context.updateTransformation();
		if (context.parent && context.parent.sofaNode())
		{
			auto wrapper = createSofaObject(collisionNode, context);
			if (!physicsNode)
			{
				context.parent = wrapper;
				context.setParent();
			}
		}
		else
			std::cerr << "Node is not valid, collision not created" << std::endl;
	}

	if (visualNode && context.parent && context.parent.sofaNode())
	{
		context.updateTransformation();
		auto visuMesh = createSofaObject(visualNode, context);

		UpdateModelStruct updateModel;
		auto visuModel = visuMesh.sofaNode().objectOfType("State");
		updateModel.verticesData = visuModel.data("position");
		updateModel.normalsData = visuModel.data("normal");
		updateModel.mesh = context.mesh;
		updateModel.material = context.material;
		m_updateModelStructs.push_back(updateModel);
	}

	auto modifiers = getModifiers(item);
	if (!modifiers.empty())
	{
		auto contextPtr = std::make_shared<sgaExec::CreationContext>(context);
		m_deferredCreations.emplace_back(contextPtr, modifiers);
	}
}

void SGAExecution::convertMesh(MeshNode* item, sgaExec::CreationContext& context)
{
	context.mesh = std::make_shared<simplerender::Mesh>(*item->instance->mesh); // Copy the mesh
	context.material = item->instance->material; // Keep the same material
	context.name = item->name;

	// Get the transformation and convert it for Sofa
	context.transformation = convertTransformation(item->transformationMatrix);
	
	// Update bounding box
	auto bb = simplerender::boundingBox(*item->mesh);
	for (int i = 0; i < 3; ++i)
	{
		context.boundingBox[0][i] = bb.first[i];
		context.boundingBox[1][i] = bb.second[i];
	}
}

void SGAExecution::fillProperties(SGANode* item, sgaExec::CreationContext& context)
{
	auto& definition = item->sgaDefinition;
	for (auto prop : definition.properties())
	{
		if (prop.id() == "name")
		{
			std::string name = context.name;
			std::replace(name.begin(), name.end(), ' ', '_'); // Removing whitespaces (is it necessary?)
			std::replace(name.begin(), name.end(), '.', '_'); // Replace dots because Sofa doesn't like them in an object name

			using ObjectType = sga::ObjectDefinition::ObjectType;
			switch (definition.type()) {
			case ObjectType::RootObject:		name = "Root";		break;
			case ObjectType::PhysicsObject:							break;
			case ObjectType::ModifierObject:	name += "_Modifier_" + std::to_string(context.modifierIndex);  break;
			case ObjectType::VisualObject:		name += "_Visu";	break;
			case ObjectType::CollisionObject:	name += "_Collis";	break;
			}
			prop.set(name);
		}
		else if (prop.id() == "dataPath")
		{
			prop.set(m_dataPath);
		}
		else if (prop.id() == "translation")
		{
			prop.set(context.transformation.translation);
		}
		else if (prop.id() == "rotation")
		{
			prop.set(context.transformation.rotation);
		}
		else if (prop.id() == "scale")
		{
			prop.set(context.transformation.scale);
		}
		else if (prop.id() == "vertices")
		{
			prop.set(context.mesh->m_vertices);
		}
		else if (prop.id() == "edges")
		{
			prop.set(context.mesh->m_edges);
		}
		else if (prop.id() == "triangles")
		{
			prop.set(context.mesh->m_triangles);
		}
		else if (prop.id() == "topologyType")
		{
			int type = -1;
			if (!context.mesh->m_triangles.empty()) type = 2;
			else if (!context.mesh->m_edges.empty()) type = 1;

			if (type != -1) prop.set(type);
		}
		else if (prop.id() == "exportedMesh")
		{

		}
		else if (prop.id() == "extendMin")
		{
			prop.set(context.boundingBox[0]);
		}
		else if (prop.id() == "extendMax")
		{
			prop.set(context.boundingBox[1]);
		}
		else if (prop.id() == "displayFlags")
		{
			std::string displayString = "showVisualModels";
			prop.set(displayString);
		}
	}
}

void SGAExecution::createSofaRoot(SGANode* item, sgaExec::CreationContext& context)
{
	fillProperties(item, context);
	m_simulationWrapper = m_objectFactory.createRoot(item->sgaDefinition, m_sofaSimulation);
	context.parent = m_simulationWrapper;
}

sga::ObjectWrapper SGAExecution::createSofaObject(SGANode* item, sgaExec::CreationContext& context)
{
	fillProperties(item, context);
	return m_objectFactory.create(item->sgaDefinition, context.parent);
}

void SGAExecution::postObjectsCreation()
{
	// Creation of the modifiers
	for (auto& deferred : m_deferredCreations)
	{
		auto& context = *deferred.first;
		int nbModifiers = deferred.second.size();
		for (int i = 0; i < nbModifiers; ++i)
		{
			context.modifierIndex = i;
			createSofaObject(deferred.second[i], context);
		}
	}
}

void SGAExecution::run(CallbackFunc updateViewFunc)
{
	m_sofaSimulation.init();

	m_originInstances = m_scene.instances();
	m_scene.instances().clear();

	for (auto& modelUdpate : m_updateModelStructs)
	{
		auto instance = std::make_shared<simplerender::ModelInstance>();
		instance->mesh = modelUdpate.mesh;
		instance->material = modelUdpate.material;
		m_scene.addInstance(instance);
	}

	m_updateViewFunc = updateViewFunc;
	m_sofaSimulation.setAnimate(true);
}

void SGAExecution::stop()
{
	m_sofaSimulation.setAnimate(false, true);

	m_scene.instances() = m_originInstances;

	m_updateModelStructs.clear();
	m_updateViewFunc();
}

void SGAExecution::render()
{
	if (!m_meshesInitialized)
	{
		for (auto& model : m_updateModelStructs)
			model.mesh->init();
		m_meshesInitialized = true;
	}
	else if(m_updateMeshes)
	{
		for (auto& model : m_updateModelStructs)
			model.mesh->updatePositions();
		m_updateMeshes = false;
	}

	m_scene.render();
}

void SGAExecution::postStep()
{
	for (auto& modelUpdate : m_updateModelStructs)
	{
		modelUpdate.verticesData.get(modelUpdate.mesh->m_vertices);
		modelUpdate.normalsData.get(modelUpdate.mesh->m_normals);
	}

	m_updateMeshes = true;
	m_updateViewFunc();
}
