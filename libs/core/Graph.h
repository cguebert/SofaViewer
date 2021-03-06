#pragma once

#include <core/core.h>
#include <core/GraphImage.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

// One node of the graph
class CORE_API GraphNode
{
public:
	virtual ~GraphNode() {}
	std::string name, type;

	using SPtr = std::shared_ptr<GraphNode>;
	std::vector<SPtr> children;
	GraphNode* parent = nullptr; // Only null for the root

	int imageId = -1; // Id of the image to draw for this node (-1 if no image)
	size_t uniqueId = 0; // Used to recognize nodes when the graph is reconstructed
	bool expanded = true; // Only the initial state

	static SPtr create() { return std::make_shared<GraphNode>(); }
};

//****************************************************************************//

namespace graph
{

using NodeFunc = std::function<void(GraphNode*)>;
using SelectFunction = std::function<bool(GraphNode*)>;
using GraphNodes = std::vector<GraphNode*>;
enum class TraversalOrder { BreathFirst, DepthFirst };

int CORE_API indexOfChild(GraphNode* parent, GraphNode* child);
void CORE_API forEach(GraphNode* root, const NodeFunc& nodeFunc, TraversalOrder order = TraversalOrder::BreathFirst);
GraphNodes CORE_API getNodes(GraphNode* root, const SelectFunction& selectFunc, TraversalOrder order = TraversalOrder::BreathFirst);

}

//****************************************************************************//

class CORE_API Graph
{
public:
	GraphNode* root() const;
	void setRoot(GraphNode::SPtr root);

	using ImagesList = std::vector<GraphImage>;
	const ImagesList& images() const;
	int addImage(const GraphImage& image); // Return the id of this image

	enum class CallbackReason : int
	{ BeginSetRoot, EndSetRoot, BeginInsertNode, EndInsertNode, BeginRemoveNode, EndRemoveNode };

	using CallbackFunc = std::function<void(int reason, GraphNode* node, int first, int last)>;
	void setUpdateCallback(CallbackFunc func); // For the GUI to respond to modifications in the graph (for now, only when setRoot is called)

	// The following methods execute the corresponding callbacks
	void insertChild(GraphNode* parent, GraphNode::SPtr child, int position);
	void removeChild(GraphNode* parent, GraphNode* child);

protected:
	void executeCallback(CallbackReason reason, GraphNode* node = nullptr, int first = 0, int last = 0);

	GraphNode::SPtr m_root;
	ImagesList m_images;
	CallbackFunc m_updateCallback;
};

inline GraphNode* Graph::root() const
{ return m_root.get(); }

inline const Graph::ImagesList& Graph::images() const
{ return m_images; }

inline void Graph::setUpdateCallback(CallbackFunc func)
{ m_updateCallback = func; }
