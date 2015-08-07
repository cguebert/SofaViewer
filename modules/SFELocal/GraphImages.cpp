#include "GraphImages.h"
#include "Document.h"

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
static const uint32_t colorTable[ALLCOLORS]=
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

Graph::Image createImage(int w, int h)
{
	Graph::Image img;
	img.width = w;
	img.height = h;
	img.data.resize(w * h * 4, 0);
	return img;
}

inline uint32_t rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{ return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

inline void fromRgba(uint32_t c, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a)
{
	a = c >> 24;
	r = c >> 16;
	g = c >> 8;
	b = c;
}

inline uint32_t interpolateColor(uint32_t c1, uint32_t c2, float amt)
{
	amt = std::max(0.f, std::min(amt, 1.f));
	unsigned char f2 = static_cast<unsigned char>(255 * amt);
	unsigned char f1 = 255 - f2;
	uint32_t t = (c1 & 0xff00ff) * f1 + (c2 & 0xff00ff) * f2;
	t >>= 8;
	t &= 0xff00ff;

	c1 = ((c1 >> 8) & 0xff00ff) * f1 + ((c2 >> 8) & 0xff00ff) * f2;
	c1 &= 0xff00ff00;
	return (c1 | t);
}

inline float smoothstep(float val, float x0, float x1)
{
	if (val < x0)	return 0.f;
	if (val >= x1)	return 1.f;
	val = (val - x0) / (x1 - x0);
	return (val*val * (3 - 2*val));
}

// Warning: no bounds check!
inline void setPixel(Graph::Image& image, int x, int y, uint32_t color)
{
	reinterpret_cast<uint32_t*>(&image.data[0])[y * image.width + x] = color;
}

inline void fill(Graph::Image& image, int x0, int x1, int y0, int y1, uint32_t color)
{
	for(int y = y0; y <= y1; ++y)
		for(int x = x0; x <= x1; ++x)
			setPixel(image, x, y, color);
}

void disk(Graph::Image& image, uint32_t color)
{
	const int w = image.width, h = image.height;
	const float cx = (w-1) / 2.0f, cy = (h-1) / 2.0f;
	const float d2 = std::min(cx, cy);
	const float d1 = d2 - 0.5f;
	const float d0 = std::max(0.f, d1 - 1.5f);
	const auto black = rgba(0, 0, 0, 255);
	const auto transparent = rgba(0, 0, 0, 0);

	for(int y = 0; y < h; ++y)
	{
		for(int x = 0; x < w; ++x)
		{
			const float dx = x - cx, dy = y - cy;
			const float d = sqrt(dx*dx + dy*dy);

			if(d < d0) // Inside
				setPixel(image, x, y, color);
			else if(d > d2) // Outside
				setPixel(image, x, y, transparent);
			else if(d < d1) // From inside to the stroke
				setPixel(image, x, y,
						 interpolateColor(color, black, smoothstep(d, d0, d1))
						 );
			else if(d > d1) // From the stroke to outside
				setPixel(image, x, y,
						 interpolateColor(black, transparent, smoothstep(d, d1, d2))
						 );
		}
	}
}

using ColorList = std::vector<uint32_t>;
const int squareSize = 10;
void squares(Graph::Image& image, const ColorList& colors)
{
	const int w = image.width, h = image.height;
	const auto black = rgba(0, 0, 0, 255);
	fill(image, 0, w-1, 0, 0, black);
	fill(image, 0, w-1, h-1, h-1, black);
	fill(image, 0, 0, 0, h-1, black);
	fill(image, w-1, w-1, 0, h-1, black);

	for(int i = 0, nb = colors.size(); i < nb; ++i)
		fill(image, 1 + i * squareSize, (i+1) * squareSize, 1, h - 2, colors[i]);
}

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

ColorList getColors(unsigned int flags)
{
	ColorList colorsList;
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
		auto img = createImage(16, 16);
		disk(img, Colors::colorTable[Colors::NODE]);
		return addImage(flags, img);
	}
	else if(flags == Colors::NODE_SLEEPING)
	{
		auto img = createImage(16, 16);
		disk(img, Colors::colorTable[Colors::NODE_SLEEPING]);
		return addImage(flags, img);
	}
	// For objects, create a rectangle composed of a colored square for each active flag
	else
	{
		auto colors = getColors(flags);
		auto img = createImage(squareSize * colors.size() + 2, squareSize + 2);
		squares(img, colors);
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

int GraphImages::addImage(unsigned int flags, const Graph::Image& image)
{
	auto id = m_graph.addImage(image);
	m_images.emplace_back(flags, id);
	return id;
}
