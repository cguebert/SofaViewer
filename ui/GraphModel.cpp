#include "GraphModel.h"

#include <core/Graph.h>

#include <QPixmap>

namespace
{

int indexOfChild(GraphNode* node)
{
	auto& children = node->parent->children;
	auto it = std::find_if(children.begin(), children.end(), [node](const GraphNode::SPtr& ptr){
		return ptr.get() == node;
	});
	auto index = std::distance(children.begin(), it);
	return index;
}

}

GraphModel::GraphModel(QObject* parent, Graph& graph)
	: QAbstractItemModel(parent)
	, m_graph(graph)
{
	updatePixmaps();
}

QModelIndexList GraphModel::getPersistentIndexList() const
{
	return persistentIndexList();
}

void GraphModel::operation(int opVal, GraphNode* node, int first, int last)
{
	auto op = static_cast<Graph::CallbackReason>(opVal);
	switch (op)
	{
	case Graph::CallbackReason::BeginSetRoot:
		beginResetModel();
		break;

	case Graph::CallbackReason::EndSetRoot:
		endResetModel();
		break;

	case Graph::CallbackReason::BeginInsertNode:
		beginInsertRows(index(node), first, last);
		break;

	case Graph::CallbackReason::EndInsertNode:
		endInsertRows();
		break;

	case Graph::CallbackReason::BeginRemoveNode:
		beginRemoveRows(index(node), first, last);
		break;

	case Graph::CallbackReason::EndRemoveNode:
		endRemoveRows();
		break;
	}
}

QModelIndex GraphModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	const GraphNode* item;
	if (!parent.isValid())
		return createIndex(0, 0, m_graph.root());
	else
		item = static_cast<GraphNode*>(parent.internalPointer());

	int nbChildren = item->children.size();
	if (row < nbChildren)
		return createIndex(row, column, item->children[row].get());
	else
		return QModelIndex();
}

QModelIndex GraphModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	GraphNode* childItem = static_cast<GraphNode*>(index.internalPointer());
	GraphNode* parentItem = childItem->parent;

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
		return m_graph.root() ? 1 : 0;
	else
	{
		auto item = static_cast<GraphNode*>(parent.internalPointer());
		return item->children.size();
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
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if(item->type.empty()) // Node
		{
			if(index.column())
				return QVariant();
			return QVariant(QString::fromStdString(item->name));
		}

		QString name = QString::fromStdString(item->type) + "  " + QString::fromStdString(item->name);
		return QVariant(name);
	}
	else if(role == Qt::DecorationRole)
	{
		GraphNode* item = static_cast<GraphNode*>(index.internalPointer());
		if(item->imageId != -1)
			return QVariant(m_pixmaps[item->imageId]);
	}

	return QVariant();
}

void GraphModel::updatePixmaps()
{
	const auto& images = m_graph.images();
	for(int i = m_pixmaps.size(), nb = images.size(); i < nb; ++i)
	{
		const auto& graphImg = images[i];
		if (graphImg.data())
		{
			QImage img(graphImg.data(), graphImg.width(), graphImg.height(), QImage::Format_ARGB32);
			m_pixmaps.push_back(QPixmap::fromImage(std::move(img)));
		}
		else // Fallback if empty image
		{
			QPixmap pix(10, 10);
			pix.fill(QColor(255, 0, 255));
			m_pixmaps.push_back(pix);
		}		
	}
}

QModelIndex GraphModel::index(GraphNode* node)
{
	if (!node || node == m_graph.root())
		return createIndex(0, 0, node);

	return createIndex(indexOfChild(node), 0, node);
}
