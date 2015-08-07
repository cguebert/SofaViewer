#pragma once

#include <core/Graph.h>

class SofaNode;

class GraphImages
{
public:
	GraphImages(Graph& graph);
	void setImage(SofaNode& node);

protected:
	int getImage(unsigned int flags);
	int imageId(unsigned int flags);
	int addImage(unsigned int flags, const Graph::Image& image);

	using ImagePair = std::pair<unsigned int, int>;
	using ImagePairList = std::vector<ImagePair>;
	ImagePairList m_images;

	Graph& m_graph;
};
