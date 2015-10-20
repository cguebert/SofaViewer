#include <core/Graph.h>

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

void Graph::executeCallback(CallbackReason reason)
{
	if(m_updateCallback)
	{
		auto val = static_cast<uint8_t>(reason);
		m_updateCallback(val);
	}
}
