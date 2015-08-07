#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

class Graph
{
public:

	class Node;
	using NodePtr = std::shared_ptr<Node>;

	// Image container for the graph decoration
	struct Image
	{
		int width, height;
		std::vector<unsigned char> data; // ARGB32
	};

	// One node of the graph
	class Node
	{
	public:
		virtual ~Node() {}
		std::string name, type;
		std::vector<NodePtr> children, objects;
		Node* parent = nullptr; // Only null for the root

		int imageId = -1; // Id of the image to draw for this node (-1 if no image)
		size_t uniqueId = 0; // Used to recognize nodes when the graph is reconstructed
		bool expanded = true; // Only the initial state

		static NodePtr create() { return std::make_shared<Graph::Node>(); }
	};

	Node* root() const;
	void setRoot(NodePtr root);

	using ImagesList = std::vector<Image>;
	const ImagesList& images() const;
	int addImage(const Image& image); // Return the id of this image

	enum class CallbackReason : uint16_t
	{ BeginSetNode, EndSetNode };

	using CallbackFunc = std::function<void(uint16_t)>;
	void setUpdateCallback(CallbackFunc func); // For the GUI to respond to modifications in the graph (for now, only when setRoot is called)

protected:
	void executeCallback(CallbackReason reason);

	NodePtr m_root;
	ImagesList m_images;
	CallbackFunc m_updateCallback;
};

inline Graph::Node* Graph::root() const
{ return m_root.get(); }

inline const Graph::ImagesList& Graph::images() const
{ return m_images; }

inline void Graph::setUpdateCallback(CallbackFunc func)
{ m_updateCallback = func; }
