#pragma once

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
		int imageId = -1;

		static NodePtr create() { return std::make_shared<Graph::Node>(); }
	};

	Node* root() const;
	void setRoot(NodePtr root);

	using ImagesList = std::vector<Image>;
	const ImagesList& images() const;
	void setImages(const ImagesList& images);

protected:
	NodePtr m_root;
	ImagesList m_images;
};

inline Graph::Node* Graph::root() const
{ return m_root.get(); }

inline void Graph::setRoot(NodePtr root)
{ m_root = root; }

inline const Graph::ImagesList& Graph::images() const
{ return m_images; }

inline void Graph::setImages(const ImagesList& images)
{ m_images = images; }
