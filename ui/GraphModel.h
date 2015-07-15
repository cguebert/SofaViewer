#pragma once

#include <QAbstractItemModel>

class Graph;

class GraphModel : public QAbstractItemModel
{
public:
	GraphModel(QObject* parent, Graph& graph);

	QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent) const override;
	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& parent, int role) const override;

protected:
	Graph& m_graph;
};
