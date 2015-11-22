#include <core/Graph.h>

#include <algorithm>
#include <deque>

namespace graph
{

int indexOfChild(GraphNode* parent, GraphNode* child)
{
	auto it = std::find_if(parent->children.begin(), parent->children.end(), [child](const GraphNode::SPtr& node){
		return node.get() == child;
	});

	if (it == parent->children.end())
		return -1;

	return std::distance(parent->children.begin(), it);
}

void forEach(GraphNode* root, const NodeFunc& nodeFunc, TraversalOrder order)
{
	std::deque<GraphNode*> openList = { root };
	if (order == TraversalOrder::BreathFirst)
	{
		while (!openList.empty())
		{
			auto current = openList.front();
			openList.pop_front();
			for (auto& child : current->children)
				openList.push_back(child.get());
			nodeFunc(current);
		}
	}
	else if (order == TraversalOrder::DepthFirst)
	{
		while (!openList.empty())
		{
			auto current = openList.front();
			openList.pop_front();
			for (auto it = current->children.rbegin(), itEnd = current->children.rend(); it != itEnd; ++it)
				openList.push_front(it->get()); // Push at the front, conversing the order, converting to raw pointers
			nodeFunc(current);
		}
	}
}

GraphNodes getNodes(GraphNode* root, const SelectFunction& selectFunc, TraversalOrder order)
{
	GraphNodes selection;
	forEach(root, [&selection, &selectFunc](GraphNode* node){
		if (selectFunc(node))
			selection.push_back(node);
	}, order);

	return selection;
}

}

//****************************************************************************//

void Graph::setRoot(GraphNode::SPtr root)
{
	executeCallback(CallbackReason::BeginSetRoot);
	m_root = root;
	executeCallback(CallbackReason::EndSetRoot, root.get());
}

int Graph::addImage(const GraphImage& image)
{
	int id = m_images.size();
	m_images.push_back(image);
	return id;
}

void Graph::insertChild(GraphNode* parent, GraphNode::SPtr child, int position)
{
	if (position < 0)
	{
		auto last = parent->children.size();
		executeCallback(CallbackReason::BeginInsertNode, parent, last, last);
		parent->children.push_back(child);
	}
	else
	{
		executeCallback(CallbackReason::BeginInsertNode, parent, position, position);
		parent->children.insert(parent->children.begin() + position, child);
	}
	executeCallback(CallbackReason::EndInsertNode, child.get());
}

void Graph::removeChild(GraphNode* parent, GraphNode* child)
{
	auto index = graph::indexOfChild(parent, child);
	if (index == -1)
		return;

	executeCallback(CallbackReason::BeginRemoveNode, parent, index, index);
	auto& children = parent->children;
	children.erase(children.begin() + index);
	executeCallback(CallbackReason::EndRemoveNode);
}

void Graph::executeCallback(CallbackReason reason, GraphNode* node, int first, int last)
{
	if(m_updateCallback)
	{
		auto val = static_cast<uint8_t>(reason);
		m_updateCallback(val, node, first, last);
	}
}
