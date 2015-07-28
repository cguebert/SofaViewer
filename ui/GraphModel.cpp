#include "GraphModel.h"

#include <core/Graph.h>

#include <QPixmap>

GraphModel::GraphModel(QObject* parent, Graph& graph)
	: QAbstractItemModel(parent)
	, m_graph(graph)
{
	initPixmaps();
}

QModelIndex GraphModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	const Graph::Node* item;
	if (!parent.isValid())
		return createIndex(0, 0, m_graph.root());
	else
		item = static_cast<Graph::Node*>(parent.internalPointer());

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

	if (childItem == m_graph.root())
		return QModelIndex();
	else if (parentItem == m_graph.root())
		return createIndex(0, 0, parentItem);

	return createIndex(indexOfChild(parentItem), 0, parentItem);
}

int GraphModel::rowCount(const QModelIndex& parent) const
{
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		return 1;
	else
	{
		auto item = static_cast<Graph::Node*>(parent.internalPointer());
		return item->objects.size() + item->children.size();
	}
}

int GraphModel::columnCount(const QModelIndex& /*index*/) const
{
	return 1;
}

QVariant GraphModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if(role == Qt::DisplayRole)
	{
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
	else if(role == Qt::DecorationRole)
	{
		Graph::Node* item = static_cast<Graph::Node*>(index.internalPointer());
		if(item->imageId != -1)
			return QVariant(m_pixmaps[item->imageId]);
	}

	return QVariant();
}

void GraphModel::initPixmaps()
{
	m_pixmaps.clear();
	for(const auto& graphImg : m_graph.images())
	{
		QImage img(&graphImg.data[0], graphImg.width, graphImg.height, QImage::Format_ARGB32);
		m_pixmaps.push_back(QPixmap::fromImage(std::move(img)));
	}
}
