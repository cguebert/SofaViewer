#include "SGAExecution.h"
#include "Document.h"

#include <sfe/sofaFrontEndLocal.h>
#include <sga/types.h>

#include <glm/gtx/matrix_decompose.hpp>

#include <fstream>

namespace sfe
{
template<> struct DataTypeTrait<glm::vec3> : public ArrayTypeTrait<glm::vec3, 3>{};
}

namespace
{

SGANode* getChild(SGANode* parent, sga::ObjectDefinition::ObjectType type)
{
	for (auto child : parent->children)
	{
		auto sgaNode = dynamic_cast<SGANode*>(child.get());
		if (sgaNode->sgaDefinition && sgaNode->sgaDefinition.type() == type)
			return sgaNode;
	}

	return nullptr;
}

std::vector<SGANode*> getModifiers(SGANode* parent)
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

}

namespace sgaExec
{

struct CreationContext
{
	std::string name;
	sga::ObjectDefinition::ObjectType objectType = sga::ObjectDefinition::ObjectType::PhysicsObject;
	sga::ObjectWrapper parent;
	simplerender::Scene::ModelPtr model;
	sga::Transformation transformation;
	sga::Vec3d boundingBox[2]; // min & max
	int modifierIndex = 0;
	int objectExportIndex = -1;
};

}

SGAExecution::SGAExecution(sga::ObjectFactory factory, const std::string& dataPath)
	: m_objectFactory(factory)
	, m_dataPath(dataPath)
	, m_sofaSimulation(sfe::getLocalSimulation())
{

}

void SGAExecution::convert(SGANode* root)
{
	sgaExec::CreationContext context;
	createSofaRoot(root, context);

	parseNode(root, context);
	postObjectsCreation();

	std::ofstream out("ExportSofaScene.scn");
	out << "<?xml version=\"1.0\"?>\n";
	m_sofaSimulation.root().exportXML(out);
}

void SGAExecution::parseNode(SGANode* node, sgaExec::CreationContext& context)
{
	if (node->model) // Instance or Mesh
		convertObject(node, context);
	else if (node->nodeType == SGANode::Type::Node || node->nodeType == SGANode::Type::Root)
	{
		sgaExec::CreationContext tmpContext = context;
		for (auto child : node->children)
		{
			parseNode(dynamic_cast<SGANode*>(child.get()), context);
			context = tmpContext; // Reset the context each time we come back
		}
	}
}

void SGAExecution::convertObject(SGANode* item, sgaExec::CreationContext& context)
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
	}

	if (collisionNode)
	{
		if (context.parent && context.parent.sofaNode())
		{
			auto wrapper = createSofaObject(collisionNode, context);
			if (!physicsNode)
				context.parent = wrapper;
		}
		else
			std::cerr << "Node is not valid, collision not created" << std::endl;
	}

	if (visualNode && context.parent && context.parent.sofaNode())
	{
		auto visuMesh = createSofaObject(visualNode, context);
	}

	auto modifiers = getModifiers(item);
	if (!modifiers.empty())
	{
		auto contextPtr = std::make_shared<sgaExec::CreationContext>(context);
		m_deferredCreations.emplace_back(contextPtr, modifiers);
	}
}

void SGAExecution::convertMesh(SGANode* item, sgaExec::CreationContext& context)
{
	context.model = item->model;
	context.name = item->name;

	// Get the transformation and convert it for Sofa
	auto modelTrans = glm::transpose(item->transformation);
	glm::vec3 scale, translation, skew;
	glm::vec4 perspective;
	glm::quat orientation;
	if (glm::decompose(modelTrans, scale, orientation, translation, skew, perspective))
	{
		glm::vec3 rotation = glm::degrees(glm::eulerAngles(orientation));
		for (int i = 0; i < 3; ++i)
		{
			context.transformation.translation[i] = translation[i];
			context.transformation.rotation[i] = rotation[i];
			context.transformation.scale[i] = scale[i];
		}
	}

	// Update bounding box
	auto bb = simplerender::boundingBox(*item->model);
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
			prop.set(context.model->m_vertices);
		}
		else if (prop.id() == "edges")
		{
			// prop.set(context.model->m_edges);
		}
		else if (prop.id() == "triangles")
		{
			prop.set(context.model->m_triangles);
		}
		else if (prop.id() == "topologyType")
		{
			int type = -1;
			if (!context.model->m_triangles.empty()) type = 2;
		//	else if (!context.model->m_edges.empty()) type = 1;

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
	auto rootNode = getChild(item, sga::ObjectDefinition::ObjectType::RootObject);
	fillProperties(rootNode, context);
	m_simulationWrapper = m_objectFactory.createRoot(rootNode->sgaDefinition, m_sofaSimulation);
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
