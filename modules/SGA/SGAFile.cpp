#include "SGAFile.h"
#include "Document.h"

#include <fstream>

SGAFile::SGAFile(simplerender::Scene& scene, Graph& graph)
	: m_scene(scene)
	, m_graph(graph)
{

}

bool SGAFile::loadFile(const std::string& path)
{
	return false;
}

bool SGAFile::saveFile(const std::string& path)
{
	return false;
}
