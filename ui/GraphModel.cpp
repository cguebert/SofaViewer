#include "GraphModel.h"


#include <core/Graph.h>

GraphModel::GraphModel(QObject* parent, Graph& graph)
	: QAbstractItemModel(parent)
	, m_graph(graph)
{
}

QModelIndex GraphModel::index(int row, int column, const QModelIndex& index) const
{
	if (!hasIndex(row, column, index))
		return QModelIndex();

	const Graph::Node* item;
	if (!index.isValid())
		item = m_graph.root();
	else
		item = static_cast<Graph::Node*>(index.internalPointer());

	int nbObjects = item->objects.size(), nbChildren = item->children.size();
	if(row < nbObjects)
		return createIndex(row, column, item->objects[row].get());
	else if (row < nbObjects + nbChildren)
		return createIndex(row, column, item->children[row - nbObjects].get());
	else
		return QModelIndex();
}

int indexOfChild(Graph::Node* node)
{
	auto& children = node->parent->children;
	auto it = std::find_if(children.begin(), children.end(), [node](const Graph::NodePtr& ptr){
		return ptr.get() == node;
	});
	auto index = std::distance(children.begin(), it);
	return index;
}

QModelIndex GraphModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	Graph::Node* childItem = static_cast<Graph::Node*>(index.internalPointer());
	Graph::Node* parentItem = childItem->parent;

	if (parentItem == m_graph.root())
		return QModelIndex();

	return createIndex(indexOfChild(parentItem), 0, parentItem);
}

int GraphModel::rowCount(const QModelIndex& index) const
{
	if (index.column() > 0)
		return 0;

	const Graph::Node* item;
	if (!index.isValid())
		item = m_graph.root();
	else
		item = static_cast<Graph::Node*>(index.internalPointer());

	if(item)
		return item->objects.size() + item->children.size();
	else
		return 0;
}

int GraphModel::columnCount(const QModelIndex& /*index*/) const
{
	return 1;
}

QVariant GraphModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	Graph::Node* item = static_cast<Graph::Node*>(index.internalPointer());
	if(item->type.empty()) // Node
	{
		if(index.column())
			return QVariant();
		return QVariant(item->name.c_str());
	}

	QString name = QString(item->type.c_str()) + "  " + item->name.c_str();
	return QVariant(name);
}
