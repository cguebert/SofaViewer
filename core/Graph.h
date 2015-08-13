#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

// One node of the graph
class GraphNode
{
public:
	virtual ~GraphNode() {}
	std::string name, type;

	using Ptr = std::shared_ptr<GraphNode>;
	std::vector<Ptr> children, objects;
	GraphNode* parent = nullptr; // Only null for the root

	int imageId = -1; // Id of the image to draw for this node (-1 if no image)
	size_t uniqueId = 0; // Used to recognize nodes when the graph is reconstructed
	bool expanded = true; // Only the initial state

	static Ptr create() { return std::make_shared<GraphNode>(); }
};

//****************************************************************************//

class Graph
{
public:
	// Image container for the graph decoration
	struct Image
	{
		int width, height;
		std::vector<unsigned char> data; // ARGB32
	};

	GraphNode* root() const;
	void setRoot(GraphNode::Ptr root);

	using ImagesList = std::vector<Image>;
	const ImagesList& images() const;
	int addImage(const Image& image); // Return the id of this image

	enum class CallbackReason : uint16_t
	{ BeginSetNode, EndSetNode };

	using CallbackFunc = std::function<void(uint16_t)>;
	void setUpdateCallback(CallbackFunc func); // For the GUI to respond to modifications in the graph (for now, only when setRoot is called)

protected:
	void executeCallback(CallbackReason reason);

	GraphNode::Ptr m_root;
	ImagesList m_images;
	CallbackFunc m_updateCallback;
};

inline GraphNode* Graph::root() const
{ return m_root.get(); }

inline const Graph::ImagesList& Graph::images() const
{ return m_images; }

inline void Graph::setUpdateCallback(CallbackFunc func)
{ m_updateCallback = func; }
