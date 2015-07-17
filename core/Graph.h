#pragma once

#include <memory>
#include <string>
#include <vector>

class Graph
{
public:

	struct Node;
	using NodePtr = std::shared_ptr<Node>;

	struct Node
	{
		std::string name, type;
		std::vector<NodePtr> children, objects;
		Node* parent = nullptr; // Only null for the root
		size_t uniqueId; // Used to identify the Node when asking its information

		static NodePtr create();
	};

	const Node* root() const;
	void setRoot(NodePtr root);

protected:

	NodePtr m_root;
};

inline const Graph::Node* Graph::root() const
{ return m_root.get(); }

inline Graph::NodePtr Graph::Node::create()
{ return std::make_shared<Graph::Node>(); }
