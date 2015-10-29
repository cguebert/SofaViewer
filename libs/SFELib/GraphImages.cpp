#include "GraphImages.h"
#include "SofaDocument.h"

#include <iostream>

namespace Colors
{

enum
{
	NODE,
	NODE_SLEEPING,
	OBJECT,
	CONTEXT,
	BMODEL,
	CMODEL,
	MMODEL,
	PROJECTIVECONSTRAINTSET,
	CONSTRAINTSET,
	IFFIELD,
	FFIELD,
	SOLVER,
	COLLISION,
	MMAPPING,
	MAPPING,
	MASS,
	TOPOLOGY,
	VMODEL,
	LOADER ,
	CONFIGURATIONSETTING,
	ALLCOLORS
};

// See http://www.graphviz.org/doc/info/colors.html
// The following is mostly the "set312" colors
static const GraphImage::Color colorTable[ALLCOLORS] =
{
	/*Node                  =*/ 0xffdedede, // color 9
	/*Sleeping Node			=*/ 0xff6f6f6f, // color 9 (darker)
	/*Object                =*/ 0xffffffff, // white
	/*Context               =*/ 0xffd7191c, // color spectral4/1
	/*BehaviorModel         =*/ 0xff93ff49, // color 7 (brighter)
	/*CollisionModel        =*/ 0xfffccde5, // color 8
	/*MechanicalState       =*/ 0xff8dd3c7, // color 1
	/*ProjectiveConstraint  =*/ 0xfffdb462, // color 6
	/*ConstraintSet         =*/ 0xfff98912, // color 6
	/*InteractionForceField =*/ 0xfffb8072, // color 4
	/*ForceField            =*/ 0xffbebada, // color 3
	/*Solver                =*/ 0xffb3de69, // color 7
	/*CollisionPipeline     =*/ 0xffbc80bd, // color 10
	/*MechanicalMapping     =*/ 0xff2b83da, // color spectral4/4
	/*Mapping               =*/ 0xff80b1d3, // color 5
	/*Mass                  =*/ 0xffffffb3, // color 2
	/*Topology              =*/ 0xffffed6f, // color 12
	/*VisualModel           =*/ 0xffeefdea, // color 11 (brighter)
	/*Loader                =*/ 0xff00daff, // cyan
	/*ConfigurationSetting  =*/ 0xffaaaaaa, // pale pink
};

} // namespace colors

namespace
{

inline void addFlag(unsigned int& flags, const std::vector<std::string>& hierarchy, const std::string& type, int flag)
{
	if(hierarchy.end() != std::find(hierarchy.begin(), hierarchy.end(), type))
		flags |= 1 << flag;
}

unsigned int getFlags(const sfe::Object& object)
{
	auto hierarchy = object.hierarchy();
	unsigned int flags = 0;
	addFlag(flags, hierarchy, "ContextObject", Colors::CONTEXT);
	addFlag(flags, hierarchy, "BehaviorModel", Colors::BMODEL);
	addFlag(flags, hierarchy, "CollisionModel", Colors::CMODEL);
	addFlag(flags, hierarchy, "BaseMechanicalState", Colors::MMODEL);
	addFlag(flags, hierarchy, "BaseProjectiveConstraintSet", Colors::PROJECTIVECONSTRAINTSET);
	addFlag(flags, hierarchy, "BaseConstraintSet", Colors::CONSTRAINTSET);
	addFlag(flags, hierarchy, "BaseInteractionForceField", Colors::IFFIELD);
	addFlag(flags, hierarchy, "BaseForceField", Colors::FFIELD);
	addFlag(flags, hierarchy, "BaseAnimationLoop", Colors::SOLVER);
	addFlag(flags, hierarchy, "OdeSolver", Colors::SOLVER);
	addFlag(flags, hierarchy, "Pipeline", Colors::COLLISION);
	addFlag(flags, hierarchy, "Intersection", Colors::COLLISION);
	addFlag(flags, hierarchy, "Detection", Colors::COLLISION);
	addFlag(flags, hierarchy, "ContactManager", Colors::COLLISION);
	addFlag(flags, hierarchy, "CollisionGroupManager", Colors::COLLISION);
	addFlag(flags, hierarchy, "BaseMapping", Colors::MAPPING); // TODO: isMechanical -> MMAPPING
	addFlag(flags, hierarchy, "BaseMass", Colors::MASS);
	addFlag(flags, hierarchy, "Topology", Colors::TOPOLOGY);
	addFlag(flags, hierarchy, "BaseTopologyObject", Colors::TOPOLOGY);
	addFlag(flags, hierarchy, "BaseLoader", Colors::LOADER);
	addFlag(flags, hierarchy, "ConfigurationSetting", Colors::CONFIGURATIONSETTING);
	addFlag(flags, hierarchy, "VisualModel", Colors::VMODEL);

	if(!flags)
		flags |= 1 << Colors::OBJECT;
	return flags;
}

GraphImage::ColorsList getColors(unsigned int flags)
{
	GraphImage::ColorsList colorsList;
	for(int i = 0; i < Colors::ALLCOLORS; ++i)
	{
		if(flags & (1 << i))
			colorsList.push_back(Colors::colorTable[i]);
	}

	return colorsList;
}

} // namespace

GraphImages::GraphImages(Graph& graph)
	: m_graph(graph)
{
}

void GraphImages::setImage(SofaNode& node)
{
	if(node.isObject)
		node.imageId = getImage(getFlags(node.object));
	else
	{
		int sleepingVal = 0;
		auto sleepingData = node.node.data("sleeping");
		if(sleepingData)
			sleepingData.get(sleepingVal);
		if(sleepingVal == 0)
			node.imageId = getImage(Colors::NODE);
		else
			node.imageId = getImage(Colors::NODE_SLEEPING);
	}
}

int GraphImages::getImage(unsigned int flags)
{
	auto id = imageId(flags);
	if(id != -1)
		return id;

	// Create the image as it doesn't yet exist

	// For nodes, create a disk
	if(flags == Colors::NODE)
	{
		auto img = GraphImage::createDiskImage(Colors::colorTable[Colors::NODE]);
		return addImage(flags, img);
	}
	else if(flags == Colors::NODE_SLEEPING)
	{
		auto img = GraphImage::createDiskImage(Colors::colorTable[Colors::NODE_SLEEPING]);
		return addImage(flags, img);
	}
	// For objects, create a rectangle composed of a colored square for each active flag
	else
	{
		auto colors = getColors(flags);
		auto img = GraphImage::createSquaresImage(colors);
		return addImage(flags, img);
	}
}

int GraphImages::imageId(unsigned int flags)
{
	auto it = std::find_if(m_images.begin(), m_images.end(), [flags](const ImagePair& img){
		return img.first == flags;
	});

	if(it == m_images.end())
		return -1;

	return it->second;
}

int GraphImages::addImage(unsigned int flags, const GraphImage& image)
{
	auto id = m_graph.addImage(image);
	m_images.emplace_back(flags, id);
	return id;
}
