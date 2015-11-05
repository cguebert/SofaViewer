#pragma once

#include <sga/ObjectFactory.h>
#include <sfe/Simulation.h>
#include <render/Scene.h>

class GraphNode;
class MeshNode;
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

	bool convert(const SimulationProperties& simuProp, GraphNode* root);

	void run(CallbackFunc updateViewFunc);
	void stop();
	void render();

private:
	void parseNode(GraphNode* node, sgaExec::CreationContext& context);
	void convertObject(MeshNode* item, sgaExec::CreationContext& context);
	void convertMesh(MeshNode* item, sgaExec::CreationContext& context);
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

	simplerender::Scene::Meshes m_originMeshes;
	simplerender::Scene::ModelInstances m_originInstances;

	struct UpdateMeshStruct
	{
		simplerender::Mesh::SPtr mesh;
		sfe::Data verticesData, normalsData;
	};
	std::vector<UpdateMeshStruct> m_updateMeshesStructs;
	CallbackFunc m_updateViewFunc;
	bool m_meshesInitialized = false;
	sfe::CallbackHandle m_stepCallbackHandle;
};