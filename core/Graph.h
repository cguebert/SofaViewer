#pragma once

#include <memory>
#include <string>
#include <vector>

class Graph
{
public:

	class Node;
	using NodePtr = std::shared_ptr<Node>;

	class Node
	{
	public:
		virtual ~Node() {}
		std::string name, type;
		std::vector<NodePtr> children, objects;
		Node* parent = nullptr; // Only null for the root

		static NodePtr create() { return std::make_shared<Graph::Node>(); }
	};

	const Node* root() const;
	void setRoot(NodePtr root);

protected:

	NodePtr m_root;
};

inline const Graph::Node* Graph::root() const
{ return m_root.get(); }

