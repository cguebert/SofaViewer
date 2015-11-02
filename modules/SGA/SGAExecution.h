#pragma once

#include <sga/ObjectFactory.h>
#include <sfe/Simulation.h>
#include <render/Scene.h>

class SGANode;
struct SimulationProperties;

namespace sgaExec
{
struct CreationContext;
}

class SGAExecution
{
public:
	using CallbackFunc = std::function<void()>;

	SGAExecution(simplerender::Scene& scene, sga::ObjectFactory factory, const std::string& dataPath);
	~SGAExecution();

	void convert(const SimulationProperties& simuProp, SGANode* root);

	void run(CallbackFunc updateViewFunc);
	void stop();
	void render();

private:
	void parseNode(SGANode* node, sgaExec::CreationContext& context);
	void convertObject(SGANode* item, sgaExec::CreationContext& context);
	void convertMesh(SGANode* item, sgaExec::CreationContext& context);
	void fillProperties(SGANode* item, sgaExec::CreationContext& context);

	void createSofaRoot(SGANode* item, sgaExec::CreationContext& context);
	sga::ObjectWrapper createSofaObject(SGANode* item, sgaExec::CreationContext& context);
	void postObjectsCreation();

	void postStep();

	simplerender::Scene& m_scene;
	sga::ObjectFactory m_objectFactory;
	std::string m_dataPath;
	sfe::Simulation m_sofaSimulation;
	sga::ObjectWrapper m_simulationWrapper;

	using DeferredCreation = std::pair<std::shared_ptr<sgaExec::CreationContext>, std::vector<SGANode*>>;
	std::vector<DeferredCreation> m_deferredCreations;

	simplerender::Scene::Models m_originModels;
	simplerender::Scene::ModelInstances m_originInstances;

	struct UpdateModelStruct
	{
		simplerender::Scene::ModelPtr model;
		sfe::Data verticesData, normalsData;
	};
	std::vector<UpdateModelStruct> m_updateModelsStructs;
	CallbackFunc m_updateViewFunc;
	bool m_modelsInitialized = false;
	sfe::CallbackHandle m_stepCallbackHandle;
};