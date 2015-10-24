#include <core/Graph.h>

#include <algorithm>

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

void Graph::addObject(GraphNode* parent, GraphNode::SPtr object)
{
	executeCallback(CallbackReason::BeginSetNode);
	parent->objects.push_back(object);
	executeCallback(CallbackReason::EndSetNode);
}

void Graph::removeObject(GraphNode* parent, GraphNode* object)
{
	executeCallback(CallbackReason::BeginSetNode);
	auto& objects = parent->objects;
	auto it = std::find_if(objects.begin(), objects.end(), [object](const GraphNode::SPtr node){
		return node.get() == object;
	});

	if (it != objects.end())
		objects.erase(it);
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
