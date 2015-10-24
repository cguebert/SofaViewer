#include <core/Graph.h>

#include <algorithm>

int indexOfChild(GraphNode* parent, GraphNode* child)
{
	auto it = std::find_if(parent->children.begin(), parent->children.end(), [child](const GraphNode::SPtr& node){
		return node.get() == child;
	});

	if (it == parent->children.end())
		return -1;

	return std::distance(parent->children.begin(), it);
}

//****************************************************************************//

void Graph::setRoot(GraphNode::SPtr root)
{
	executeCallback(CallbackReason::BeginSetNode);
	m_root = root;
	executeCallback(CallbackReason::EndSetNode);

}

int Graph::addImage(const GraphImage& image)
{
	int id = m_images.size();
	m_images.push_back(image);
	return id;
}

void Graph::addChild(GraphNode* parent, GraphNode::SPtr child)
{
	executeCallback(CallbackReason::BeginSetNode);
	parent->children.push_back(child);
	executeCallback(CallbackReason::EndSetNode);
}

void Graph::insertChild(GraphNode* parent, GraphNode::SPtr child, int position)
{
	executeCallback(CallbackReason::BeginSetNode);
	if (position < 0)
		parent->children.push_back(child);
	else
		parent->children.insert(parent->children.begin() + position, child);
	executeCallback(CallbackReason::EndSetNode);
}

void Graph::removeChild(GraphNode* parent, GraphNode* child)
{
	executeCallback(CallbackReason::BeginSetNode);
	auto& children = parent->children;
	auto it = std::find_if(children.begin(), children.end(), [child](const GraphNode::SPtr node){
		return node.get() == child;
	});

	if (it != children.end())
		children.erase(it);
	executeCallback(CallbackReason::EndSetNode);
}

void Graph::executeCallback(CallbackReason reason)
{
	if(m_updateCallback)
	{
		auto val = static_cast<uint8_t>(reason);
		m_updateCallback(val);
	}
}
