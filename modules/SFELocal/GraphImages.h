#pragma once

#include <core/Graph.h>

class SofaNode;

class GraphImages
{
public:
	void setImage(SofaNode& node);
	Graph::ImagesList images();

protected:
	int getImage(unsigned int flags);
	int imageId(unsigned int flags);
	int addImage(unsigned int flags, const Graph::Image& image);

	using ImagePair = std::pair<unsigned int, Graph::Image>;
	using ImagePairList = std::vector<ImagePair>;
	ImagePairList m_images;
};
