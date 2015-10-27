#pragma once

#include <sga/ObjectFactory.h>
#include <sfe/Simulation.h>

class SGANode;

namespace sgaExec
{
struct CreationContext;
}

namespace simplerender
{
class Scene;
}

class SGAExecution
{
public:
	SGAExecution(sga::ObjectFactory factory, const std::string& dataPath);

	void convert(SGANode* root);

private:
	void parseNode(SGANode* node, sgaExec::CreationContext& context);
	void convertObject(SGANode* item, sgaExec::CreationContext& context);
	void convertMesh(SGANode* item, sgaExec::CreationContext& context);
	void fillProperties(SGANode* item, sgaExec::CreationContext& context);

	void createSofaRoot(SGANode* item, sgaExec::CreationContext& context);
	sga::ObjectWrapper createSofaObject(SGANode* item, sgaExec::CreationContext& context);
	void postObjectsCreation();

	sga::ObjectFactory m_objectFactory;
	std::string m_dataPath;
	sfe::Simulation m_sofaSimulation;
	sga::ObjectWrapper m_simulationWrapper;

	using DeferredCreation = std::pair<std::shared_ptr<sgaExec::CreationContext>, std::vector<SGANode*>>;
	std::vector<DeferredCreation> m_deferredCreations;
};