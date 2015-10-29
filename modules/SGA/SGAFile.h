#pragma once

#include <string>

class Graph;

namespace simplerender
{
class Scene;
}

class SGAFile
{
public:
	SGAFile(simplerender::Scene& scene, Graph& graph);

	bool loadFile(const std::string& path);
	bool saveFile(const std::string& path);

private:
	simplerender::Scene& m_scene;
	Graph& m_graph;
};