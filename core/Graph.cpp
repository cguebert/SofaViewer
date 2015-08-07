#include <core/Graph.h>

void Graph::setRoot(NodePtr root)
{
	executeCallback(CallbackReason::BeginSetNode);
	m_root = root;
	executeCallback(CallbackReason::EndSetNode);

}

int Graph::addImage(const Image& image)
{
	int id = m_images.size();
	m_images.push_back(image);
	return id;
}

void Graph::executeCallback(CallbackReason reason)
{
	if(m_updateCallback)
	{
		auto val = static_cast<uint16_t>(reason);
		m_updateCallback(val);
	}
}
